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
#include <sys/types.h>

#include <qfile.h>
#include <qfileinfo.h>
#include <qstring.h>

#ifdef HAVE_MEMINFO
#include <linux/kernel.h> // for struct sysinfo
#include <sys/sysinfo.h>  // for sysinfo()
#endif

#ifdef HAVE_GETRLIMIT
#include <sys/resource.h> // for getrlimit()
#endif

#include "libkwave/memcpy.h"
#include "KwaveApp.h"
#include "MemoryManager.h"
#include "SwapFile.h"

/** number of elements in the m_cached_swap list */
#define CACHE_SIZE 4

//***************************************************************************
MemoryManager::MemoryManager()
    :m_physical_allocated(0), m_physical_limit(0), m_virtual_allocated(0),
     m_virtual_limit(0), m_swap_dir("/tmp"), m_unmapped_swap(),
     m_mapped_swap(), m_cached_swap(), m_physical_size(), m_lock()
{
    // determine amount of physical memory
    m_physical_limit = totalPhysical();
}

//***************************************************************************
MemoryManager::~MemoryManager()
{
    close();
}

//***************************************************************************
void MemoryManager::close()
{
    // print warnings for each physical memory block
    Q_ASSERT(m_physical_size.isEmpty());

    // remove all remaining swap files and print warnings
    Q_ASSERT(m_cached_swap.isEmpty());
    Q_ASSERT(m_mapped_swap.isEmpty());
}

//***************************************************************************
MemoryManager &MemoryManager::instance()
{
    return KwaveApp::memoryManager();
}

//***************************************************************************
void MemoryManager::setPhysicalLimit(unsigned int mb)
{
    QMutexLocker lock(&m_lock);

    m_physical_limit = mb;
    mb = totalPhysical();
    if (m_physical_limit > mb) m_physical_limit = mb;
}

//***************************************************************************
void MemoryManager::setVirtualLimit(unsigned int mb)
{
    QMutexLocker lock(&m_lock);

    m_virtual_limit = mb;
/** @todo write a function to find out the limit of virtual memory */
//    mb = totalVirtual();
//    if (m_virtual_limit > mb) m_virtual_limit = mb;
}

//***************************************************************************
void MemoryManager::setSwapDirectory(const QString &dir)
{
    QMutexLocker lock(&m_lock);
    m_swap_dir = dir;
}

//***************************************************************************
unsigned int MemoryManager::totalPhysical()
{
    unsigned int total = 4096;

#ifdef HAVE_MEMINFO
    // get the physically installed memory
    unsigned int installed_physical = total;
    struct sysinfo info;

    sysinfo(&info);
    installed_physical = (info.totalram >> 20); // convert to megabytes
    if (installed_physical < total) total = installed_physical;
#endif

#ifdef HAVE_GETRLIMIT
    struct rlimit limit;

    // check ulimit of data segment size
    if (getrlimit(RLIMIT_DATA, &limit) == 0) {
	unsigned int physical_ulimit = limit.rlim_cur >> 20;
	if (physical_ulimit < total) total = physical_ulimit;
    }

    // check ulimit of resident set size
    if (getrlimit(RLIMIT_RSS, &limit) == 0) {
	unsigned int rss_ulimit = limit.rlim_cur >> 20;
	if (rss_ulimit < total) total = rss_ulimit;
    }

    // check ulimit of total (physical+virtual) system memory
#ifdef RLIMIT_AS
    if (getrlimit(RLIMIT_AS, &limit) == 0) {
	unsigned int total_ulimit = limit.rlim_cur >> 20;
	if (total_ulimit < total) total = total_ulimit;
    }
#endif
#endif

    return total;
}

//***************************************************************************
void *MemoryManager::allocate(size_t size)
{
    QMutexLocker lock(&m_lock);

    void *block = 0;
    block = allocatePhysical(size);
    if (!block) block = allocateVirtual(size);

    return block;
}

//***************************************************************************
void *MemoryManager::allocatePhysical(size_t size)
{
    // check for limits
    unsigned limit = totalPhysical();
    if (m_physical_limit < limit) limit = m_physical_limit;
    unsigned int used = physicalUsed();
    unsigned int available = (used < limit) ? (limit-used) : 0;
    if ((size >> 20) >= available) return 0;

    // try to allocate
    void *block = ::malloc(size);
    if (block) m_physical_size.insert(block, size);

    return block;
}

//***************************************************************************
size_t MemoryManager::physicalUsed()
{
    size_t used = 0;
    QMap<void*,size_t>::Iterator it;
    for (it=m_physical_size.begin(); it != m_physical_size.end(); ++it) {
	used += (it.data() >> 10) + 1;
    }
    return (used >> 10);
}

//***************************************************************************
size_t MemoryManager::virtualUsed()
{
    size_t used = 0;

    QPtrListIterator<SwapFile> it_c(m_cached_swap);
    for (; (it_c.current()); ++it_c) {
	used += (it_c.current()->size() >> 10) + 1;
    }

    QPtrListIterator<SwapFile> it_m(m_mapped_swap);
    for (; (it_m.current()); ++it_m) {
	used += (it_m.current()->size() >> 10) + 1;
    }

    QPtrListIterator<SwapFile> it_u(m_unmapped_swap);
    for (; (it_u.current()); ++it_u) {
	used += (it_u.current()->size() >> 10) + 1;
    }

    return (used >> 10);
}

//***************************************************************************
QString MemoryManager::nextSwapFileName()
{
    static unsigned int nr = 0;
    QFileInfo file;
    QString filename;

    filename = qApp->name();
    filename += "-";
    filename += QString::number(nr++);
    filename += "-";

    // these 6 chars are needed for mkstemp !
    filename += "XXXXXX";

    file.setFile(m_swap_dir, filename);
    return file.absFilePath();
}

//***************************************************************************
SwapFile *MemoryManager::allocateVirtual(size_t size)
{
    // check for limits
    unsigned int limit = 4096; // totalVirtual();
    if (m_virtual_limit < limit) limit = m_virtual_limit;
    unsigned int used = virtualUsed();
    unsigned int available = (used < limit) ? (limit-used) : 0;
    if ((size >> 20) >= available) {
	qDebug("MemoryManager::allocateVirtual(%u): out of memory, "\
	       "(used: %uMB, available: %uMB, limit=%uMB)",
	       (unsigned int)size, used, available, limit);
        return 0;
    }

    // try to allocate
    SwapFile *swap = new SwapFile();
    Q_ASSERT(swap);
    if (!swap) return 0;

    if (swap->allocate(size, nextSwapFileName())) {
	// succeeded, remember the block<->object in our map
	m_unmapped_swap.append(swap);
	return swap;
    } else {
	// failed: give up, delete the swapfile object
	delete swap;
    }

    return 0;
}

//***************************************************************************
void *MemoryManager::convertToVirtual(void *block, size_t old_size,
                                      size_t new_size)
{
    Q_ASSERT(m_physical_size.find(block) != 0);
    if (m_physical_size.find(block) == 0) return 0;

    SwapFile *new_swap = allocateVirtual(new_size);
    if (!new_swap) return 0;

    // copy old stuff to new location
    new_swap->write(0, block, old_size);

    // remove old memory
    m_physical_size.remove(block);
    ::free(block);

    return new_swap;
}

//***************************************************************************
void *MemoryManager::resize(void *block, size_t size)
{
    QMutexLocker lock(&m_lock);

    // case 1: physical memory
    if (m_physical_size.contains(block)) {
	// if we are increasing: check if we get too large
	size_t current_size = m_physical_size[block];
	if ((size > current_size) && (physicalUsed() +
	    ((size - current_size) >> 20) > m_physical_limit))
	{
	    // too large -> move to virtual memory
	    qDebug("MemoryManager::resize(%uMB) -> moving to swap",
	           (unsigned int)(size >> 20));
	    return convertToVirtual(block, current_size, size);
	}

	// try to resize the physical memory
	void *new_block = ::realloc(block, size);
	if (new_block) {
	    m_physical_size.remove(block);
	    m_physical_size.insert(new_block, size);
	    return new_block;
	} else {
	    // resizing failed, try to allocate virtual memory for it
	    return convertToVirtual(block, current_size, size);
	}
    }

    // case 2: mapped swapfile in cache -> unmap !
    unmapFromCache(block); // make sure it is not in the cache

    // case 3: mapped swapfile -> forbidden !
    Q_ASSERT(!m_mapped_swap.containsRef(reinterpret_cast<SwapFile *>(block)));
    if (m_mapped_swap.containsRef(reinterpret_cast<SwapFile *>(block)))
	return 0;

    // case 4: unmapped swapfile -> resize
    Q_ASSERT(m_unmapped_swap.containsRef(reinterpret_cast<SwapFile *>(block)));
    if (m_unmapped_swap.contains(reinterpret_cast<SwapFile *>(block))) {
	// resize the pagefile
	SwapFile *swap = reinterpret_cast<SwapFile *>(block);
	if (swap->resize(size))
	    return swap; // succeeded
    }

    return 0;
}

//***************************************************************************
void MemoryManager::free(void *&block)
{
    if (!block) return;
    QMutexLocker lock(&m_lock);

    Q_ASSERT(!m_mapped_swap.contains(reinterpret_cast<SwapFile *>(block)));
    if (m_mapped_swap.contains(reinterpret_cast<SwapFile *>(block))) {
	// no-good: swapfile is still mapped !?
	unmap(block);
    }

    unmapFromCache(block); // make sure it is not in the cache

    if (m_unmapped_swap.contains(reinterpret_cast<SwapFile *>(block))) {
	// remove the pagefile
	SwapFile *swap = reinterpret_cast<SwapFile *>(block);
	m_unmapped_swap.removeRef(swap);
	delete swap;
	block = 0;
    }

    if (m_physical_size.contains(block)) {
	// physical memory
	void *b = block;
	m_physical_size[block] = 0;
	::free(b);
	m_physical_size.remove(block);
    }

    block = 0;
}

//***************************************************************************
void *MemoryManager::map(void *block)
{
    Q_ASSERT(block);
    if (!block) return 0; // object not found ?

    // simple case: physical memory does not need to be mapped
    if (m_physical_size.contains(block)) return block;

//  qDebug("    MemoryManager::map(%p)", block);

    // if it is already in the cache -> shortcut !
    if (m_cached_swap.containsRef(reinterpret_cast<SwapFile *>(block))) {
	SwapFile *swap = reinterpret_cast<SwapFile *>(block);

	m_cached_swap.removeRef(swap);
	m_mapped_swap.prepend(swap);
//	qDebug("    MemoryManager::map(), cache hit! - %p", swap);
	return swap->address();
    }

    // other simple case: already mapped
    // DANGEROUS: we have no reference counting => forbid this!
    if (m_mapped_swap.containsRef(reinterpret_cast<SwapFile *>(block))) {
	Q_ASSERT(!m_mapped_swap.containsRef(reinterpret_cast<SwapFile *>(
	         block)));
	return 0;
    }

    // more complicated case: unmapped swapfile
    if (m_unmapped_swap.containsRef(reinterpret_cast<SwapFile *>(block))) {
	// locate the swapfile object
	SwapFile *swap = reinterpret_cast<SwapFile *>(block);

	// map it into memory
	void *mapped = swap->map();
	Q_ASSERT(mapped);
	if (!mapped) return 0;

	// remember that we have mapped it, move the entry from the
	// "unmapped_swap" to the "mapped_swap" list
	m_unmapped_swap.removeRef(swap);
	m_mapped_swap.append(swap);

	return mapped;
    } else {
	Q_ASSERT(m_unmapped_swap.containsRef(
	    reinterpret_cast<SwapFile *>(block)));
    }

    // nothing known about this object !?
    return 0;
}

//***************************************************************************
void MemoryManager::unmapFromCache(void *block)
{
    if (m_cached_swap.containsRef(reinterpret_cast<SwapFile *>(block))) {
	SwapFile *swap = reinterpret_cast<SwapFile *>(block);

	qDebug("    MemoryManager::unmapFromCache(%p)", block);
	swap->unmap();
	m_cached_swap.removeRef(swap);
	m_unmapped_swap.append(swap);
    }
}

//***************************************************************************
void MemoryManager::unmap(void *block)
{
    // simple case: physical memory does not need to be unmapped
    if (m_physical_size.contains(block)) return;

//  qDebug("    MemoryManager::unmap(%p)", block);

    // just to be sure: should also not be in cache!
//    Q_ASSERT(!m_cached_swap.containsRef(reinterpret_cast<SwapFile *>(block)));
    unmapFromCache(block);

    // unmapped swapfile: already unmapped !?
    if (m_unmapped_swap.contains(reinterpret_cast<SwapFile *>(block))) {
	Q_ASSERT(!m_unmapped_swap.contains(
	         reinterpret_cast<SwapFile *>(block)));
	return; // nothing to do
    }

    // mapped swapfile: move it into the cache
    if (m_mapped_swap.containsRef(reinterpret_cast<SwapFile *>(block))) {
	SwapFile *swap = reinterpret_cast<SwapFile *>(block);

	// make room in the cache if necessary
	while (m_cached_swap.count() >= CACHE_SIZE) {
	    unmapFromCache(m_cached_swap.first());
	}

	// move it into the swap file cache
	m_mapped_swap.removeRef(swap);
	m_cached_swap.append(swap);
    } else {
	Q_ASSERT(m_mapped_swap.containsRef(
	         reinterpret_cast<SwapFile *>(block)));
    }

}

//***************************************************************************
int MemoryManager::readFrom(void *block, unsigned int offset,
                            void *buffer, unsigned int length)
{
    Q_ASSERT(block);
    if (!block) return 0;

    // simple case: memcpy from physical memory
    if (m_physical_size.contains(block)) {
        MEMCPY(buffer, reinterpret_cast<char *>(block) + offset, length);
        return length;
    }

    // make sure it's not mmapped
    unmapFromCache(block);
    if (m_mapped_swap.containsRef(reinterpret_cast<SwapFile *>(block))) {
	Q_ASSERT(!m_mapped_swap.containsRef(
	          reinterpret_cast<SwapFile *>(block)));
        return 0;
    }

    // now it must be in unmapped swap
    Q_ASSERT(m_unmapped_swap.containsRef(reinterpret_cast<SwapFile *>(block)));
    if (!m_unmapped_swap.containsRef(reinterpret_cast<SwapFile *>(block))) {
	return 0;
    }

    SwapFile *swap = reinterpret_cast<SwapFile *>(block);
    swap->read(offset, buffer, length);

    return length;
}

//***************************************************************************
int MemoryManager::writeTo(void *block, unsigned int offset,
                           void *buffer, unsigned int length)
{
    Q_ASSERT(block);
    if (!block) return 0;

    // simple case: memcpy to physical memory
    if (m_physical_size.contains(block)) {
        MEMCPY(reinterpret_cast<char *>(block) + offset, buffer, length);
        return length;
    }

    // make sure it's not mmapped
    unmapFromCache(block);
    if (m_mapped_swap.containsRef(reinterpret_cast<SwapFile *>(block))) {
	Q_ASSERT(m_mapped_swap.containsRef(
	         reinterpret_cast<SwapFile *>(block)));
        return 0;
    }

    // now it must be in unmapped swap
    Q_ASSERT(m_unmapped_swap.containsRef(reinterpret_cast<SwapFile *>(block)));
    if (!m_unmapped_swap.containsRef(reinterpret_cast<SwapFile *>(block))) {
	return 0;
    }

    SwapFile *swap = reinterpret_cast<SwapFile *>(block);
    swap->write(offset, buffer, length);

    return length;
}

//***************************************************************************
//***************************************************************************
