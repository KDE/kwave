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
#include <qfile.h>

class QString;

class SwapFile
{
public:
    /** Constructor */
    SwapFile();

    /** Destructor */
    virtual ~SwapFile();

    /**
     * Allocates virtual memory by creating a swap file and
     * mapping it into cpu memory space with <c>mmap</c>.
     * @param size number of bytes to allocate
     * @param filename full path to the swap file, actually a template
     *                 that <b>must</b> contain 6 "X" characters at the end!
     * @return pointer to the allocated memory or 0 if failed
     */
    void *allocate(size_t size, const QString &filename);

    /**
     * Returns the address of the allocated memory or 0 if
     * nothing has been allocated.
     */
    inline void *address() { return m_address; };

    /**
     * Returns the size of the allocated memory or 0 if
     * nothing has been allocated.
     */
    inline size_t size()   {return m_size; };

    /**
     * Resizes the allocated swap file.
     * @param size the new size
     * @return pointer to new resized storage or 0 if failed
     */
    void *resize(size_t size);

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

    /** Number of alocated bytes or 0 */
    size_t m_size;

};

#endif /* _SWAP_FILE_H_ */
