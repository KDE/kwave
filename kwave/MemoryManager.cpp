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

#include "KwaveApp.h"
#include "MemoryManager.h"
#include "SwapFile.h"

//***************************************************************************
MemoryManager::MemoryManager()
    :m_physical_allocated(0), m_physical_limit(0), m_virtual_allocated(0),
     m_virtual_limit(0), m_swap_dir("/tmp"), m_swap_files(),
     m_physical_size(), m_lock()
{

    m_physical_limit = totalPhysical();
}

//***************************************************************************
MemoryManager::~MemoryManager()
{
}

//***************************************************************************
void MemoryManager::close()
{
    // print warnings for each physical memory block

    // remove all remaining swap files and print warnings

}

//***************************************************************************
MemoryManager &MemoryManager::instance()
{
    return KwaveApp::memoryManager();
}

//***************************************************************************
void MemoryManager::setPhysicalLimit(unsigned int mb)
{
    MutexGuard lock(m_lock);

    m_physical_limit = mb;
    mb = totalPhysical();
    if (m_physical_limit > mb) m_physical_limit = mb;
}

//***************************************************************************
void MemoryManager::setVirtualLimit(unsigned int mb)
{
    MutexGuard lock(m_lock);

    m_virtual_limit = mb;
/** @todo write a function to find out the limit of virtual memory */
//    mb = totalVirtual();
//    if (m_virtual_limit > mb) m_virtual_limit = mb;
}

//***************************************************************************
void MemoryManager::setSwapDirectory(const QString &dir)
{
    MutexGuard lock(m_lock);
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
    if (getrlimit(RLIMIT_AS, &limit) == 0) {
	unsigned int total_ulimit = limit.rlim_cur >> 20;
	if (total_ulimit < total) total = total_ulimit;
    }
#endif

    return total;
}

//***************************************************************************
void *MemoryManager::allocate(size_t size)
{
    MutexGuard lock(m_lock);

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
    QMap<void*,SwapFile*>::Iterator it;
    for (it=m_swap_files.begin(); it != m_swap_files.end(); ++it) {
	Q_ASSERT(it.data());
	if (it.data()) used += (it.data()->size() >> 10) + 1;
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
void *MemoryManager::allocateVirtual(size_t size)
{
    // round up to whole pages
    if (size & (4096-1)) {
	size |= (4096-1);
	size++;
    }

    // check for limits
    unsigned int limit = 4096; // totalVirtual();
    if (m_virtual_limit < limit) limit = m_virtual_limit;
    unsigned int used = virtualUsed();
    unsigned int available = (used < limit) ? (limit-used) : 0;
    if ((size >> 20) >= available) return 0;

    // try to allocate
    SwapFile *swap = new SwapFile();
    Q_ASSERT(swap);
    if (!swap) return 0;

    void *block = swap->allocate(size, nextSwapFileName());
    if (block) {
	// succeeded, remember the block<->object in our map
	m_swap_files.insert(block, swap);
	return block;
    } else {
	delete swap;
    }

    return 0;
}

//***************************************************************************
void *MemoryManager::convertToVirtual(void *block, size_t old_size,
                                      size_t new_size)
{
    size_t current_size = m_physical_size[block];
    Q_ASSERT(current_size);
    void *new_block = allocateVirtual(new_size);
    if (!new_block) return 0;

    // copy old stuff to new location
    ::memcpy(new_block, block, old_size);

    // remove old memory
    m_physical_size[block] = 0;
    void *b = block;
    ::free(b);
    m_physical_size.remove(block);

    return new_block;
}

//***************************************************************************
void *MemoryManager::resize(void *block, size_t size)
{
    MutexGuard lock(m_lock);

    if (m_physical_size.contains(block)) {
	// if we are increasing: check if we get too large
	size_t current_size = m_physical_size[block];
	if ((size > current_size) && (physicalUsed() +
	    ((size - current_size) >> 20) > m_physical_limit))
	{
	    // too large -> move to virtual memory
	    debug("MemoryManager::resize(%uMB) -> moving to swap", size>>20);
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

    if (m_swap_files.contains(block)) {
	// resize the pagefile
	SwapFile *swap = m_swap_files[block];
	Q_ASSERT(swap);
	if (!swap) return 0;
	
	void *new_block = swap->resize(size);
	if (new_block) {
	    // update the map entry
	    m_swap_files.remove(block);
	    m_swap_files.insert(new_block, swap);
	    return new_block;
	}
    }

    return 0;
}

//***************************************************************************
void MemoryManager::free(void *&block)
{
    if (!block) return;
    MutexGuard lock(m_lock);

    if (m_swap_files.contains(block)) {
	// remove the pagefile
	SwapFile *swap = m_swap_files[block];
	Q_ASSERT(swap);
	if (swap) {
	    m_swap_files[block] = 0;
	    delete swap;
	}
	m_swap_files.remove(block);
    };

    if (m_physical_size.contains(block)) {
	// physical memory
	void *b = block;
	Q_ASSERT(b);
	m_physical_size[block] = 0;
	if (b) ::free(b);
	m_physical_size.remove(block);
    }

    block = 0;
}

//***************************************************************************
//***************************************************************************
