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
bool RIFFParser::isValidName(const char *name)
{
    int i;
    for (i=0; i < 4; ++i) {
        char c = name[i];
        if ((c >= 'a') && (c <= 'z')) continue;
        if ((c >= 'A') && (c <= 'Z')) continue;
        if ((c >= '0') && (c <= '9')) continue;
        if ((c == ' ') || (c == '(') || (c == ')'))continue;
        return false;
    }
    return true;
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
            // evaluate distance to average length
            if (dist_be > dist_le) ++le_matches;
            if (dist_le > dist_be) ++be_matches;
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
RIFFChunk *RIFFParser::addChunk(RIFFChunk *parent, const QCString &name,
                                const QCString &format, u_int32_t length,
                                u_int32_t phys_offset, u_int32_t phys_length,
                                RIFFChunk::ChunkType type)
{
    // do not add anything to garbage, use the garbage's parent instead
    while (parent && (parent->type() == RIFFChunk::Garbage)) {
        debug("parenting '%s' to '%s'", name.data(),
            parent->parent() ? parent->parent()->name().data() : "");
        parent = parent->parent();
    }

    // if no parent found, use root
    if (!parent) {
        debug("parenting chunk at 0x%08X to root", phys_offset);
        parent = &m_root;
    }
    ASSERT(parent);

    // create a new chunk object
    RIFFChunk *chunk = new RIFFChunk(parent, name, format, length,
                                     phys_offset, phys_length);
    ASSERT(chunk);
    if (!chunk) return 0;

    debug("new chunk, name='%s', len=%u, ofs=0x%08X, phys_len=%u, root=%p '%s'",
          name.data(),length,phys_offset,phys_length, parent,
          parent ? parent->name().data() : ""
    );

    chunk->setType(type);
    parent->subChunks().append(chunk);

    return chunk;
}

//***************************************************************************
bool RIFFParser::addGarbageChunk(RIFFChunk *parent, u_int32_t offset,
                                 u_int32_t length)
{
    debug("adding garbage chunk at 0x%08X, length=%u",offset,length);

    // create the new chunk first
    QCString name(16);
    name.sprintf("[0x%08X]", offset);
    RIFFChunk *chunk = addChunk(parent, name, "", length, offset,
                                length, RIFFChunk::Garbage);
    return (chunk);
}

//***************************************************************************
bool RIFFParser::addEmptyChunk(RIFFChunk *parent, const QCString &name,
                               u_int32_t offset)
{
    // create the new chunk first
    RIFFChunk *chunk = addChunk(parent, name, "----", 0, offset,
                                0, RIFFChunk::Empty);
    return (chunk);
}

//***************************************************************************
bool RIFFParser::parse(RIFFChunk *parent, u_int32_t offset, u_int32_t length)
{
    bool error = false;
    RIFFChunkList found_chunks;
    found_chunks.setAutoDelete(false);

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
        if (!isValidName(name)) {
            warning("invalid chunk name at offset 0x%08X", offset);
            // unreadable name -> make it a "garbage" chunk
            debug("addGarbageChunk(offset=0x%08X, length=%u)",offset,length);
            addGarbageChunk(parent, offset, length);
            error = true;
            break;
        }

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
        RIFFChunk *chunk = addChunk(parent, name, format, len, offset,
                                    phys_len, RIFFChunk::Sub);
        if (!chunk) break;
        found_chunks.append(chunk);

        // if not at the end of the file, parse all further chunks
        length -= chunk->physLength() + 8;
        offset  = chunk->physEnd() + 1;
//        debug("parse loop end: offset=0x%08X, length=%u",offset,length);
    } while (length);

    // parse for sub-chunks in the chunks we newly found
    QListIterator<RIFFChunk> it(found_chunks);
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
    RIFFChunkList chunks;
    listAllChunks(m_root, chunks);

    QListIterator<RIFFChunk> it(chunks);
    for (; it.current(); ++it) {
	RIFFChunk *chunk = it.current();
        if (chunk->path() == path) return chunk;
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
    bool found_something = false;

    // first search in all garbage areas
    RIFFChunkList all_chunks;
    listAllChunks(m_root, all_chunks);

    QListIterator<RIFFChunk> ic(all_chunks);
    for (; ic.current(); ++ic) {
        RIFFChunk *chunk = ic.current();
        if (chunk->type() == RIFFChunk::Garbage) {
            // search for the name
            debug("searching in garbage at 0x%08X", chunk->physStart());
            QValueList<u_int32_t> offsets = scanForName(name,
                chunk->physStart(), chunk->physLength());
            if (offsets.count()) found_something = true;

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
    if (!found_something) {
        debug("brute-force search from 0x%08X to 0x%08X",
              0, m_root.physEnd());
        QValueList<u_int32_t> offsets = scanForName(name,
            0, m_root.physLength());

        // process the results -> convert them into chunks
        QValueList<u_int32_t>::Iterator it = offsets.begin();
        u_int32_t end = m_root.physEnd();
        for (;it != offsets.end(); ++it) {
            u_int32_t pos = (*it);
            u_int32_t len = end-pos+1;
            debug("found at [0x%08X...0x%08X] len=%u", pos, end, len);
            parse(&m_root, pos, len);
            debug("-------------------------------");
        }
    }

    return 0;
}

//***************************************************************************
void RIFFParser::repair()
{
    // remove all main chunks that contain only garbage and convert them
    // into garbage chunks
    bool start_over;
    do {
        start_over = false;
        RIFFChunkList chunks;
        listAllChunks(m_root, chunks);
        QListIterator<RIFFChunk> it(chunks);

        for ( ;it.current() && !start_over; ++it) {
            RIFFChunk *chunk = it.current();

            // skip garbage chunks themselfes
            if (chunk->type() == RIFFChunk::Garbage) continue;

            RIFFChunkList &subchunks = chunk->subChunks();
            bool contains_only_garbage = true;
            QListIterator<RIFFChunk> sub(subchunks);
            for (; sub.current(); ++sub) {
                RIFFChunk::ChunkType type = (*sub.current()).type();
                if (type != RIFFChunk::Garbage) {
                    contains_only_garbage = false;
                    break;
                }
            }

            if (subchunks.count() && contains_only_garbage) {
                debug("chunk at 0x%08X contains only garbage!",
                    chunk->physStart());
                // -> convert into a garbage chunk !
                chunk->setType(RIFFChunk::Garbage);
                subchunks.setAutoDelete(true);
                subchunks.clear();
                chunks.clear();

                // start over the scan...
                start_over = true;
                break;
            }
        }
    } while (start_over);

    // first pass: resolve overlaps
    resolveOverlaps();

    // second pass: join garbage to truncated chunks
    joinGarbage();

    // third pass: crawl in garbage for known chunks
    // crawlInGarbage();

    // fourth pass: throw away all remaining garbage
    discardGarbage();
}

//***************************************************************************
void RIFFParser::resolveOverlaps()
{
    debug("resolving overlaps...");
}

//***************************************************************************
void RIFFParser::joinGarbage()
{
    debug("joining garbage...");
}

//***************************************************************************
void RIFFParser::discardGarbage()
{
    debug("discarding garbage...");
}

//***************************************************************************
//***************************************************************************
