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
    :m_dev(device), m_root(0, "", "", device.size(), 0, device.size())
{
    m_root.setType(RIFFChunk::Root);
}

//***************************************************************************
RIFFParser::~RIFFParser()
{
}

//***************************************************************************
bool RIFFParser::parse()
{
    // find all primary chunks
    return parse(&m_root, 0, m_dev.size());
}

//***************************************************************************
bool RIFFParser::addGarbageChunk(RIFFChunk *parent, u_int32_t offset,
                                 u_int32_t length)
{
    ASSERT(parent);
    if (!parent) return false;

    // create the new chunk first
    RIFFChunk *chunk = new RIFFChunk(parent, "####", "", length,
        offset, length);
    ASSERT(chunk);
    if (!chunk) return false;

    chunk->setType(RIFFChunk::Garbage);
    parent->subChunks().append(chunk);
    return true;
}

//***************************************************************************
bool RIFFParser::addEmptyChunk(RIFFChunk *parent, const QCString &name,
                               u_int32_t offset)
{
    ASSERT(parent);
    if (!parent) return false;

    // create the new chunk first
    RIFFChunk *chunk = new RIFFChunk(parent, name, "----", 0, offset, 0);
    ASSERT(chunk);
    if (!chunk) return false;

    chunk->setType(RIFFChunk::Empty);
    parent->subChunks().append(chunk);
    return true;
}

//***************************************************************************
bool RIFFParser::parse(RIFFChunk *parent, u_int32_t offset, u_int32_t length)
{
    int i;
    bool error = false;

    ASSERT(parent);
    ASSERT(m_dev.isDirectAccess());
    if (!m_dev.isDirectAccess()) return false;
    if (!parent) return false;

    do {
        ASSERT(error==false);
//        debug("RIFFParser::parse(offset=0x%08X, length=%u)", offset, length);

        // make sure that we are still in the source (file)
        if (offset >= m_dev.size()) {
            error = true;
            break;
        }

        // chunks with less than 4 bytes are not possible
        if (length < 4) {
            warning("chunk with less than 4 bytes at offset %u, "\
                    "length=%u bytes!", offset, length);
            // too short stuff is "garbage"
            addGarbageChunk(parent, offset, length);
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
                warning("invalid chunk name at offset 0x%08X", offset);
                // unreadable name -> make it a "garbage" chunk
                addGarbageChunk(parent, offset, length);
                error = true;
                break;
            }
        }
        if (error) break;

        // get the length stored in the chunk itself
        u_int32_t len = 0;
        if (length >= 8) {
            // length information present
            m_dev.readBlock((char*)(&len), 4);
#if defined(IS_BIG_ENDIAN)
            len = bswap_32(len);
#endif
        } else {
            // valid name but no length information -> badly truncated
            // -> make it a zero-length chunk
            debug("empty chunk at 0x%08X", offset);
            addEmptyChunk(parent, name, offset);
            error = true;
            break;
        }

        // read the format if present
        char format[5];
        memset(format, 0x00, sizeof(format));
        if (length >= 12) m_dev.readBlock(&format[0], 4);

        // calculate the physical length of the chunk
        u_int32_t phys_len = (length-8 < len) ? (length-8) : len;
        if (phys_len & 1) phys_len++;

        // now create a new chunk, per default type is "sub-chunk"
//        debug("new chunk, name='%s', len=%u, ofs=0x%08X, phys_len=%u",
//            name,len,offset,phys_len);
        RIFFChunk *chunk = new RIFFChunk(parent, name, format,
            len, offset, phys_len);
        ASSERT(chunk);
        if (!chunk) return false;
        parent->subChunks().append(chunk);

        // if not at the end of the file, parse all further chunks
        length -= chunk->physLength() + 8;
        offset  = chunk->physEnd() + 1;
    } while (length);

    QListIterator<RIFFChunk> it(parent->subChunks());
    for (; it.current(); ++it) {
	RIFFChunk *chunk = it.current();
	
	if ((chunk->name() == "RIFF") ||
	    (chunk->name() == "WAVE") ||
	    (chunk->name() == "LIST"))
	{
            chunk->setType(RIFFChunk::Main);

            QCString path = (parent ? parent->path() : QCString("")) +
                            "/" + chunk->name();
            debug("scanning for chunks in '%s' (type='%s'), offset=%u",
                  path.data(), chunk->format().data(),
                  chunk->dataStart());
            if (!parse(chunk, chunk->dataStart(), chunk->dataLength())) {
                error = true;
            }

        }
    }

    return (!error);
}

//***************************************************************************
void RIFFParser::dumpStructure()
{
    m_root.dumpStructure();
}

//***************************************************************************
RIFFChunk *RIFFParser::findChunk(const QCString &path)
{
    QCString name = path;
    QCString rest = "";
    RIFFChunkList &chunks = m_root.subChunks();

    while (name.length()) {
        if (name.find('/') == 0) name = name.mid(1);
        int slash;
        if ((slash=name.find('/')) > 0) {
            rest = name.mid(slash);
            name = name.left(slash);
        }

        QListIterator<RIFFChunk> it(chunks);
        for (; it.current(); ++it) {
            RIFFChunk *chunk = it.current();
            if (chunk->name() == name) {
                if (!rest.length()) return chunk;
                chunks = chunk->subChunks();
            }
        }

        name = rest;
        rest = "";
    }
    return 0;
}

//***************************************************************************
QValueList<u_int32_t> RIFFParser::scanForName(const QCString &name,
    u_int32_t offset, u_int32_t length)
{
    QValueList<u_int32_t> matches;

    ASSERT(length >= 4);
    u_int32_t end = offset + ((length > 4) ? (length - 4) : 0);
    char buffer[5];
    memset(buffer, 0x00, sizeof(buffer));

    m_dev.at(offset);
    m_dev.readBlock(&buffer[0], 4);

    debug("scannig for '%s' at [0x%08X...0x%08X] ...", name.data(),
          offset, end);
    u_int32_t pos;
    for (pos = offset; pos <= end; ++pos) {
        if (name == buffer) {
            // found the name
            matches.append(pos);
        }
        // try the next offset
        buffer[0] = buffer[1];
        buffer[1] = buffer[2];
        buffer[2] = buffer[3];
        buffer[3] = m_dev.getch();
    }

    return matches;
}

//***************************************************************************
void RIFFParser::listAllChunks(RIFFChunk &parent, RIFFChunkList &list)
{
    list.setAutoDelete(false);
    list.append(&parent);
    QListIterator<RIFFChunk> it(parent.subChunks());
    for (; it.current(); ++it) {
        listAllChunks(*it.current(), list);
    }
}

//***************************************************************************
RIFFChunk *RIFFParser::findMissingChunk(const QCString &name)
{
    // first search in all garbage areas
    RIFFChunkList all_chunks;
    listAllChunks(m_root, all_chunks);

    QListIterator<RIFFChunk> ic(all_chunks);
    for (; ic.current(); ++ic) {
        RIFFChunk *chunk = ic.current();
        if (chunk->type() == RIFFChunk::Garbage) {
            // search for the name
            debug("searching in garbage chunk at 0x%08X", chunk->physStart());
            QValueList<u_int32_t> offsets = scanForName(name,
                chunk->physStart(), chunk->physLength());

            // process the results -> convert them into chunks
            QValueList<u_int32_t>::Iterator it = offsets.begin();
            u_int32_t end = chunk->physEnd();
            for (;it != offsets.end(); ++it) {
                u_int32_t pos  = (*it);
                debug("found at [0x%08X...0x%08X]", pos, end);
                parse(chunk, pos, end-pos+1);
                debug("-------------------------------");
            }
        }
    }

    // not found in garbage? search over the rest of the file"
//    debug("brute-force search without success...");

    return 0;
}

//***************************************************************************
//***************************************************************************
