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

#include <qlist.h>
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

    debug("[0x%08X-0x%08X] (%10u/%10u) %7s, '%s'",
          physStart(), physEnd(), physLength(), length(),
          t.data(), p.data()
    );

    // recursively dump all sub-chunks
    QListIterator<RIFFChunk> it(m_sub_chunks);
    for (; it.current(); ++it) {
        RIFFChunk *chunk = it.current();
        chunk->dumpStructure();
    }

}

//***************************************************************************
//***************************************************************************
