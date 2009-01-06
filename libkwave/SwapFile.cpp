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

#include <errno.h>
#include <stdio.h>      // for mkstemp (according to the man page)
#include <stdlib.h>     // for mkstemp (according to reality)
#include <unistd.h>     // for unlink()
#include <sys/mman.h>   // for mmap etc.

#include "SwapFile.h"

// just for debugging: number of open swapfiles
static unsigned int g_instances = 0;

//***************************************************************************
SwapFile::SwapFile()
    :m_file(), m_address(0), m_size(0), m_pagesize(1 << 16)
{
    // determine the system's native page size
    m_pagesize = 64*1024; // ### fake 64MB pagesize
    // ### MUST be a power of two !!!

    g_instances++;
}

//***************************************************************************
SwapFile::~SwapFile()
{
    close();
    g_instances--;
}

//***************************************************************************
static inline unsigned int round_up(unsigned int size, unsigned int units)
{
    unsigned int modulo = (size % units);
    if (modulo) size += (units-modulo);
    return size;
}

//***************************************************************************
bool SwapFile::allocate(size_t size, const QString &filename)
{
    Q_ASSERT(!m_address); // MUST NOT be mapped !
    if (m_address) return false;

    if (m_size) close();
//     qDebug("SwapFile::allocate(%u), instances: %u",
// 	   size, g_instances);

    // try to create the temporary file
    QByteArray name = QFile::encodeName(filename);
#ifdef HAVE_MKSTEMP
    int fd = mkstemp(name.data());
    if (fd < 0) {
	qDebug("SwapFile::allocate(%u) failed, instances: %u",
	       static_cast<unsigned int>(size), g_instances);
	return false;
    }
    m_file.open(fd, QIODevice::Unbuffered | QIODevice::ReadWrite);
#else /* HAVE_MKSTEMP */
    m_file.setFileName(filename);
    m_file.open(QIODevice::Unbuffered | QIODevice::ReadWrite);
#endif/* HAVE_MKSTEMP */

    // when it is created, also try to unlink it so that it will always
    // be removed, even if the application crashes !
#ifdef HAVE_UNLINK
    unlink(name.data());
#endif /* HAVE_UNLINK */

    m_file.seek(round_up(size, m_pagesize));
    if (static_cast<qint64>(m_file.pos() + 1) < static_cast<qint64>(size)) {
	qWarning("SwapFile::allocate(%d MB) failed, DISK FULL ?",
	         static_cast<unsigned int>(size >> 20));
	m_size = 0;
	return false;
    }
    m_file.putChar(0);

    // now the size is valid
    m_size = size;

//  qDebug("SwapFile::allocate(%d kB)",size >> 10);
    return true;
}

//***************************************************************************
SwapFile *SwapFile::resize(size_t size)
{
    Q_ASSERT(!m_address); // MUST NOT be mappped !
    if (m_address) return 0;
    if (size == m_size) return this; // nothing to do

    // special case: shutdown
    if (size == 0) {
	close();
	return this;
    }

    // round up the new size to a full page
    size = round_up(size, m_pagesize);

    // resize the file
    //  qDebug("SwapFile::resize(%u)", size);
    m_file.seek(size-1);
    if (static_cast<qint64>(m_file.pos()) == static_cast<qint64>(size - 1)) {
	if (size > m_size) {
	    // growing: mark the new "last byte"
	    m_file.putChar(0);
	} else {
	    // shrinking: only truncate the file
	    m_file.flush();
	    int res = ftruncate(m_file.handle(), size);
	    if (res) perror("ftruncate failed");
	}

	m_size = size;
    } else {
	qWarning("SwapFile::resize(): seek failed. DISK FULL !?");
	return 0;
    }

//  qDebug("SwapFile::resize() to size=%u", m_size);
    return this;
}

//***************************************************************************
void SwapFile::close()
{
    if (m_address) munmap(m_address, m_size);
    m_address = 0;
    m_size = 0;
    if (m_file.isOpen()) m_file.close();
    if (m_file.exists(m_file.fileName())) m_file.remove();
}

//***************************************************************************
void *SwapFile::map()
{
//  qDebug("    SwapFile::map() - m_size=%u", m_size);
    m_file.flush();

    m_address = mmap(0, m_size,
                     PROT_READ | PROT_WRITE, MAP_SHARED,
                     m_file.handle(), 0);
    if (m_address == reinterpret_cast<void *>(-1)) m_address = 0;

    return m_address;
}

//***************************************************************************
void *SwapFile::unmap()
{
    Q_ASSERT(m_address);
    Q_ASSERT(m_size);

    if (m_size && m_address) {
//	qDebug("      --- SwapFile::unmap() (%p)", this);
	munmap(m_address, m_size);
    }

    m_address = 0;
    return this;
}

//***************************************************************************
int SwapFile::read(unsigned int offset, void *buffer, unsigned int length)
{
    // seek to the given offset
    if (!m_file.seek(offset)) return -1;

    // read into the buffer
    m_file.flush();
    return m_file.read(reinterpret_cast<char *>(buffer), length);
}

//***************************************************************************
int SwapFile::write(unsigned int offset, const void *buffer,
                    unsigned int length)
{
    // seek to the given offset
    if (!m_file.seek(offset)) return -1;

    // write data from the buffer
    return m_file.write(reinterpret_cast<const char *>(buffer), length);
}

//***************************************************************************
//***************************************************************************
