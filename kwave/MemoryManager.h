/***************************************************************************
        MemoryManager.h  -  Manager for virtual and physical memory
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

#ifndef _MEMORY_MANAGER_H_
#define _MEMORY_MANAGER_H_

#include "config.h"
#include <stddef.h>  // for size_t

#include <qdir.h>
#include <qmap.h>
#include <qstring.h>
#include "mt/Mutex.h"
#include "mt/MutexGuard.h"

class SwapFile;

class MemoryManager
{
public:
    /** Constructor */
    MemoryManager();

    /** Destructor */
    virtual ~MemoryManager();

    /** Closes the memory manager and does cleanups at program shutdown */
    void close();

    /**
     * Gets a block of memory, either in physical memory or in a swap file
     * if there is not enough physical memory. If there is not enough
     * memory at all, the return value will be a null pointer.
     * @param size number of bytes to allocate
     */
    void *allocate(size_t size);

    /**
     * Resizes a block of memory to a new size. If the block will no longer
     * fit in physical memory, the block will be swapped out to a page file
     * and a pointer different to the passed one will be returned.
     * @param block pointer to the existing block
     * @param size new size of the block in bytes
     * @return the new memory block
     */
    void *resize(void *block, size_t size);

    /**
     * Frees a block of memory that has been previously allocated with the
     * allocate() function.
     * @param block reference to the pointer to the block to be freed. The
     *              pointer will be set to null afterwards.
     */
    void free(void *&block);

    /**
     * Sets the limit of physical memory that can be used.
     * @param mb number of whole megabytes of the limit
     */
    void setPhysicalLimit(unsigned int mb);

    /**
     * Sets the limit of virtual memory that can be used.
     * @param mb number of whole megabytes of the limit
     */
    void setVirtualLimit(unsigned int mb);

    /**
     * Sets the directory where swap files should be stored
     * @param dir directory
     */
    void setSwapDirectory(const QString &dir);

    /**
     * Returns the total amount of theoretically available physical
     * memory, as the minimum of the totally installed memory and
     * ulimit settings.
     */
    unsigned int totalPhysical();

    /**
     * Returns the global instance of the memory manager from the
     * KwaveApp.
     */
    static MemoryManager &instance();

protected:

    /** Returns the currently allocated physical memory */
    size_t physicalUsed();

    /** Returns the currently allocated virtual memory */
    size_t virtualUsed();

    /** Returns a new swap file name */
    QString nextSwapFileName();

    /** Tries to allocate physical memory */
    void *allocatePhysical(size_t size);

    /** Tries to allocate virtual memory */
    void *allocateVirtual(size_t size);

private:

    /** Currently allocated amount of physical memory */
    unsigned int m_physical_allocated;

    /** Limit of the physical memory */
    unsigned int m_physical_limit;

    /** Currently allocated amount of physical memory */
    unsigned int m_virtual_allocated;

    /** Limit of the cirtual memory, 0 = disabled */
    unsigned int m_virtual_limit;

    /** Path where to store swap files */
    QDir m_swap_dir;

    /**
     * Map for translating virtual memory addresses
     * into SwapFile objects.
     */
    QMap<void*, SwapFile*> m_swap_files;

    /**
     * Map for sizes of objects in physical memory.
     */
    QMap<void*, size_t> m_physical_size;

    /** Mutex for ensuring exclusive access */
    Mutex m_lock;

};

#endif /* _MEMORY_MANAGER_H_ */
