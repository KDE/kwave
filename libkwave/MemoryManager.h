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

#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "config.h"
#include <stddef.h>  // for size_t

#include <QtCore/QDir>
#include <QtCore/QHash>
#include <QtCore/QMutex>
#include <QtCore/QString>

#include <kdemacros.h>

#include "libkwave/LRU_Cache.h"

namespace Kwave
{
    class SwapFile;

    /** handle for memory manager */
    typedef int Handle;

    class KDE_EXPORT MemoryManager
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
	 * @return handle of a storage object, to be used to be mapped
	 *         into physical memory through map() or zero if out of memory
	 */
	Kwave::Handle allocate(size_t size) KDE_EXPORT;

	/**
	 * Resizes a block of memory to a new size. If the block will no longer
	 * fit in physical memory, the block will be swapped out to a page file.
	 *
	 * @param handle handle of the existing block
	 * @param size new size of the block in bytes
	 * @return true if successful, false if out of memory or if the
	 *         block is currently in use
	 */
	bool resize(Kwave::Handle handle, size_t size) KDE_EXPORT;

	/**
	 * Returns the allocated size of the block
	 * @note this may be more than allocated, can be rounded up
	 */
	size_t sizeOf(Kwave::Handle handle) KDE_EXPORT;

	/**
	 * Frees a block of memory that has been previously allocated with the
	 * allocate() function.
	 *
	 * @param handle reference to the handle to the block to be freed. The
	 *               handle will be set to zero afterwards.
	 */
	void free(Kwave::Handle &handle) KDE_EXPORT;

	/**
	 * Sets the limit of physical memory that can be used.
	 * @param mb number of whole megabytes of the limit
	 */
	void setPhysicalLimit(quint64 mb) KDE_EXPORT;

	/**
	 * Sets the limit of virtual memory that can be used.
	 * @param mb number of whole megabytes of the limit
	 */
	void setVirtualLimit(quint64 mb) KDE_EXPORT;

	/**
	 * Sets the directory where swap files should be stored
	 * @param dir directory
	 */
	void setSwapDirectory(const QString &dir) KDE_EXPORT;

	/**
	 * Sets the limit of memory that can be used for undo/redo.
	 * @param mb number of whole megabytes of the limit
	 */
	void setUndoLimit(quint64 mb) KDE_EXPORT;

	/**
	 * Returns the limit of memory that can be used for undo/redo
	 * in units of whole megabytes
	 */
	quint64 undoLimit() const KDE_EXPORT;

	/**
	 * Returns the total amount of theoretically available physical
	 * memory, as the minimum of the totally installed memory and
	 * ulimit settings.
	 */
	quint64 totalPhysical() KDE_EXPORT;

	/**
	 * Returns the global instance of the memory manager from the
	 * KwaveApp.
	 */
	static MemoryManager &instance() KDE_EXPORT;

	/**
	 * Map a portion of memory and return the physical address.
	 *
	 * @param handle handle of the object that identifies the
	 *        memory block
	 * @return pointer to the mapped area or null if failed
	 */
	void *map(Kwave::Handle handle) KDE_EXPORT;

	/**
	 * Unmap a memory area, previously mapped with map()
	 *
	 * @param handle handle of a mapped block mapped with map()
	 */
	void unmap(Kwave::Handle handle) KDE_EXPORT;

	/**
	 * Read from a memory block into a buffer
	 *
	 * @param handle handle of the object that identifies the
	 *        memory block
	 * @param offset offset within the object [bytes]
	 * @param buffer pointer to a buffer that is to be filled
	 * @param length number of bytes to read
	 * @return number of read bytes or < 0 if failed
	 */
	int readFrom(Kwave::Handle handle, unsigned int offset,
	             void *buffer, unsigned int length) KDE_EXPORT;

	/**
	 * Write a buffer into a memory block
	 *
	 * @param handle handle of the object that identifies the
	 *        memory block
	 * @param offset offset within the object [bytes]
	 * @param buffer pointer to a buffer that is to be written
	 * @param length number of bytes to write
	 * @return number of written bytes or < 0 if failed
	 */
	int writeTo(Kwave::Handle handle, unsigned int offset,
		    const void *buffer, unsigned int length) KDE_EXPORT;

#ifdef DEBUG_MEMORY
	/** structure used for statistics */
	typedef struct {
	    /** physical memory */
	    struct {
		quint64 handles; /**< used handles     */
		quint64 bytes;   /**< allocated bytes  */
		quint64 limit;   /**< maximum allowed  */
		quint64 allocs;  /**< number of allocs */
		quint64 frees;   /**< number of frees  */
	    } physical;

	    /** swap files */
	    struct {
		/** mapped swap files */
		struct {
		    quint64 handles; /**< used handles    */
		    quint64 bytes;   /**< allocated bytes */
		} mapped;
		/** swap files mapped (cached) */
		struct {
		    quint64 handles; /**< used handles    */
		    quint64 bytes;   /**< mapped bytes    */
		} cached;
		/** unmapped swap files */
		struct {
		    quint64 handles; /**< used handles    */
		    quint64 bytes;   /**< allocated bytes */
		} unmapped;
		quint64 limit;  /**< maximum allowed  */
		quint64 allocs; /**< number of allocs */
		quint64 frees;  /**< number of frees  */
	    } swap;
	} statistic_t;
#endif

    protected:

	/** returns the currently allocated physical memory */
	quint64 physicalUsed() KDE_NO_EXPORT;

	/** returns the currently allocated virtual memory */
	quint64 virtualUsed() KDE_NO_EXPORT;

	/** returns a new swap file name */
	QString nextSwapFileName(Kwave::Handle handle) KDE_NO_EXPORT;

	/** convert a physical memory block into a new larger pagefile */
	bool convertToVirtual(Kwave::Handle handle, size_t new_size) KDE_NO_EXPORT;

	/** convert a swapfile into a physical memory block */
	bool convertToPhysical(Kwave::Handle handle, size_t new_size) KDE_NO_EXPORT;

	/** tries to allocate physical memory */
	Kwave::Handle allocatePhysical(size_t size) KDE_NO_EXPORT;

	/** tries to convert to physical RAM */
	void tryToMakePhysical(Kwave::Handle handle);

	/**
	 * Tries to allocate virtual memory
	 *
	 * @param size number of bytes to allocate
	 * @return handle of a SwapFile object or zero if failed
	 */
	Kwave::Handle allocateVirtual(size_t size) KDE_NO_EXPORT;

    private:

	/**
	 * get a new handle.
	 * @note the handle does not need to be freed later
	 * @return a non-zero handle or zero if all handles are in use
	 */
	Kwave::Handle newHandle();

	/**
	 * try to make some room in the physical memory area by kicking
	 * out the oldest entries to swap if possible.
	 *
	 * @param size number of bytes to free
	 * @return true if successful, false if failed
	 */
	bool freePhysical(size_t size);

	/**
	 * Makes sure that the object is not a swapfile in cache. If so,
	 * it will be unmapped and moved to the m_unmapped_swap list.
	 *
	 * @param handle handle of a block in m_cached_swap
	 */
	void unmapFromCache(Kwave::Handle handle) KDE_NO_EXPORT;

	/** dump current state (for debugging) */
	void dump(const char *function);

    private:

	typedef struct physical_memory_t {
	    void  *m_data;     /**< pointer to the physical memory */
	    size_t m_size;     /**< size of the block */
	    int    m_mapcount; /**< counter for mmap */
	} physical_memory_t;

    private:

	/** limit of the physical memory */
	quint64 m_physical_limit;

	/** total maximum physical memory (system dependent) */
	quint64 m_physical_max;

	/** limit of the virtual memory, 0 = disabled */
	quint64 m_virtual_limit;

	/** path where to store swap files */
	QDir m_swap_dir;

	/** limit of memory available for undo/redo */
	quint64 m_undo_limit;

	/** map of objects in physical memory */
	Kwave::LRU_Cache<Kwave::Handle, physical_memory_t> m_physical;

	/** map of swapfile objects that are not mapped into memory */
	QHash<Kwave::Handle, Kwave::SwapFile *> m_unmapped_swap;

	/** map of swapfile objects that are already mapped into memory */
	QHash<Kwave::Handle, Kwave::SwapFile *> m_mapped_swap;

	/**
	 * cache for swapfiles that have been recently used, are mapped
	 * and get unmapped if the queue is full. The queue will be used
	 * as a FIFO with fixed size.
	 */
	QHash<Kwave::Handle, Kwave::SwapFile *> m_cached_swap;

	/** mutex for ensuring exclusive access */
	QMutex m_lock;

	/** last used handle */
	static Kwave::Handle m_last_handle;

#ifdef DEBUG_MEMORY
	/** statistics, for debugging */
	statistic_t m_stats;
#endif /* DEBUG_MEMORY */
    };

}

#endif /* MEMORY_MANAGER_H */

//***************************************************************************
//***************************************************************************
