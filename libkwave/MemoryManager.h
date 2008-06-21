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

#include <QDir>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QString>

#include <kdemacros.h>

class SwapFile;

class MemoryManager
{
public:
    /** Constructor */
    MemoryManager() KDE_NO_EXPORT;

    /** Destructor */
    virtual ~MemoryManager() KDE_NO_EXPORT;

    /** Closes the memory manager and does cleanups at program shutdown */
    void close() KDE_NO_EXPORT;

    /**
     * Gets a block of memory, either in physical memory or in a swap file
     * if there is not enough physical memory. If there is not enough
     * memory at all, the return value will be a null pointer.
     *
     * @param size number of bytes to allocate
     * @return pointer to a storage object, to be used to be mapped
     *         into physical memory through map()
     */
    void *allocate(size_t size) KDE_EXPORT;

    /**
     * Resizes a block of memory to a new size. If the block will no longer
     * fit in physical memory, the block will be swapped out to a page file
     * and a pointer different to the passed one will be returned.
     * @param block pointer to the existing block
     * @param size new size of the block in bytes
     * @return the new memory block
     */
    void *resize(void *block, size_t size) KDE_EXPORT;

    /**
     * Frees a block of memory that has been previously allocated with the
     * allocate() function.
     * @param block reference to the pointer to the block to be freed. The
     *              pointer will be set to null afterwards.
     */
    void free(void *&block) KDE_EXPORT;

    /**
     * Sets the limit of physical memory that can be used.
     * @param mb number of whole megabytes of the limit
     */
    void setPhysicalLimit(unsigned int mb) KDE_EXPORT;

    /**
     * Sets the limit of virtual memory that can be used.
     * @param mb number of whole megabytes of the limit
     */
    void setVirtualLimit(unsigned int mb) KDE_EXPORT;

    /**
     * Sets the directory where swap files should be stored
     * @param dir directory
     */
    void setSwapDirectory(const QString &dir) KDE_EXPORT;

    /**
     * Returns the total amount of theoretically available physical
     * memory, as the minimum of the totally installed memory and
     * ulimit settings.
     */
    unsigned int totalPhysical() KDE_EXPORT;

    /**
     * Returns the global instance of the memory manager from the
     * KwaveApp.
     */
    static MemoryManager &instance() KDE_EXPORT;

    /**
     * Map a portion of memory and return the physical address.
     *
     * @param block pointer to the object that identifies the
     *        memory block
     * @return pointer to the mapped area or null if failed
     */
    void *map(void *block) KDE_EXPORT;

    /**
     * Unmap a memory area, previously mapped with map()
     *
     * @param block pointer to the previously mapped block
     */
    void unmap(void *block) KDE_EXPORT;

    /**
     * Read from a memory block into a buffer
     *
     * @param block pointer to the object that identifies the
     *        memory block
     * @param offset offset within the object [bytes]
     * @param buffer pointer to a buffer that is to be filled
     * @param length number of bytes to read
     * @return number of read bytes or < 0 if failed
     */
    int readFrom(void *block, unsigned int offset,
                 void *buffer, unsigned int length) KDE_EXPORT;

    /**
     * Write a buffer into a memory block
     *
     * @param block pointer to the object that identifies the
     *        memory block
     * @param offset offset within the object [bytes]
     * @param buffer pointer to a buffer that is to be written
     * @param length number of bytes to write
     * @return number of written bytes or < 0 if failed
     */
    int writeTo(void *block, unsigned int offset,
                const void *buffer, unsigned int length) KDE_EXPORT;

protected:

    /** Returns the currently allocated physical memory */
    size_t physicalUsed() KDE_NO_EXPORT;

    /** Returns the currently allocated virtual memory */
    size_t virtualUsed() KDE_NO_EXPORT;

    /** Returns a new swap file name */
    QString nextSwapFileName() KDE_NO_EXPORT;

    /** Convert a physical memory block into a new larger pagefile */
    void *convertToVirtual(void *block, size_t old_size, size_t new_size)
	KDE_NO_EXPORT;

    /** Tries to allocate physical memory */
    void *allocatePhysical(size_t size) KDE_NO_EXPORT;

    /**
     * Tries to allocate virtual memory
     *
     * @param size number of bytes to allocate
     * @return pointer to a SwapFile object or null if failed
     */
    SwapFile *allocateVirtual(size_t size) KDE_NO_EXPORT;

private:

    /**
     * Makes sure that the object is not a swapfile in cache. If so,
     * it will be unmapped and moved to the m_unmapped_swap list.
     *
     * @param block pointer to a block
     */
    void unmapFromCache(void *block) KDE_NO_EXPORT;

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

    /** List of swapfile objects that are not mapped into memory */
    QList<SwapFile *> m_unmapped_swap;

    /** List of swapfile objects that are already mapped into memory */
    QList<SwapFile *> m_mapped_swap;

    /**
     * Cache for swapfiles that have been recently used, are mapped
     * and get unmapped if the queue is full. The queue will be used
     * as a FIFO with fixed size.
     */
    QList<SwapFile *> m_cached_swap;

    /**
     * Map for sizes of objects in physical memory.
     */
    QMap<void*, size_t> m_physical_size;

    /** Mutex for ensuring exclusive access */
    QMutex m_lock;

    /** pagesize of the system, used for mmapping files */
    unsigned int m_pagesize;

};

#endif /* _MEMORY_MANAGER_H_ */
