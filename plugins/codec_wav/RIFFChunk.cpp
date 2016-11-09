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

#include "config.h"

#include <QList>
#include <QString>

#include "libkwave/String.h"

#include "RIFFChunk.h"

//***************************************************************************
Kwave::RIFFChunk::RIFFChunk(RIFFChunk *parent, const QByteArray &name,
                            const QByteArray &format, quint32 length,
                            quint32 phys_offset, quint32 phys_length)
    :m_type(Sub), m_name(name), m_format(format), m_parent(parent),
     m_chunk_length(length), m_phys_offset(phys_offset),
     m_phys_length(phys_length), m_sub_chunks()
{
}

//***************************************************************************
Kwave::RIFFChunk::~RIFFChunk()
{
    while (!m_sub_chunks.isEmpty()) {
        Kwave::RIFFChunk *chunk = m_sub_chunks.takeLast();
        if (chunk) delete chunk;
    }
}

//***************************************************************************
// #define CHECK(x) Q_ASSERT(!(x)); if (x) return false;
#define CHECK(x) if (x) return false;
bool Kwave::RIFFChunk::isSane() const
{
    CHECK(m_type == Empty);
    CHECK(m_type == Garbage);
    CHECK((m_type == Main) && m_sub_chunks.isEmpty());
    CHECK((m_type == Root) && m_sub_chunks.isEmpty());

#ifdef DEBUG
    if (m_phys_length & 0x1) {
	// size is not an even number: no criterium for insanity
	// but worth a warning
	qWarning("%s: physical length is not an even number: %u",
	        path().data(), m_phys_length);
    }
#endif /* DEBUG */

    unsigned int datalen = dataLength();
    if (m_type == Main) datalen += 4;
    if (((datalen + 1) < m_phys_length) || (datalen > m_phys_length)) {
	qWarning("%s: dataLength=%u, phys_length=%u",
	         path().data(), datalen, m_phys_length);
	return false;
    }

    foreach (const Kwave::RIFFChunk *chunk, subChunks())
        if (chunk && !chunk->isSane()) return false;
    return true;
}

//***************************************************************************
quint32 Kwave::RIFFChunk::physEnd() const
{
    quint32 end = m_phys_offset + m_phys_length;
    if (m_phys_length) --end;
    if ((m_type != Root) && (m_type != Garbage)) end += 8;
    return end;
}

//***************************************************************************
const QByteArray Kwave::RIFFChunk::path() const
{
    QByteArray p = "";

    if (m_parent) p += m_parent->path() + '/';
    p += m_name;
    if (m_type == Main) p += ':' + m_format;

    if (m_parent) {
	QListIterator<Kwave::RIFFChunk *> it(m_parent->subChunks());
	unsigned int before = 0;
	unsigned int after  = 0;
	const Kwave::RIFFChunk *chunk = 0;
	while (it.hasNext()) {
            chunk = it.next();
            if (!chunk) continue;
            if (chunk == this) break;
	    if (chunk->name() != m_name) continue;
	    if (chunk->type() != m_type) continue;
	    if ((m_type == Main) && (chunk->format() != m_format)) continue;
	    before++;
	}
	if ((chunk == this) && (it.hasNext())) chunk = it.next();
	while ((chunk != this) && (it.hasNext())) {
            chunk = it.next();
            if (!chunk) continue;
	    if (chunk->name() != m_name) continue;
	    if (chunk->type() != m_type) continue;
	    if ((m_type == Main) && (chunk->format() != m_format)) continue;
	    after++;
	}
	if (before + after != 0) {
	    QByteArray index;
	    index.setNum(before);
	    p += '(' + index + ')';
	}
    }

    return p;
}

//***************************************************************************
quint32 Kwave::RIFFChunk::dataStart() const
{
    return m_phys_offset + ((m_type == Main) ? 12 : 8);
}

//***************************************************************************
quint32 Kwave::RIFFChunk::dataLength() const
{
    return m_chunk_length - ((m_type == Main) ? 4 : 0);
}

//***************************************************************************
void Kwave::RIFFChunk::setLength(quint32 length)
{
    m_chunk_length = length;
    m_phys_length  = length;
}

//***************************************************************************
bool Kwave::RIFFChunk::isChildOf(Kwave::RIFFChunk *chunk)
{
    if (!chunk) return (m_type == Root); // only root has null as parent
    if (chunk == m_parent) return true;
    if (m_parent) return m_parent->isChildOf(chunk);
    return false;
}

//***************************************************************************
void Kwave::RIFFChunk::fixSize()
{
    // pass one: fix sizes of sub chunks recursively
    foreach (Kwave::RIFFChunk *chunk, subChunks())
        if (chunk) chunk->fixSize();

    // pass two: sum up sub-chunks if type is main or root.
    if ((m_type == Main) || (m_type == Root)) {
	quint32 old_length = m_phys_length;
	m_phys_length = 0;
	if (m_type == Main) m_phys_length += 4;

        foreach (Kwave::RIFFChunk *chunk, subChunks()) {
            if (!chunk) continue;
	    quint32 len = chunk->physEnd() - chunk->physStart() + 1;
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
void Kwave::RIFFChunk::dumpStructure()
{
    // translate the type into a user-readable string
    const char *t = "?";
    switch (m_type) {
        case Root:    t = "ROOT";    break;
        case Main:    t = "MAIN";    break;
        case Sub:     t = "SUB";     break;
        case Garbage: t = "GARBAGE"; break;
        case Empty:   t = "EMPTY";   break;
    }

    // dump this chunk
    qDebug("[0x%08X-0x%08X] (%10u/%10u) %7s, '%s'",
          m_phys_offset, physEnd(), physLength(), length(),
          t, path().data()
    );

    // recursively dump all sub-chunks
    foreach (Kwave::RIFFChunk *chunk, m_sub_chunks)
        if (chunk) chunk->dumpStructure();

}

//***************************************************************************
//***************************************************************************
