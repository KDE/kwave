/***************************************************************************
           SwapFile.cpp  -  Provides virtual memory in a swap file
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

#include "config.h"
#include <stdio.h>      // for mkstemp (according to the man page)
#include <stdlib.h>     // for mkstemp (according to reality)
#include <unistd.h>     // for unlink()
#include <sys/mman.h>   // for mmap etc.

#include <qcstring.h>
#include <qfile.h>

#include "SwapFile.h"

//***************************************************************************
SwapFile::SwapFile()
    :m_file(), m_address(0), m_size(0)
{
}

//***************************************************************************
SwapFile::~SwapFile()
{
    close();
}

//***************************************************************************
void *SwapFile::allocate(size_t size, const QString &filename)
{
    if (m_address || m_size) close();

    // try to create the temporary file
    // when it is created, also try to unlink it so that it will always
    // be removed, even if the application crashes !
#ifdef HAVE_MKSTEMP
    char *name = qstrdup(filename.local8Bit());
    int fd = mkstemp(name);
    ASSERT(fd >= 0);
    if (fd < 0) return 0;
    m_file.open(IO_Raw | IO_WriteOnly, fd);
#ifdef HAVE_UNLINK
    unlink(name);
#endif /* HAVE_UNLINK */
    if (name) delete name;
#else /* HAVE_MKSTEMP */
    m_file.setName(filename);
    m_file.open(IO_Raw | IO_WriteOnly);
#ifdef HAVE_UNLINK
    unlink(filename);
#endif /* HAVE_UNLINK */
#endif/* HAVE_MKSTEMP */

    m_file.at(size+4096);
    m_file.putch(0);

    m_address = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED,
	m_file.handle(), 0);
    if (m_address == (void*)(-1)) m_address = 0;
    if (m_address) m_size = size;

    debug("SwapFile::allocate(%d MB) at %p",size>>20, m_address);
    return m_address;
}

//***************************************************************************
void *SwapFile::resize(size_t size)
{
    ASSERT(m_address);
    if (!m_address) return 0;
    if (size == m_size) return m_address; // nothing to do

    if (size == 0) {
	close();
	return 0;
    }

    // unmap the old memory
    munmap(m_address, m_size);

    // resize the file
    debug("SwapFile::resize(%u)", size);

    if (lseek(m_file.handle(), size, SEEK_SET) > 0) {
	ftruncate(m_file.handle(), size);
	
	static char dummy = 0;
	write(m_file.handle(), &dummy, 1);
	
	debug("SwapFile::resize(): seek was ok");
	m_size = size;
    } else {
	debug("SwapFile::resize(): seek failed !");
	size = 0;
    }

    // re-map the memory
    m_address = mmap(0, m_size, PROT_READ | PROT_WRITE, MAP_SHARED,
	m_file.handle(), 0);
    ASSERT(m_address);
    if (!m_address) m_size = 0;
    debug("SwapFile::resize(): new area mmapped to %p", m_address);

    return (size) ? m_address : 0;
}

//***************************************************************************
void SwapFile::close()
{
    if (m_address) munmap(m_address, m_size);
    m_address = 0;
    m_size = 0;
    if (m_file.isOpen()) m_file.close();
    if (m_file.exists()) m_file.remove();
}

//***************************************************************************
//***************************************************************************
