/*************************************************************************
            RIFFChunk.h  -  information about a RIFF chunk
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

#ifndef _RIFF_CHUNK_H_
#define _RIFF_CHUNK_H_

#include <sys/types.h>
#include <qlist.h>
#include <qstring.h>

class RIFFChunk
{
public:
    typedef enum {MainChunk, SubChunk} ChunkType;

    inline ChunkType type() { return m_type; };
    inline void setType(ChunkType type) { m_type = type;};

    inline const QCString &name() { return m_name; };
    inline void setName(const QCString &name) { m_name = name; };

    inline const QCString &format() { return m_format; };
    inline void setFormat(const QCString &format) { m_format = format; };

    inline const QCString &parent() { return m_parent; };
    inline void setParent(const QCString &parent) { m_parent = parent;};

    inline const QCString path() { return m_parent + "/" + m_name; };

private:
    ChunkType m_type;
    QCString m_name;
    QCString m_format;
    QCString m_parent;

public:
    u_int32_t m_chunk_length;
    u_int32_t m_phys_offset;
    u_int32_t m_phys_length;

    QList<RIFFChunk> m_sub_chunks;
};

#endif /* _RIFF_CHUNK_H_ */
