/***************************************************************************
             SwapFile.h  -  Provides virtual memory in a swap file
			     -------------------
    begin                : Fri Aug 17 2001
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

#ifndef SWAP_FILE_H
#define SWAP_FILE_H

#include "config.h"

#include <stdlib.h>  // for size_t

#include <QFile>
#include <QTemporaryFile>

class QString;

namespace Kwave
{

    class SwapFile
    {
    public:
	/**
	 * Constructor
	 * @param name full path to the swap file, actually a template
	 *             that <b>must</b> contain 6 "X" characters at the end!
	 */
	explicit SwapFile(const QString &name);

	/** Destructor */
	virtual ~SwapFile();

	/**
	 * Allocates virtual memory by creating an empty swap file.
	 * Must be mapped into memory before used.
	 *
	 * @param size number of bytes to allocate
	 * @return true if succeeded, false if failed
	 */
	bool allocate(size_t size);

	/**
	 * Returns the address of the allocated memory or 0 if
	 * nothing has been allocated.
	 */
	inline void *address() const   { return m_address; }

	/**
	 * Returns the size of the allocated memory or 0 if
	 * nothing has been allocated.
	 */
	inline size_t size() const     { return m_size; }

	/** returns the map count */
	inline int mapCount() const    { return m_map_count; }

	/**
	 * Returns the size of one storage unit in bytes
	 */
	inline size_t pagesize() const { return m_pagesize; }

	/**
	 * Resizes the allocated swap file.
	 * @param size the new size
	 * @return true if successful or false if failed
	 */
	bool resize(size_t size);

	/**
	 * Map the memory and return the physical address.
	 *
	 * @return pointer to the mapped area or null if failed
	 */
	void *map();

	/**
	 * Unmap a memory area, previously mapped with map()
	 *
	 * @return current reference count
	 */
	int unmap();

	/**
	 * Read bytes into a buffer
	 *
	 * @param offset offset within the file [bytes]
	 * @param buffer pointer to a buffer that is to be filled
	 * @param length number of bytes to read
	 * @return number of read bytes or < 0 if failed
	 */
	int read(unsigned int offset, void *buffer, unsigned int length);

	/**
	 * Write bytes from a buffer
	 *
	 * @param offset offset within the file [bytes]
	 * @param buffer pointer to a buffer with data
	 * @param length number of bytes to write
	 * @return number of written bytes or < 0 if failed
	 */
	int write(unsigned int offset, const void *buffer, unsigned int length);

    private:

	/**
	 * Frees the allocated memory by unmapping and deleting
	 * the swap file.
	 */
	void close();

    private:

	/** file used for swapping */
	QTemporaryFile m_file;

	/** address of the allocated virtual memory or 0 */
	void *m_address;

	/** number of allocated bytes or 0 */
	size_t m_size;

	/** size of one storage unit */
	size_t m_pagesize;

	/** reference count for mmap [0...N] */
	int m_map_count;

    };
}

#endif /* SWAP_FILE_H */

//***************************************************************************
//***************************************************************************
