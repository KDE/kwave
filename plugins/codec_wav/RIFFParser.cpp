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

#include <endian.h>
#include <byteswap.h>
#include <math.h>
#include <stdlib.h>

#include <qiodevice.h>
#include <qlist.h>
#include <qstring.h>
#include <qstringlist.h>

#include "RIFFChunk.h"
#include "RIFFParser.h"

#if defined(IS_BIG_ENDIAN)
#define SYSTEM_ENDIANNES BigEndian
#else
#define SYSTEM_ENDIANNES LittleEndian
#endif

//***************************************************************************
RIFFParser::RIFFParser(QIODevice &device, const QStringList &main_chunks,
                       const QStringList &known_subchunks)
    :m_dev(device), m_root(0, "", "", device.size(), 0, device.size()),
     m_main_chunk_names(main_chunks), m_sub_chunk_names(known_subchunks),
     m_endianness(Unknown)
{
    m_root.setType(RIFFChunk::Root);
}

//***************************************************************************
RIFFParser::~RIFFParser()
{
}

//***************************************************************************
void RIFFParser::detectEndianness()
{
    QValueList<u_int32_t> riff_offsets = scanForName("RIFF",
        m_root.physStart(), m_root.physLength());
    QValueList<u_int32_t> rifx_offsets = scanForName("RIFX",
        m_root.physStart(), m_root.physLength());

    // if RIFF found and RIFX not found -> little endian
    if (riff_offsets.count() && !rifx_offsets.count()) {
        debug("detected little endian format");
        m_endianness = LittleEndian;
        return;
    }

    // if RIFX found and RIFF not found -> big endian
    if (rifx_offsets.count() && !riff_offsets.count()) {
        debug("detected big endian format");
        m_endianness = BigEndian;
        return;
    }

    // not detectable -> detect by searching all known chunks and
    // detect best match
    debug("doing statistic search to determine endianness...");
    unsigned int le_matches = 0;
    unsigned int be_matches = 0;
    QStringList names;
    names += m_main_chunk_names;
    names += m_sub_chunk_names;
    QStringList::Iterator it;

    // average length should be approx. half of file size
    double half = (m_dev.size() >> 1);

    // loop over all chunk names
    for (it = names.begin(); it != names.end(); ++it ) {
        // scan all offsets where the name matches
        QCString name = (*it).latin1();
        QValueList<u_int32_t> offsets = scanForName(name,
            m_root.physStart(), m_root.physLength());

        // loop over all found offsets
        QValueList<u_int32_t>::Iterator it_ofs = offsets.begin();
        for (; it_ofs != offsets.end(); ++it_ofs) {
            m_dev.at(*it_ofs + 4);

            // read length, assuming little endian
            u_int32_t len = 0;
            m_dev.readBlock((char*)(&len), 4);
#if defined(IS_BIG_ENDIAN)
            double dist_le = fabs(half - bswap_32(len));
            double dist_be = fabs(half - len);
#else
            double dist_le = fabs(half - len);
            double dist_be = fabs(half - bswap_32(len));
#endif
            if (dist_be > 1.3*dist_le) ++le_matches;
            if (dist_le > 1.3*dist_be) ++be_matches;
        }
    }
    debug("big endian matches:    %u", be_matches);
    debug("little endian matches: %u", le_matches);

    if (le_matches > be_matches) {
        debug("assuming little endian");
        m_endianness = LittleEndian;
    } else if (be_matches > le_matches) {
        debug("assuming big endian");
        m_endianness = BigEndian;
    } else {
        // give up :-(
        debug("unable to determine endianness");
        m_endianness = Unknown;
    }
}

//***************************************************************************
bool RIFFParser::parse()
{
    // first of all we have to find out the endianness of our source
    detectEndianness();

    // not detectable -> no chance of finding anything useful -> give up!
    if (m_endianness == Unknown) {
        warning("unable to detect endianness -> giving up!");
        return false;
    }

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
    QCString name(16);
    name.sprintf("[0x%08X]", offset);
    RIFFChunk *chunk = new RIFFChunk(parent, name, "", length,
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

    // remember the number of sub-chunks that are already present
    // in the chunk, these have to be skipped in the recursion later
    unsigned int old_chunk_count = parent->subChunks().count();

    do {
        ASSERT(error==false);
//        debug("RIFFParser::parse(offset=0x%08X, length=%u)", offset, length);

        // make sure that we are still in the source (file)
        if (offset >= m_dev.size()) {
            error = true;
            break;
        }

        // abort search if we passed the same position twice
        // (this might happen if an intensive search is performed
        // and one position can be reached in two or more ways)
        // only exception: the root chunk, this always overlaps
        // with the first chunk at start of search!
        RIFFChunk *prev = chunkAt(offset);
        if (prev && (m_root.subChunks().count())) break;

        // chunks with less than 4 bytes are not possible
        if (length < 4) {
            warning("chunk with less than 4 bytes at offset 0x%08X, "\
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
//                debug("addGarbageChunk(offset=0x%08X, length=%u)",offset,length);
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
            if (m_endianness != SYSTEM_ENDIANNES) len = bswap_32(len);
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
        if (!chunk) break;
        parent->subChunks().append(chunk);

        // if not at the end of the file, parse all further chunks
        length -= chunk->physLength() + 8;
        offset  = chunk->physEnd() + 1;
//        debug("parse loop end: offset=0x%08X, length=%u",offset,length);
    } while (length);

    // parse for sub-chunks in the chunks we newly found
    QListIterator<RIFFChunk> it(parent->subChunks());
    while (old_chunk_count--) ++it; // skip already known ones
    for (; it.current(); ++it) {
	RIFFChunk *chunk = it.current();
	
	if ( (m_main_chunk_names.grep(chunk->name()).count()) &&
	     (chunk->dataLength() >= 4) )
	{
            chunk->setType(RIFFChunk::Main);

            QCString path = (parent ? parent->path() : QCString("")) +
                            "/" + chunk->name();
            debug("scanning for chunks in '%s' (format='%s'), offset=0x%08X, length=%u",
                  path.data(), chunk->format().data(),
                  chunk->dataStart(), chunk->dataLength());
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
RIFFChunk *RIFFParser::chunkAt(u_int32_t offset)
{
    RIFFChunkList list;
    listAllChunks(m_root, list);
    QListIterator<RIFFChunk> it(list);
    for (; it.current(); ++it) {
        if ((*it.current()).physStart() == offset) return it.current();
    }
    return 0;
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
                u_int32_t pos = (*it);
                u_int32_t len = end-pos+1;
                debug("found at [0x%08X...0x%08X] len=%u", pos, end, len);
                parse(chunk, pos, len);
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
