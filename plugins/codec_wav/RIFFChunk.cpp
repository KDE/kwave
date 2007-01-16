/*************************************************************************
          RIFFChunk.cpp  -  information about a RIFF chunk
                             -------------------
    begin                : Tue Mar 20 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
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

#include <qptrlist.h>
#include <qstring.h>

#include "RIFFChunk.h"

//***************************************************************************
RIFFChunk::RIFFChunk(RIFFChunk *parent, const QCString &name,
                     const QCString &format, u_int32_t length,
                     u_int32_t phys_offset,  u_int32_t phys_length)
    :m_type(Sub), m_name(name), m_format(format), m_parent(parent),
     m_chunk_length(length), m_phys_offset(phys_offset),
     m_phys_length(phys_length), m_sub_chunks()
{
}

//***************************************************************************
RIFFChunk::~RIFFChunk()
{
    m_sub_chunks.setAutoDelete(true);
    m_sub_chunks.clear();
}

//***************************************************************************
#define CHECK(x) Q_ASSERT(!(x)); if (x) return false;
bool RIFFChunk::isSane()
{
    CHECK(m_type == Empty);
    CHECK(m_type == Garbage);
    CHECK((m_type == Main) && m_sub_chunks.isEmpty());
    CHECK((m_type == Root) && m_sub_chunks.isEmpty());

    if (m_phys_length & 0x1) {
	// size is not an even number: no criterium for insanity
	// but worth a warning
	qWarning("%s: physical length is not an even number: %u",
	        path().data(), m_phys_length);
    }

    unsigned int datalen = dataLength();
    if (m_type == Main) datalen += 4;
    if ((datalen+1 < m_phys_length) || (datalen > m_phys_length)) {
	qWarning("%s: dataLength=%u, phys_length=%u",
	         path().data(), datalen, m_phys_length);
	return false;
    }

    QPtrListIterator<RIFFChunk> it(subChunks());
    for (; it.current(); ++it) {
	if (!it.current()->isSane()) return false;
    }
    return true;
}

//***************************************************************************
u_int32_t RIFFChunk::physEnd()
{
    u_int32_t end = m_phys_offset + m_phys_length;
    if (m_phys_length) --end;
    if ((m_type != Root) && (m_type != Garbage)) end += 8;
    return end;
}

//***************************************************************************
u_int32_t RIFFChunk::dataStart()
{
    return m_phys_offset + ((m_type == Main) ? 12 : 8);
}

//***************************************************************************
u_int32_t RIFFChunk::dataLength()
{
    return m_chunk_length - ((m_type == Main) ? 4 : 0);
}

//***************************************************************************
void RIFFChunk::setLength(u_int32_t length)
{
    m_chunk_length = length;
    m_phys_length  = length;
}

//***************************************************************************
bool RIFFChunk::isChildOf(RIFFChunk *chunk)
{
    if (!chunk) return (m_type == Root); // only root has null as parent
    if (chunk == m_parent) return true;
    if (m_parent) return m_parent->isChildOf(chunk);
    return false;
}

//***************************************************************************
void RIFFChunk::fixSize()
{
    QPtrListIterator<RIFFChunk> it(subChunks());

    // pass one: fix sizes of sub chunks recursively
    for (; it.current(); ++it) {
	it.current()->fixSize();
    }

    // pass two: sum up sub-chunks if type is main or root.
    if ((m_type == Main) || (m_type == Root)) {
	u_int32_t old_length = m_phys_length;
	m_phys_length = 0;
	if (m_type == Main) m_phys_length += 4;

	for (it.toFirst(); it.current(); ++it) {
	    u_int32_t len = it.current()->physEnd() -
	                    it.current()->physStart() + 1;
	    m_phys_length += len;
	}
	if (m_phys_length != old_length) {
	    qDebug("%s: setting size from %u to %u",
	        path().data(), old_length, m_phys_length);
	}
	// chunk length is always equal to physical length for
	// main and root !
	m_chunk_length = m_phys_length;
    } else {
	// just round up if no main or root chunk
	if (m_phys_length & 0x1) {
	    m_phys_length++;
	    qDebug("%s: rounding up size to %u", path().data(), m_phys_length);
	}

	// adjust chunk size to physical size if not long enough
	if ((m_chunk_length+1 != m_phys_length) &&
	    (m_chunk_length != m_phys_length))
	{
	    qDebug("%s: resizing chunk from %u to %u",
	        path().data(), m_chunk_length, m_phys_length);
	    m_chunk_length = m_phys_length;
	}

    }

}

//***************************************************************************
void RIFFChunk::dumpStructure()
{
    // translate the type into a user-readable string
    QString t = "?";
    switch (m_type) {
        case Root:    t = "ROOT";    break;
        case Main:    t = "MAIN";    break;
        case Sub:     t = "SUB";     break;
        case Garbage: t = "GARBAGE"; break;
        case Empty:   t = "EMPTY";   break;
    }

    // dump this chunk
    QCString p = path();
    if (m_type == Main) p += " (" + m_format + ")";

    qDebug("[0x%08X-0x%08X] (%10u/%10u) %7s, '%s'",
          m_phys_offset, physEnd(), physLength(), length(),
          t.local8Bit().data(), p.data()
    );

    // recursively dump all sub-chunks
    QPtrListIterator<RIFFChunk> it(m_sub_chunks);
    for (; it.current(); ++it) {
        RIFFChunk *chunk = it.current();
        chunk->dumpStructure();
    }

}

//***************************************************************************
//***************************************************************************
