/***************************************************************************
      MemoryManager.cpp  -  Manager for virtual and physical memory
			     -------------------
    begin                : Wed Aug 08 2001
    copyright            : (C) 2000 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "config.h"

#include <stdlib.h>
#include <unistd.h>

#include <limits>

#include <QFile>
#include <QFileInfo>
#include <QLatin1Char>
#include <QMutexLocker>
#include <QString>

#include "libkwave/MemoryManager.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"
#include "libkwave/memcpy.h"

/** static instance of the memory manager */
static Kwave::MemoryManager g_instance;

/** last used handle */
Kwave::Handle Kwave::MemoryManager::m_last_handle = 0;

//***************************************************************************
Kwave::MemoryManager::MemoryManager()
    :m_undo_limit(0), m_physical(), m_lock()
{
    // reset statistics
#ifdef DEBUG_MEMORY
    memset(&m_stats, 0x00, sizeof(m_stats));
#endif /* DEBUG_MEMORY */
}

//***************************************************************************
Kwave::MemoryManager::~MemoryManager()
{
    close();
}

//***************************************************************************
void Kwave::MemoryManager::close()
{
    QMutexLocker lock(&m_lock);

    // print warnings for each physical memory block
    Q_ASSERT(m_physical.isEmpty());
}

//***************************************************************************
Kwave::MemoryManager &Kwave::MemoryManager::instance()
{
    return g_instance;
}

//***************************************************************************
void Kwave::MemoryManager::setUndoLimit(quint64 mb)
{
    QMutexLocker lock(&m_lock);
    m_undo_limit = mb;
}

//***************************************************************************
quint64 Kwave::MemoryManager::undoLimit() const
{
    return m_undo_limit;
}

//***************************************************************************
Kwave::Handle Kwave::MemoryManager::newHandle()
{
    for (unsigned int i = 0; i < std::numeric_limits<int>::max(); i++) {
	// increment to get the next handle
	m_last_handle++;

	// allow only non-zero positive values (handle wraparound)
	if (m_last_handle <= 0) {
	    m_last_handle = 0;
	    continue;
	}

	// if handle is in use -> next one please...
	if (m_physical.contains(m_last_handle))      continue;

	// valid number and not in use -> found a new one :-)
	return m_last_handle;
    }

    // if we reach this point all handles are in use :-(
    return 0;
}

//***************************************************************************
Kwave::Handle Kwave::MemoryManager::allocate(size_t size)
{
    QMutexLocker lock(&m_lock);

    Kwave::Handle handle = allocatePhysical(size);
#ifdef DEBUG_MEMORY
    if (!handle) {
	qWarning("Kwave::MemoryManager::allocate(%u) - out of memory!",
	          Kwave::toUint(size));
    }
    dump("allocate");
#endif /* DEBUG_MEMORY */

    return handle;
}

//***************************************************************************
Kwave::Handle Kwave::MemoryManager::allocatePhysical(size_t size)
{
    // get a new handle
    Kwave::Handle handle = newHandle();
    if (!handle) return 0; // out of handles :-(

    // try to allocate via malloc
    void *mem = ::malloc(size);
    if (!mem) return 0; // out of memory :-(

    // store the object in the list of physical objects
    physical_memory_t phys;
    phys.m_data     = mem;
    phys.m_size     = size;
    phys.m_mapcount = 0;
    Q_ASSERT(!m_physical.contains(handle));
    m_physical.insert(handle, phys);
#ifdef DEBUG_MEMORY
    m_stats.physical.handles++;
    m_stats.physical.allocs++;
    m_stats.physical.bytes += size;
#endif /* DEBUG_MEMORY */

    return handle;
}

//***************************************************************************
quint64 Kwave::MemoryManager::physicalUsed()
{
    quint64 used = 0;
    foreach (const physical_memory_t &mem, m_physical.values())
	used += (mem.m_size >> 10) + 1;
    return (used >> 10);
}

//***************************************************************************
bool Kwave::MemoryManager::resize(Kwave::Handle handle, size_t size)
{
    QMutexLocker lock(&m_lock);

//     qDebug("Kwave::MemoryManager[%9d] - resize to %u", handle,
//            Kwave::toUint(size));

    // case 1: physical memory
    if (m_physical.contains(handle)) {
	const physical_memory_t phys_c = m_physical[handle];

	// check: it must not be mmapped!
	Q_ASSERT(!phys_c.m_mapcount);
	if (phys_c.m_mapcount) return false;

#ifdef DEBUG_MEMORY
	size_t current_size = phys_c.m_size;
#endif /* DEBUG_MEMORY */

	// try to resize the physical memory
	physical_memory_t phys = m_physical[handle];
	void *old_block = phys.m_data;
	void *new_block = ::realloc(old_block, size);
	if (new_block) {
	    phys.m_data     = new_block;
	    phys.m_size     = size;
	    phys.m_mapcount = 0;
	    m_physical[handle] = phys;
#ifdef DEBUG_MEMORY
	    m_stats.physical.bytes -= current_size;
	    m_stats.physical.bytes += size;
#endif /* DEBUG_MEMORY */

	    dump("resize");
	    return true;
	}
    }

    return false; // nothing known about this object / invalid handle?
}

//***************************************************************************
size_t Kwave::MemoryManager::sizeOf(Kwave::Handle handle)
{
    if (!handle) return 0;
    QMutexLocker lock(&m_lock);

    // case 1: physical memory
    if (m_physical.contains(handle)) {
	const physical_memory_t phys_c = m_physical[handle];
	return phys_c.m_size;
    }

    return 0;
}

//***************************************************************************
void Kwave::MemoryManager::free(Kwave::Handle &handle)
{
    if (!handle) return;
    QMutexLocker lock(&m_lock);

//     qDebug("Kwave::MemoryManager[%9d] - free", handle);

    if (m_physical.contains(handle)) {
	// physical memory (must not be mapped)
	Q_ASSERT(!m_physical[handle].m_mapcount);

#ifdef DEBUG_MEMORY
	m_stats.physical.handles--;
	m_stats.physical.frees++;
	m_stats.physical.bytes -= m_physical[handle].m_size;
#endif /* DEBUG_MEMORY */

	void *b = m_physical[handle].m_data;
	Q_ASSERT(b);
	m_physical.remove(handle);
	::free(b);
	handle = 0;

	dump("free");
	return;
    }

    Q_ASSERT(!handle);
    handle = 0;
}

//***************************************************************************
void *Kwave::MemoryManager::map(Kwave::Handle handle)
{
    QMutexLocker lock(&m_lock);

    Q_ASSERT(handle);
    if (!handle) return Q_NULLPTR; // object not found ?

    // simple case: physical memory does not really need to be mapped
    if (m_physical.contains(handle)) {
	m_physical[handle].m_mapcount++;
// 	qDebug("Kwave::MemoryManager[%9d] - mmap -> physical", handle);
	return m_physical[handle].m_data;
    }

    // nothing known about this object !?
    return Q_NULLPTR;
}

//***************************************************************************
void Kwave::MemoryManager::unmap(Kwave::Handle handle)
{
    Q_UNUSED(handle)
}

//***************************************************************************
int Kwave::MemoryManager::readFrom(Kwave::Handle handle, unsigned int offset,
                                   void *buffer, unsigned int length)
{
    QMutexLocker lock(&m_lock);

    if (!handle) return 0;

    // simple case: physical memory -> memcpy(...)
    if (m_physical.contains(handle)) {
//  	qDebug("Kwave::MemoryManager[%9d] - readFrom -> physical", handle);
	char *data = reinterpret_cast<char *>(m_physical[handle].m_data);
        MEMCPY(buffer, data + offset, length);
        return length;
    }

    // no physical mem
    return 0;
}

//***************************************************************************
int Kwave::MemoryManager::writeTo(Kwave::Handle handle, unsigned int offset,
                                  const void *buffer, unsigned int length)
{
    QMutexLocker lock(&m_lock);

    if (!handle) return 0;

    // simple case: memcpy to physical memory
    if (m_physical.contains(handle)) {
	physical_memory_t &mem = m_physical[handle];
// 	qDebug("Kwave::MemoryManager[%9d] - writeTo -> physical", handle);
	char *data = reinterpret_cast<char *>(mem.m_data);
	Q_ASSERT(length <= mem.m_size);
	Q_ASSERT(offset < mem.m_size);
	Q_ASSERT(offset + length <= mem.m_size);
	MEMCPY(data + offset, buffer, length);
	return length;
    }

    return 0;
}

//***************************************************************************
void Kwave::MemoryManager::dump(const char *function)
{
#if 0
    quint64 p_used  = physicalUsed();

    qDebug("------- %s -------", function);
    foreach (const Kwave::Handle &handle, m_physical.keys())
	qDebug("        P[%5u]: %5u", static_cast<unsigned int>(handle),
	                              m_physical[handle].m_size >> 20);
#endif

#ifdef DEBUG_MEMORY
    qDebug("------- %s -------", function);
    qDebug("physical:    %12llu, %12llu / %12llu (%12llu : %12llu)",
	   m_stats.physical.handles,
	   m_stats.physical.bytes,
	   m_stats.physical.limit,
	   m_stats.physical.allocs,
	   m_stats.physical.frees);
    qDebug("mapped swap: %12llu, %12llu / %12llu (%12llu : %12llu)",
	   m_stats.swap.mapped.handles,
	   m_stats.swap.mapped.bytes,
	   m_stats.swap.limit,
	   m_stats.swap.allocs,
	   m_stats.swap.frees);
    qDebug("cached:      %12llu, %12llu",
	   m_stats.swap.cached.handles,
	   m_stats.swap.cached.bytes);
    qDebug("unmapped:    %12llu, %12llu",
	   m_stats.swap.unmapped.handles,
	   m_stats.swap.unmapped.bytes);
    qDebug("-----------------------------------------------------------------");

#else /* DEBUG_MEMORY */
    Q_UNUSED(function)
#endif /* DEBUG_MEMORY */
}

//***************************************************************************
//***************************************************************************
