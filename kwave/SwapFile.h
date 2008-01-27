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

#ifndef _SWAP_FILE_H_
#define _SWAP_FILE_H_

#include "config.h"
#include <sys/types.h>  // for size_t

#include <QFile>

class MemoryManager;
class QString;

class SwapFile
{
public:
    /** Constructor */
    SwapFile();

    /** Destructor */
    virtual ~SwapFile();

    /**
     * Allocates virtual memory by creating an empty swap file.
     * Must be mapped into memory before used.
     *
     * @param size number of bytes to allocate
     * @param filename full path to the swap file, actually a template
     *                 that <b>must</b> contain 6 "X" characters at the end!
     * @return true if succeeded, false if failed
     */
    bool allocate(size_t size, const QString &filename);

    /**
     * Returns the address of the allocated memory or 0 if
     * nothing has been allocated.
     */
    inline void *address()   { return m_address; };

    /**
     * Returns the size of the allocated memory or 0 if
     * nothing has been allocated.
     */
    inline size_t size()     { return m_size; };

    /**
     * Returns the size of one storage unit in bytes
     */
    inline size_t pagesize() { return m_pagesize; };

    /**
     * Resizes the allocated swap file.
     * @param size the new size
     * @return pointer to this object or 0 if failed
     */
    SwapFile *resize(size_t size);

protected:

    friend class MemoryManager;

    /**
     * Map the memory and return the physical address.
     *
     * @return pointer to the mapped area or null if failed
     */
    void *map();

    /**
     * Unmap a memory area, previously mapped with map()
     *
     * @param block pointer to the previously mapped block
     * @param pointer to this
     */
    void *unmap();

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

    /** File used for swapping */
    QFile m_file;

    /** Address of the allocated virtual memory or 0 */
    void *m_address;

    /** Number of allocated bytes or 0 */
    size_t m_size;

    /** size of one storage unit */
    size_t m_pagesize;

};

#endif /* _SWAP_FILE_H_ */
