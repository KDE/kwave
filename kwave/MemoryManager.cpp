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

#ifdef HAVE_MEMINFO
#include <linux/kernel.h> // for struct sysinfo
#include <sys/sysinfo.h>  // for sysinfo()
#endif

#ifdef HAVE_GETRLIMIT
#include <sys/resource.h> // for getrlimit()
#endif

#include "KwaveApp.h"
#include "MemoryManager.h"

//***************************************************************************
MemoryManager::MemoryManager()
    :m_physical_allocated(0), m_physical_limit(0), m_virtual_allocated(0),
     m_virtual_limit(0), m_swap_dir("/tmp"), m_lock()
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
    unsigned int used = 0; // ### physicalUsed();
    unsigned int available = (used < limit) ? (limit-used) : 0;
    if (size >= (available << 20)) return 0;

    // try to allocate
    void *block = ::malloc(size);

    return block;
}

//***************************************************************************
void *MemoryManager::allocateVirtual(size_t size)
{
    // check for limits
    unsigned int limit = 4096; // totalVirtual();
    if (m_virtual_limit < limit) limit = m_virtual_limit;
    unsigned int used = 0; // ### virtualUsed();
    unsigned int available = (used < limit) ? (limit-used) : 0;
    if (size >= (available << 20)) return 0;

    // try to allocate

    return 0;
}

//***************************************************************************
unsigned int MemoryManager::resize(void *&block, size_t size)
{
    MutexGuard lock(m_lock);

    void *n = ::realloc(block, size);
    if (n) {
	block = n;
	return size;
    } else {
	return 0;
    }
}

//***************************************************************************
void MemoryManager::free(void *&block)
{
    MutexGuard lock(m_lock);

    if (block) ::free(block);
    block = 0;
}

//***************************************************************************
//***************************************************************************
