/*************************************************************************
         RIFFParser.cpp  -  parser for the RIFF format
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
#include <qiodevice.h>
#include <qlist.h>
#include <qstring.h>

#include "RIFFChunk.h"
#include "RIFFParser.h"

//***************************************************************************
RIFFParser::RIFFParser(QIODevice &device)
    :m_dev(device), m_chunks()
{
}

//***************************************************************************
RIFFParser::~RIFFParser()
{
}

//***************************************************************************
bool RIFFParser::parse()
{
    // find all primary chunks
    return parse("", 0, m_dev.size(), m_chunks);
}

//***************************************************************************
bool RIFFParser::parse(const QCString &parent, u_int32_t offset,
                       u_int32_t length, QList<RIFFChunk> &chunks)
{
    int i;
    bool error = false;

    ASSERT(m_dev.isDirectAccess());
    if (!m_dev.isDirectAccess()) return false;

    do {
        // chunks with less than 8 bytes are not possible
        if (length < 8) {
            warning("chunk with less than 8 bytes at offset %u, "\
                    "length=%u bytes!", offset, length);
            error = true;
            break;
        }

        m_dev.at(offset);

        // get the chunk name
        char name[5];
        m_dev.readBlock(&name[0], 4);
        name[4]=0;

        // check if the name really contains only ASCII characters
        for (i=0; i < 4; ++i) {
            if (!QChar(name[i]).isLetterOrNumber() && (name[i] != ' ')) {
                warning("invalid chunk name: %c%c%c%c",
                        name[0], name[1], name[2], name[3]);
                error = true;
                break;
            }
        }

        // get the length of the chunk
        u_int32_t len = 0;
        m_dev.readBlock((char*)(&len), 4);
#if defined(IS_BIG_ENDIAN)
        len = bswap_32(len);
#endif

        // read the format
        char format[5];
        m_dev.readBlock(&format[0], 4);
        format[4] = 0;

        // now create a new chunk object
        RIFFChunk *chunk = new RIFFChunk();
        ASSERT(chunk);
        if (!chunk) return false;

        chunk->setName(name);
        chunk->setFormat(format);
        chunk->m_chunk_length = len;
        chunk->m_phys_offset = offset;

        // calculate the physical length of the chunk
        u_int32_t phys_len = (length-8 < len) ? (length-8) : len;
        if (phys_len & 1) phys_len++;
        chunk->m_phys_length = phys_len;
        chunk->setParent(parent);
        chunk->setType(RIFFChunk::SubChunk);
        chunks.append(chunk);

        debug("[0x%08X-0x%08X] (%8d) '%s', phys. length=%u",
              chunk->m_phys_offset,
              chunk->m_phys_offset + chunk->m_phys_length - 1,
              chunk->m_chunk_length,
              chunk->path().data(),
              chunk->m_phys_length
        );

        // if not at the end of the file, parse all further chunks
        unsigned int rest = length - chunk->m_phys_length - 8;
        unsigned int next = offset + chunk->m_phys_length + 8;

        offset = next;
        length = rest;
    } while (length);

    QListIterator<RIFFChunk> it(chunks);
    for (; it.current(); ++it) {
	RIFFChunk *chunk = it.current();
	
	if ((chunk->name() == "RIFF") ||
	    (chunk->name() == "WAVE") ||
	    (chunk->name() == "LIST"))
	{
            chunk->setType(RIFFChunk::MainChunk);

            QCString path = parent + "/" + chunk->name();
            debug("scanning for chunks in '%s' (type='%s'), offset=%u",
                  path.data(), chunk->format().data(),
                  chunk->m_phys_offset+8);
            if (!parse(path, chunk->m_phys_offset+12,
                  chunk->m_phys_length-4, chunk->m_sub_chunks))
            {
                error = true;
            }

        }

    }

    return !error;
}

//***************************************************************************
bool RIFFParser::findMissingChunk(const QCString &name)
{
    return false;
}

//***************************************************************************
//***************************************************************************
