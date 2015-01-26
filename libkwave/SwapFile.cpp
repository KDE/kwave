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

#include <config.h>

#include <limits.h>
#include <unistd.h>     // for unlink() and getpagesize()

#include "libkwave/String.h"
#include "libkwave/SwapFile.h"
#include "libkwave/Utils.h"

// just for debugging: number of open swapfiles
static unsigned int g_instances = 0;

/** minimum swap file size: 16 MB */
#define MINIMUM_SIZE (16 << 20)

/** size increment when resizing: 4 MB */
#define BLOCK_SIZE (4 << 20)

//***************************************************************************
Kwave::SwapFile::SwapFile(const QString &name)
    :m_file(name), m_address(0), m_size(0), m_pagesize(0), m_map_count(0)
{
    // determine the system's native page size
#if defined(HAVE_GETPAGESIZE)
    if (!m_pagesize) m_pagesize = getpagesize();
#endif
#if defined(HAVE_SYSCONF) && defined(_SC_PAGE_SIZE)
    if (!m_pagesize) m_pagesize = sysconf(_SC_PAGESIZE);
#endif

    // fallback: assume 4kB pagesize
    if (Kwave::toInt(m_pagesize) <= 0) {
	qWarning("SwapFile: unable to determine page size, using fallback");
	m_pagesize = (4 << 10);
    }

    g_instances++;
}

//***************************************************************************
Kwave::SwapFile::~SwapFile()
{
    close();
    g_instances--;
}

//***************************************************************************
bool Kwave::SwapFile::allocate(size_t size)
{
    Q_ASSERT(!m_address); // MUST NOT be mapped !
    if (m_address) return false;

    if (m_size) close();
//     qDebug("SwapFile::allocate(%u), instances: %u",
//            Kwave::toUint(size), g_instances);

    // try to create the temporary file
    if (!m_file.open()) {
	qWarning("SwapFile(%s) -> open failed", DBG(m_file.fileName()));
	return false;
    }

    // when it is created, also try to unlink it so that it will always
    // be removed, even if the application crashes !
#ifdef HAVE_UNLINK
    unlink(m_file.fileName().toLocal8Bit().data());
#endif /* HAVE_UNLINK */

    // round up the new size to a full page
    size_t rounded = Kwave::round_up<size_t>(size, m_pagesize);

    // touch each new page in order to *really* allocate the disk space
    size_t offset = 0;
    while (offset < rounded) {
	// 	qDebug("SwapFile: touching at offset 0x%08X",
	// 		Kwave::toUint(offset));
	m_file.seek(offset);
	m_file.putChar(0);
	m_file.flush();
	if (m_file.pos() != static_cast<qint64>(offset + 1)) {
	    qWarning("SwapFile::allocate(): seek failed. DISK FULL !?");
	    return false;
	}
	offset += m_pagesize;
    }

    m_file.seek(rounded - 1);
    m_file.putChar(0);
    m_file.flush();
    if (m_file.pos() + 1 < static_cast<qint64>(size)) {
	qWarning("SwapFile::allocate(%d MB) failed, DISK FULL ?",
	         Kwave::toUint(size >> 20));
	m_size = 0;
	return false;
    }

    // now the size is valid
    m_size = size;

//  qDebug("SwapFile::allocate(%d kB)",size >> 10);
    return true;
}

//***************************************************************************
bool Kwave::SwapFile::resize(size_t size)
{
    Q_ASSERT(!m_address); // MUST NOT be mappped !
    Q_ASSERT(!m_map_count);
    if (m_address || m_map_count) return false;
    if (size == m_size) return true; // nothing to do

    // special case: shutdown
    if (size == 0) {
	close();
	return true;
    }

    // this file seems to be a growing one:
    // round up the new size to a full block
    size_t rounded = Kwave::round_up<size_t>(size, BLOCK_SIZE);

    // optimization: if rounded size already matches -> done
    if (rounded == m_size) {
// 	qDebug("SwapFile::resize(%u MB) -> skipped, already big enough",
// 	       Kwave::toUint(size >> 20));
	return true;
    }

    // do not shrink below minimum size
    if ((size < m_size) && (size < MINIMUM_SIZE)) {
// 	qDebug("SwapFile::resize(%u MB) -> skipped, limited by min size",
// 	       Kwave::toUint(size >> 20));
	return true;
    }

    // resize the file
    //  qDebug("SwapFile::resize(%u)", size);

    // touch each new page in order to *really* allocate the disk space
    size_t offset = static_cast<size_t>((m_size + m_pagesize - 1) / m_pagesize);
    while (offset < rounded) {
// 	qDebug("SwapFile: touching at offset 0x%08X",
// 		Kwave::toUint(offset));
	m_file.seek(offset);
	m_file.putChar(0);
	m_file.flush();
	if (m_file.pos() != static_cast<qint64>(offset + 1)) {
	    qWarning("SwapFile::resize(): seek failed. DISK FULL !?");
	    return false;
	}
	offset += m_pagesize;
    }

    bool ok = true;
    ok &= m_file.resize(rounded);
    ok &= m_file.seek(rounded - 1);
    ok &= m_file.putChar(0);
    ok &= m_file.flush();
    if (ok) {
	m_size = rounded;
    } else {
	qWarning("SwapFile::resize(): failed. DISK FULL !?");
    }

//  qDebug("SwapFile::resize() to size=%u", m_size);
    return ok;
}

//***************************************************************************
void Kwave::SwapFile::close()
{
    Q_ASSERT(!m_map_count);
    if (m_address) m_file.unmap(static_cast<uchar *>(m_address));
    m_address = 0;
    m_size = 0;

    m_file.resize(0);
    if (m_file.isOpen()) m_file.close();

    if (m_file.exists(m_file.fileName())) {
	if (!m_file.remove()) {
	    qWarning("SwapFile(%s) -> remove FAILED", DBG(m_file.fileName()));
	}
    }

}

//***************************************************************************
void *Kwave::SwapFile::map()
{
//  qDebug("    SwapFile::map() - m_size=%u", m_size);

    // shortcut if already mapped
    if (m_map_count) {
	m_map_count++;
	Q_ASSERT(m_address);
	return m_address;
    }

    m_file.flush();

    m_address = m_file.map(0, m_size, QFile::NoOptions);

    // map -1 to null pointer
    if (m_address == reinterpret_cast<void *>(-1)) m_address = 0;

    // if succeeded, increase map reference counter
    if (m_address) {
	m_map_count++;
    } else {
	qWarning("SwapFile(%s) -> map FAILED", DBG(m_file.fileName()));
    }

    return m_address;
}

//***************************************************************************
int Kwave::SwapFile::unmap()
{
    Q_ASSERT(m_address);
    Q_ASSERT(m_size);
    Q_ASSERT(m_map_count);

    // first decrease refcount and do nothing if not zero
    if (!m_map_count) return 0;
    if (--m_map_count) return m_map_count;

    // really do the unmap
    if (m_size && m_address) {
//	qDebug("      --- SwapFile::unmap() (%p)", this);
	if (!m_file.unmap(static_cast<uchar *>(m_address))) {
	    qWarning("SwapFile(%s) -> unmap FAILED", DBG(m_file.fileName()));
	}
    }

    m_address = 0;
    return m_map_count;
}

//***************************************************************************
int Kwave::SwapFile::read(unsigned int offset, void *buffer,
                          unsigned int length)
{
    // seek to the given offset
    if (!m_file.seek(offset)) return -1;

    // read into the buffer
    if (!m_file.flush()) return -1;

    if (length > INT_MAX) length = INT_MAX;
    return Kwave::toInt(
	m_file.read(reinterpret_cast<char *>(buffer), length)
    );
}

//***************************************************************************
int Kwave::SwapFile::write(unsigned int offset, const void *buffer,
                           unsigned int length)
{
    // seek to the given offset
    if (!m_file.seek(offset)) return -1;

    // write data from the buffer
    if (length > INT_MAX) length = INT_MAX;
    return Kwave::toInt(
	m_file.write(reinterpret_cast<const char *>(buffer), length)
    );
}

//***************************************************************************
//***************************************************************************
