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
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include <QFile>
#include <QFileInfo>
#include <QMutexLocker>
#include <QString>

#ifdef HAVE_SYSINFO
#include <linux/kernel.h> // for struct sysinfo
#include <sys/sysinfo.h>  // for sysinfo()
#endif

#ifdef HAVE_GETRLIMIT
#include <sys/resource.h> // for getrlimit()
#endif

#include "libkwave/memcpy.h"
#include "libkwave/MemoryManager.h"

#include "SwapFile.h"

/** number of elements in the m_cached_swap list */
#define CACHE_SIZE 16

/** static instance of the memory manager */
static Kwave::MemoryManager g_instance;

/** last used handle */
Kwave::Handle Kwave::MemoryManager::m_last_handle = 0;

//***************************************************************************
Kwave::MemoryManager::MemoryManager()
    :m_physical_allocated(0), m_physical_limit(0), m_virtual_allocated(0),
     m_virtual_limit(0), m_swap_dir("/tmp"), m_undo_limit(0),
     m_physical(), m_unmapped_swap(), m_mapped_swap(),
     m_cached_swap(), m_lock()
{
    // determine amount of physical memory
    m_physical_limit = totalPhysical();
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

    // remove all remaining swap files and print warnings
    Q_ASSERT(m_unmapped_swap.isEmpty());
    Q_ASSERT(m_mapped_swap.isEmpty());
    Q_ASSERT(m_cached_swap.isEmpty());
}

//***************************************************************************
Kwave::MemoryManager &Kwave::MemoryManager::instance()
{
    return g_instance;
}

//***************************************************************************
void Kwave::MemoryManager::setPhysicalLimit(unsigned int mb)
{
    QMutexLocker lock(&m_lock);

    m_physical_limit = mb;
    mb = totalPhysical();
    if (m_physical_limit > mb) m_physical_limit = mb;
}

//***************************************************************************
void Kwave::MemoryManager::setVirtualLimit(unsigned int mb)
{
    QMutexLocker lock(&m_lock);

    m_virtual_limit = mb;

/** @todo write a function to find out the limit of virtual memory */
//    mb = totalVirtual();
//    if (m_virtual_limit > mb) m_virtual_limit = mb;
}

//***************************************************************************
void Kwave::MemoryManager::setSwapDirectory(const QString &dir)
{
    QMutexLocker lock(&m_lock);
    m_swap_dir = dir;
}

//***************************************************************************
void Kwave::MemoryManager::setUndoLimit(unsigned int mb)
{
    QMutexLocker lock(&m_lock);

    m_undo_limit = mb;
    mb = totalPhysical();
    if (m_undo_limit > mb) m_undo_limit = mb;
}

//***************************************************************************
unsigned int Kwave::MemoryManager::undoLimit() const
{
    return m_undo_limit;
}

//***************************************************************************
unsigned int Kwave::MemoryManager::totalPhysical()
{
    quint64 total = 4096;

#ifdef HAVE_SYSINFO
    // get the physically installed memory
    quint64 installed_physical = total;
    struct sysinfo info;

    sysinfo(&info);

    // find out installed memory and convert to megabytes
#ifdef HAVE_SYSINFO_MEMUNIT
    installed_physical = (info.totalram * info.mem_unit) >> 20;
#else /* HAVE_SYSINFO_MEMUNIT */
    installed_physical = info.totalram >> 20;
#endif /* HAVE_SYSINFO_MEMUNIT */
    qDebug("info.totalram = %lu MB",
	static_cast<long unsigned int>(installed_physical));

    if (installed_physical && (installed_physical < total))
	total = installed_physical;
#endif /* HAVE_SYSINFO */

#ifdef HAVE_GETRLIMIT
    struct rlimit limit;

    // check ulimit of data segment size
    if (getrlimit(RLIMIT_DATA, &limit) == 0) {
	unsigned int physical_ulimit = limit.rlim_cur >> 20;
	if (physical_ulimit < total) total = physical_ulimit;
    }

    // check ulimit of total (physical+virtual) system memory
#ifdef RLIMIT_AS
    if (getrlimit(RLIMIT_AS, &limit) == 0) {
	unsigned int total_ulimit = limit.rlim_cur >> 20;
	if (total_ulimit < total) total = total_ulimit;
    }
#endif /* RLIMIT_AS */
#endif /* HAVE_GETRLIMIT */

    return total;
}

//***************************************************************************
Kwave::Handle Kwave::MemoryManager::newHandle()
{
    for (unsigned int i = 0; i < INT_MAX; i++) {
	// increment to get the next handle
	m_last_handle++;

	// allow only non-zero positive values (handle wraparound)
	if (m_last_handle <= 0) {
	    m_last_handle = 0;
	    continue;
	}

	// if handle is in use -> next one please...
	if (m_physical.contains(m_last_handle))      continue;
	if (m_mapped_swap.contains(m_last_handle))   continue;
	if (m_unmapped_swap.contains(m_last_handle)) continue;
	if (m_cached_swap.contains(m_last_handle))   continue;

	// valid number and not in use -> found a new one :-)
	return m_last_handle;
    }

    // if we reach this point all handles are in use :-(
    return 0;
}

//***************************************************************************
bool Kwave::MemoryManager::freePhysical(size_t size)
{
    size_t freed = 0;

    if (m_physical.isEmpty()) return false;

    QList<Kwave::Handle> handles = m_physical.keys();
    QMutableListIterator<Kwave::Handle> it(handles);
    it.toBack();
    while (it.hasPrevious()) {
	Kwave::Handle handle = it.previous();
	const physical_memory_t &p = m_physical[handle];
	if (p.m_mapcount) continue; // in use :-(

	// convert to swapfile
	size_t s = p.m_size;
	qDebug("Kwave::MemoryManager[%9d] - swapping %2u MB out to make "\
	       "space for %2u MB", handle,
	       static_cast<unsigned int>(s >> 20),
	       static_cast<unsigned int>(size >> 20));

	if (convertToVirtual(handle, s)) {
	    freed += s;
	    if (freed >= size) return true;

	    // abort if the list is now empty
	    if (m_physical.isEmpty()) return false;
	}
    }

    return false;
}

//***************************************************************************
Kwave::Handle Kwave::MemoryManager::allocate(size_t size)
{
    QMutexLocker lock(&m_lock);

    Kwave::Handle handle = allocatePhysical(size);
    if (!handle) {
	// try to make some room in the physical memory area
	if (freePhysical(size)) {
	    // and try again to allocate physical memory
	    handle = allocatePhysical(size);
	}

	if (!handle) {
	    // fallback: allocate a swapfile
	    handle = allocateVirtual(size);
	}
    }

    if (!handle) {
	qWarning("Kwave::MemoryManager::allocate(%u) - out of memory!",
	          static_cast<unsigned int>(size));
    }

    dump("allocate");
    return handle;
}

//***************************************************************************
Kwave::Handle Kwave::MemoryManager::allocatePhysical(size_t size)
{
    // check for limits
    unsigned limit = totalPhysical();
    if (m_physical_limit < limit) limit = m_physical_limit;
    unsigned int used = physicalUsed();
    unsigned int available = (used < limit) ? (limit - used) : 0;
    if ((size >> 20) >= available) return 0;

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

    return handle;
}

//***************************************************************************
size_t Kwave::MemoryManager::physicalUsed()
{
    size_t used = 0;
    foreach (const physical_memory_t &mem, m_physical.values())
	used += (mem.m_size >> 10) + 1;
    return (used >> 10);
}

//***************************************************************************
size_t Kwave::MemoryManager::virtualUsed()
{
    size_t used = 0;

    foreach (const SwapFile *swapfile, m_cached_swap.values())
	used += (swapfile->size() >> 10) + 1;

    foreach (const SwapFile *swapfile, m_mapped_swap.values())
	used += (swapfile->size() >> 10) + 1;

    foreach (const SwapFile *swapfile, m_unmapped_swap.values())
	used += (swapfile->size() >> 10) + 1;

    return (used >> 10);
}

//***************************************************************************
QString Kwave::MemoryManager::nextSwapFileName(Kwave::Handle handle)
{
    QFileInfo file;
    QString filename;

    // these 6 'X' chars are needed for mkstemp !
    filename = "kwave-swapfile-%1-XXXXXX";
    filename = filename.arg(static_cast<unsigned int>(handle),
                            10, 10, QChar('0'));

    file.setFile(m_swap_dir, filename);
    return file.absoluteFilePath();
}

//***************************************************************************
Kwave::Handle Kwave::MemoryManager::allocateVirtual(size_t size)
{
    // check for limits
    unsigned int limit = INT_MAX; // totalVirtual() in MB
    if (m_virtual_limit < limit) limit = m_virtual_limit;
    unsigned int used = virtualUsed(); // in MB
    unsigned int available = (used < limit) ? (limit - used) : 0;
    if ((size >> 20) >= available) {
	qWarning("Kwave::MemoryManager::allocateVirtual(%u): out of memory, "\
	         "(used: %uMB, available: %uMB, limit=%uMB)",
	         static_cast<unsigned int>(size), used, available, limit);
	dump("allocateVirtual");
        return 0;
    }

    // get a new handle
    Kwave::Handle handle = newHandle();
    if (!handle) return 0; // out of handles :-(

    // try to allocate
    SwapFile *swap = new SwapFile(nextSwapFileName(handle));
    Q_ASSERT(swap);
    if (!swap) return 0; // out of memory :-(

    if (swap->allocate(size)) {
	// succeeded, store the object in our map
	m_unmapped_swap.insert(handle, swap);
	return handle;
    } else {
	qWarning("Kwave::MemoryManager::allocateVirtual(%u): OOM, "\
	         "(used: %uMB) - failed resizing swap file",
	         static_cast<unsigned int>(size), used);
	// failed: give up, delete the swapfile object
	delete swap;
    }

    return 0;
}

//***************************************************************************
bool Kwave::MemoryManager::convertToVirtual(Kwave::Handle handle,
                                            size_t new_size)
{
    // check: it must be in physical space, otherwise the rest makes no sense
    Q_ASSERT(m_physical.contains(handle));
    if (!m_physical.contains(handle)) return false;

    // get the old object in physical memory
    physical_memory_t mem = m_physical[handle];
    Q_ASSERT(mem.m_data);
    Q_ASSERT(mem.m_size);
    if (!mem.m_data || !mem.m_size) return false;

    // allocate a new object, including a new handle
    // if successful it has been stored in m_unmapped_swap
    Kwave::Handle temp_handle = allocateVirtual(new_size);
    if (!temp_handle) return false;

    // copy old stuff to new location
    SwapFile *swap = m_unmapped_swap[temp_handle];
    swap->write(0, mem.m_data, mem.m_size);

    // free the old physical memory
    ::free(mem.m_data);
    m_physical.remove(handle);

    // discard the new (temporary) handle and re-use the old one
    m_unmapped_swap.remove(temp_handle); // temp_handle is now no longer valid
    m_unmapped_swap.insert(handle, swap);

    // we now have the old data with new size and old handle in m_unmapped_swap
//     qDebug("Kwave::MemoryManager[%9d] - moved to swap", handle);

    dump("convertToVirtual");
    return true;
}

//***************************************************************************
bool Kwave::MemoryManager::convertToPhysical(Kwave::Handle handle,
                                             size_t new_size)
{
    Q_ASSERT(new_size);
    if (!new_size) return false;

    // check: it must be in physical space, otherwise the rest makes no sense
    Q_ASSERT(m_unmapped_swap.contains(handle));
    if (!m_unmapped_swap.contains(handle)) return false;
    SwapFile *swap = m_unmapped_swap[handle];
    Q_ASSERT(swap);
    if (!swap) return false;

    // allocate a new object, including a new handle
    // if successful it has been stored in m_physical
    Kwave::Handle temp_handle = allocatePhysical(new_size);
    if (!temp_handle) return false;

    physical_memory_t mem = m_physical[temp_handle];
    Q_ASSERT(mem.m_data);
    Q_ASSERT(mem.m_size >= new_size);
    if (!mem.m_data || !mem.m_size) {
	m_physical.remove(temp_handle);
	return false;
    }

    // copy old stuff to new location
    if (new_size <= swap->size()) {
	// shrinked
	swap->read(0, mem.m_data, new_size);
    } else {
	// grown
	swap->read(0, mem.m_data, swap->size());
    }

    // free the old swapfile
    m_unmapped_swap.remove(handle);
    delete swap;

    // discard the new (temporary) handle and re-use the old one
    m_physical.remove(temp_handle); // temp_handle is now no longer valid
    m_physical.insert(handle, mem);

    // we now have the old data with new size and old handle in m_physical
    qDebug("Kwave::MemoryManager[%9d] - reloaded %2u MB from swap",
           handle, static_cast<unsigned int>(mem.m_size >> 20));

    dump("convertToPhysical");
    return true;
}

//***************************************************************************
void Kwave::MemoryManager::tryToMakePhysical(Kwave::Handle handle)
{
    if (!handle) return;
    if (m_physical.contains(handle))    return; // already ok
    if (m_mapped_swap.contains(handle)) return; // not allowed
    if (m_cached_swap.contains(handle)) return; // already fast enough

    Q_ASSERT(m_unmapped_swap.contains(handle));
    if (!m_unmapped_swap.contains(handle)) return;

    const SwapFile *swap = m_unmapped_swap[handle];
    Q_ASSERT(swap);
    if (!swap) return;

    size_t size = swap->size();

    unsigned limit = totalPhysical();
    if (m_physical_limit < limit) limit = m_physical_limit;
    unsigned int used = physicalUsed();
    unsigned int available = (used < limit) ? (limit - used) : 0;

    // if we would go over the physical limit...
    if ((size >> 20) >= available) {
	// ...try to swap out some old stuff
	if (!freePhysical(size)) return;
    }

    // try to convert the swapfile back to physical RAM
    convertToPhysical(handle, size);
}

//***************************************************************************
bool Kwave::MemoryManager::resize(Kwave::Handle handle, size_t size)
{
    QMutexLocker lock(&m_lock);

//     qDebug("Kwave::MemoryManager[%9d] - resize to %u", handle,
//            static_cast<unsigned int>(size));

    // case 1: physical memory
    if (m_physical.contains(handle)) {
	const physical_memory_t phys_c = m_physical[handle];

	// check: it must not be mmapped!
	Q_ASSERT(!phys_c.m_mapcount);
	if (phys_c.m_mapcount) return false;

	// if we are increasing: check if we get too large
	size_t current_size = phys_c.m_size;
	if ((size > current_size) && (physicalUsed() +
	    ((size - current_size) >> 20) > m_physical_limit))
	{
	    // first try to swap out some old stuff
	    m_physical[handle].m_mapcount++;
	    bool physical_freed = freePhysical(size);
	    m_physical[handle].m_mapcount--;

	    if (!physical_freed) {
		// still too large -> move to virtual memory
		qDebug("Kwave::MemoryManager[%9d] - resize(%uMB) -> moving to swap",
		    handle, static_cast<unsigned int>(size >> 20));
		return convertToVirtual(handle, size);
	    }
	}

	// try to resize the physical memory
	physical_memory_t phys = m_physical[handle];
	void *old_block = phys.m_data;
	void *new_block = ::realloc(old_block, size);
	if (new_block) {
	    phys.m_data     = new_block;
	    phys.m_size     = size;
	    phys.m_mapcount = 0;
	    m_physical[handle] = phys;

	    dump("resize");
	    return true;
	} else {
	    // resizing failed, try to allocate virtual memory for it
	    return convertToVirtual(handle, size);
	}
    }

    // case 2: mapped swapfile in cache -> unmap !
    unmapFromCache(handle); // make sure it is not in the cache

    // case 3: mapped swapfile -> forbidden !
    Q_ASSERT(!m_mapped_swap.contains(handle));
    if (m_mapped_swap.contains(handle))
	return 0;

    // case 4: unmapped swapfile -> resize
    Q_ASSERT(m_unmapped_swap.contains(handle));
    if (m_unmapped_swap.contains(handle)) {

	// try to find space in the physical memory
	if ((physicalUsed() + (size >> 20) > m_physical_limit)) {
	    // free some space if necessary
	    if (freePhysical(size)) {
		if (convertToPhysical(handle, size)) return true;
	    }
	} else {
	    // try to convert into the currently available phys. RAM
	    if (convertToPhysical(handle, size)) return true;
	}

	// not enough free RAM: resize the pagefile
// 	qDebug("Kwave::MemoryManager[%9d] - resize swap %u -> %u MB",
// 	        handle,
// 	        static_cast<unsigned int>(swap->size() >> 20),
// 	        static_cast<unsigned int>(size >> 20));

	dump("resize");
	SwapFile *swap = m_unmapped_swap[handle];
	return swap->resize(size);
    }

    return false; // nothing known about this object / invalid handle?
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
	void *b = m_physical[handle].m_data;
	Q_ASSERT(b);
	m_physical.remove(handle);
	::free(b);
	handle = 0;

	dump("free");
	return;
    }

    Q_ASSERT(!m_mapped_swap.contains(handle));
    if (m_mapped_swap.contains(handle)) {
	// no-good: swapfile is still mapped !?
	unmap(handle);
    }

    unmapFromCache(handle); // make sure it is not in the cache

    if (m_unmapped_swap.contains(handle)) {
	// remove the pagefile
	SwapFile *swap = m_unmapped_swap[handle];
	m_unmapped_swap.remove(handle);
	Q_ASSERT(!swap->mapCount());
	delete swap;
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
    if (!handle) return 0; // object not found ?

    // try to convert to physical RAM
    tryToMakePhysical(handle);

    // simple case: physical memory does not really need to be mapped
    if (m_physical.contains(handle)) {
	m_physical[handle].m_mapcount++;
// 	qDebug("Kwave::MemoryManager[%9d] - mmap -> physical", handle);
	return m_physical[handle].m_data;
    }

    // no physical mem -> must be a swapfile

    // if it is already in the cache -> shortcut !
    if (m_cached_swap.contains(handle)) {
	SwapFile *swap = m_cached_swap[handle];
	m_cached_swap.remove(handle);
	m_mapped_swap.insert(handle, swap);
// 	qDebug("Kwave::MemoryManager[%9d] - mmap -> cache hit", handle);
	Q_ASSERT(swap->mapCount() == 1);
	return swap->address();
    }

    // other simple case: already mapped
    if (m_mapped_swap.contains(handle)) {
	SwapFile *swap = m_mapped_swap[handle];
	Q_ASSERT(swap->mapCount() >= 1);
// 	qDebug("Kwave::MemoryManager[%9d] - mmap -> recursive(%d)",
// 		handle, swap->mapCount());
	return swap->map(); // increase map count to 2...
    }

    // more complicated case: unmapped swapfile
    if (m_unmapped_swap.contains(handle)) {
	// map it into memory
	SwapFile *swap = m_unmapped_swap[handle];
	Q_ASSERT(!swap->mapCount());
	void *mapped = swap->map();
	if (!mapped) {
	    qDebug("Kwave::MemoryManager[%9d] - mmap FAILED", handle);
	    return 0;
	}

	// remember that we have mapped it, move the entry from the
	// "unmapped_swap" to the "mapped_swap" list
	m_unmapped_swap.remove(handle);
	m_mapped_swap.insert(handle, swap);

// 	qDebug("Kwave::MemoryManager[%9d] - mmap -> new mapping", handle);
	return mapped;
    } else {
	Q_ASSERT(m_unmapped_swap.contains(handle));
    }

    // nothing known about this object !?
    return 0;
}

//***************************************************************************
void Kwave::MemoryManager::unmapFromCache(Kwave::Handle handle)
{
    if (m_cached_swap.contains(handle)) {
// 	qDebug("Kwave::MemoryManager[%9d] - unmapFromCache", handle);
	SwapFile *swap = m_cached_swap[handle];
	Q_ASSERT(swap->mapCount() == 1);
	swap->unmap();
	Q_ASSERT(!swap->mapCount());
	m_cached_swap.remove(handle);
	m_unmapped_swap.insert(handle, swap);
    }

    dump("unmap");
}

//***************************************************************************
void Kwave::MemoryManager::unmap(Kwave::Handle handle)
{
    QMutexLocker lock(&m_lock);

    // simple case: physical memory does not really need to be unmapped
    if (m_physical.contains(handle)) {
// 	qDebug("Kwave::MemoryManager[%9d] - unmap -> physical", handle);
	Q_ASSERT(m_physical[handle].m_mapcount);
	if (m_physical[handle].m_mapcount)
	    m_physical[handle].m_mapcount--;
	return;
    }

//     qDebug("Kwave::MemoryManager[%9d] - unmap swap", handle);

    // just to be sure: should also not be in cache!
    Q_ASSERT(!m_cached_swap.contains(handle));
    unmapFromCache(handle);

    // unmapped swapfile: already unmapped !?
    if (m_unmapped_swap.contains(handle)) {
	Q_ASSERT(!m_unmapped_swap.contains(handle));
	return; // nothing to do
    }

    // must be a mapped swapfile: move it into the cache
    Q_ASSERT(m_mapped_swap.contains(handle));
    if (m_mapped_swap.contains(handle)) {
	SwapFile *swap = m_mapped_swap[handle];
	Q_ASSERT(swap->mapCount());
	if (swap->mapCount() > 1) {
	    // only unmap and internally reduce the map count
	    swap->unmap();
// 	    qDebug("Kwave::MemoryManager[%9d] - unmap -> recursive(%d)",
// 		    handle, swap->mapCount());
	} else if (swap->mapCount() == 1) {
	    // move to cache instead of really unmapping

	    // make room in the cache if necessary
	    while (m_cached_swap.count() >= CACHE_SIZE) {
		unmapFromCache(m_cached_swap.keys().first());
	    }

	    // move it into the swap file cache
	    m_mapped_swap.remove(handle);
	    m_cached_swap.insert(handle, swap);
// 	    qDebug("Kwave::MemoryManager[%9d] - unmap -> moved to cache",
// 	           handle);
	}
    }
}

//***************************************************************************
int Kwave::MemoryManager::readFrom(Kwave::Handle handle, unsigned int offset,
                                   void *buffer, unsigned int length)
{
    QMutexLocker lock(&m_lock);

    if (!handle) return 0;

    // try to convert to physical RAM
    tryToMakePhysical(handle);

    // simple case: physical memory -> memcpy(...)
    if (m_physical.contains(handle)) {
//  	qDebug("Kwave::MemoryManager[%9d] - readFrom -> physical", handle);
	char *data = reinterpret_cast<char *>(m_physical[handle].m_data);
        MEMCPY(buffer, data + offset, length);
        return length;
    }

    // no physical mem -> must be a swapfile

    // still in the cache and mapped -> memcpy(...)
    if (m_cached_swap.contains(handle)) {
	SwapFile *swap = m_cached_swap[handle];
	Q_ASSERT(swap->mapCount() == 1);
	char *data = reinterpret_cast<char *>(swap->address());
	Q_ASSERT(data);
	if (!data) return 0;
	MEMCPY(buffer, data + offset, length);
	qDebug("Kwave::MemoryManager[%9d] - readFrom -> cached swap", handle);
	return length;
    }

    // currently mmapped -> memcpy(...)
    if (m_mapped_swap.contains(handle)) {
	SwapFile *swap = m_mapped_swap[handle];
	Q_ASSERT(swap->mapCount() >= 1);
	char *data = reinterpret_cast<char *>(swap->address());
	Q_ASSERT(data);
	if (!data) return 0;
	MEMCPY(buffer, data + offset, length);
	qDebug("Kwave::MemoryManager[%9d] - readFrom -> mapped swap", handle);
	return length;
    }

    // now it must be in unmapped swap -> read(...)
    Q_ASSERT(m_unmapped_swap.contains(handle));
    if (m_unmapped_swap.contains(handle)) {
	qDebug("Kwave::MemoryManager[%9d] - readFrom -> unmapped swap", handle);
	SwapFile *swap = m_unmapped_swap[handle];
	length = swap->read(offset, buffer, length);
	return length;
    }

    return 0;
}

//***************************************************************************
int Kwave::MemoryManager::writeTo(Kwave::Handle handle, unsigned int offset,
                                  const void *buffer, unsigned int length)
{
    QMutexLocker lock(&m_lock);

    if (!handle) return 0;

    // try to convert to physical RAM
    tryToMakePhysical(handle);

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

    // make sure it's not mmapped
    unmapFromCache(handle);

    // writing to mapped swap is not allowed
    Q_ASSERT(!m_mapped_swap.contains(handle));
    if (m_mapped_swap.contains(handle)) {
        return 0;
    }

    // now it must be in unmapped swap
    Q_ASSERT(m_unmapped_swap.contains(handle));
    if (m_unmapped_swap.contains(handle)) {
	qDebug("Kwave::MemoryManager[%9d] - writeTo -> unmapped swap", handle);
	SwapFile *swap = m_unmapped_swap[handle];
	swap->write(offset, buffer, length);
	return length;
    }

    return 0;
}

//***************************************************************************
void Kwave::MemoryManager::dump(const char *function)
{
#if 0
    unsigned int v_used  = static_cast<unsigned int>(virtualUsed());
    unsigned int p_used  = static_cast<unsigned int>(physicalUsed());

    qDebug("------- %s -------", function);
    foreach (const Kwave::Handle &handle, m_physical.keys())
	qDebug("        P[%5u]: %5u", static_cast<unsigned int>(handle),
	                              m_physical[handle].m_size >> 20);

    unsigned int m = 0;
    foreach (const Kwave::Handle &handle, m_mapped_swap.keys()) {
	m += m_mapped_swap[handle]->size() >> 20;
	qDebug("        M[%5u]: %5u", static_cast<unsigned int>(handle),
	                              m_mapped_swap[handle]->size() >> 20);
    }

    unsigned int c = 0;
    foreach (const Kwave::Handle &handle, m_cached_swap.keys()) {
	c += m_cached_swap[handle]->size() >> 20;
	qDebug("        C[%5u]: %5u", static_cast<unsigned int>(handle),
	                              m_cached_swap[handle]->size() >> 20);
    }

    unsigned int u = 0;
    foreach (const Kwave::Handle &handle, m_unmapped_swap.keys()) {
	u += m_unmapped_swap[handle]->size() >> 20;
	qDebug("        U[%5u]: %5u", static_cast<unsigned int>(handle),
	                              m_unmapped_swap[handle]->size() >> 20);
    }

    qDebug("physical: %5u MB, virtual: %5u MB [m:%5u, c:%5u, u:%5u]",
           p_used, v_used, m, c, u);
#else
    Q_UNUSED(function);
#endif
}

//***************************************************************************
//***************************************************************************
