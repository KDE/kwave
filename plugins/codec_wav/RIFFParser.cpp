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

#include <math.h>
#include <stdlib.h>

#include <limits>

#include <QIODevice>
#include <QLatin1String>
#include <QList>
#include <QMutableListIterator>
#include <QString>
#include <QStringList>
#include <QtEndian>
#include <QtGlobal>

#include <KLocalizedString>

#include "libkwave/String.h"

#include "RIFFChunk.h"
#include "RIFFParser.h"

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
#define SYSTEM_ENDIANNES Kwave::BigEndian
#else
#define SYSTEM_ENDIANNES Kwave::LittleEndian
#endif

/**
 * saturated conversion of a quint64 into a quint32
 * @param x numeric value, 64 bit
 * @return the value of x clipped to 0xFFFFFFFF
 */
static inline quint32 toUint32(quint64 x) {
    const quint64 max = std::numeric_limits<quint32>::max();
    return static_cast<quint32>(qMin(x, max));
}

//***************************************************************************
Kwave::RIFFParser::RIFFParser(QIODevice &device,
                              const QStringList &main_chunks,
                              const QStringList &known_subchunks)
    :m_dev(device),
     m_root(Q_NULLPTR, "", "", toUint32(device.size()), 0,
            toUint32(device.size())),
     m_main_chunk_names(main_chunks), m_sub_chunk_names(known_subchunks),
     m_endianness(Kwave::UnknownEndian), m_cancel(false)
{
    m_root.setType(Kwave::RIFFChunk::Root);
}

//***************************************************************************
Kwave::RIFFParser::~RIFFParser()
{
}

//***************************************************************************
bool Kwave::RIFFParser::isValidName(const char *name)
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
Kwave::RIFFChunk::ChunkType Kwave::RIFFParser::guessType(const QByteArray &name)
{
    if (!isValidName(name)) return Kwave::RIFFChunk::Garbage;
    return (m_main_chunk_names.contains(QLatin1String(name))) ?
        Kwave::RIFFChunk::Main : Kwave::RIFFChunk::Sub;
}

//***************************************************************************
bool Kwave::RIFFParser::isKnownName(const QByteArray &name)
{
    if (m_main_chunk_names.contains(QLatin1String(name))) return true;
    if (m_sub_chunk_names.contains(QLatin1String(name))) return true;
    return false;
}

//***************************************************************************
void Kwave::RIFFParser::detectEndianness()
{
    // first try the easy way, works if file is sane
    QString sane_name = QLatin1String(read4ByteString(0));
    if (sane_name == _("RIFF")) {
        m_endianness = LittleEndian;
        return;
    }
    if (sane_name == _("RIFX")) {
        m_endianness = Kwave::BigEndian;
        return;
    }

    // ok, our file is damaged at least a bit, try to discover
    emit action(i18n("Detecting endianness (standard search)..."));
    emit progress(0);

    QList<quint32> riff_offsets = scanForName("RIFF",
        m_root.physStart(), m_root.physLength(), 0, 2);
    if (m_cancel) return;

    QList<quint32> rifx_offsets = scanForName("RIFX",
        m_root.physStart(), m_root.physLength(), 1, 2);
    if (m_cancel) return;

    // if RIFF found and RIFX not found -> little endian
    if (riff_offsets.count() && !rifx_offsets.count()) {
        qDebug("detected little endian format");
        m_endianness = LittleEndian;
        emit progress(100);
        return;
    }

    // if RIFX found and RIFF not found -> big endian
    if (rifx_offsets.count() && !riff_offsets.count()) {
        qDebug("detected big endian format");
        m_endianness = Kwave::BigEndian;
        emit progress(100);
        return;
    }

    // not detectable -> detect by searching all known chunks and
    // detect best match
    emit action(i18n("Detecting endianness (statistic search)..."));
    qDebug("doing statistic search to determine endianness...");
    unsigned int le_matches = 0;
    unsigned int be_matches = 0;
    QStringList names;
    names += m_main_chunk_names;
    names += m_sub_chunk_names;

    // average length should be approx. half of file size
    double half = static_cast<double>(m_dev.size() >> 1);

    // loop over all chunk names
    int count = names.count();
    int index = 0;
    foreach (QString chunk_name, names) {
        // scan all offsets where the name matches
        QByteArray name = chunk_name.toLatin1();
        QList<quint32> offsets = scanForName(name,
            m_root.physStart(), m_root.physLength(),
            index, count);
        if (m_cancel) return;

        // loop over all found offsets
        foreach (quint32 ofs, offsets) {
            m_dev.seek(ofs + 4);

            // read length, assuming little endian
            quint32 len = 0;
            m_dev.read(reinterpret_cast<char *>(&len), 4);
            double dist_le = fabs(half - qFromLittleEndian<quint32>(len));
            double dist_be = fabs(half - qFromBigEndian<quint32>(len));

            // evaluate distance to average length
            if (dist_be > dist_le) ++le_matches;
            if (dist_le > dist_be) ++be_matches;
        }

        emit progress(100 * (++index) / count);
    }
    qDebug("big endian matches:    %u", be_matches);
    qDebug("little endian matches: %u", le_matches);

    if (le_matches > be_matches) {
        qDebug("assuming little endian");
        m_endianness = Kwave::LittleEndian;
    } else if (be_matches > le_matches) {
        qDebug("assuming big endian");
        m_endianness = Kwave::BigEndian;
    } else {
        // give up :-(
        qDebug("unable to determine endianness");
        m_endianness = Kwave::UnknownEndian;
    }

    emit progress(100);
}

//***************************************************************************
QByteArray Kwave::RIFFParser::read4ByteString(qint64 offset)
{
    char s[5] = {0, 0, 0, 0, 0};

    m_dev.seek(offset);
    m_dev.read(&s[0], 4);

    return QByteArray(s);
}

//***************************************************************************
bool Kwave::RIFFParser::parse()
{
    // first of all we have to find out the endianness of our source
    detectEndianness();

    // not detectable -> no chance of finding anything useful -> give up!
    if (m_endianness == Kwave::UnknownEndian) {
        qWarning("unable to detect endianness -> giving up!");
        return false;
    }

    // find all primary chunks
    return parse(&m_root, 0, toUint32(m_dev.size()));
}

//***************************************************************************
Kwave::RIFFChunk *Kwave::RIFFParser::addChunk(
    Kwave::RIFFChunk *parent, const QByteArray &name,
    const QByteArray &format, quint32 length,
    quint32 phys_offset, quint32 phys_length,
    Kwave::RIFFChunk::ChunkType type)
{
    // do not add anything to garbage, use the garbage's parent instead
    while (parent && (parent->type() == Kwave::RIFFChunk::Garbage)) {
        parent = parent->parent();
    }

    // if no parent found, use root
    if (!parent) {
        parent = &m_root;
    }
    Q_ASSERT(parent);

    // create a new chunk object
    Kwave::RIFFChunk *chunk = new Kwave::RIFFChunk(
        parent, name, format, length, phys_offset, phys_length);
    Q_ASSERT(chunk);
    if (!chunk) return Q_NULLPTR;
    chunk->setType(type);

    // sort the chunk into the parent, order by physical start
    Kwave::RIFFChunk *before = Q_NULLPTR;
    Kwave::RIFFChunkList &chunks = parent->subChunks();
    foreach (Kwave::RIFFChunk *c, chunks) {
        if (!c) continue;
        quint32 pos = c->physStart();
        if (pos > phys_offset) {
            before = c;
            break;
        }
    }

    int index = (before) ? chunks.indexOf(before) : chunks.size();
    chunks.insert(index, chunk);

    return chunk;
}

//***************************************************************************
bool Kwave::RIFFParser::addGarbageChunk(Kwave::RIFFChunk *parent,
                                        quint32 offset,
                                        quint32 length)
{
    qDebug("adding garbage chunk at 0x%08X, length=%u",offset,length);

    // create the new chunk first
    QByteArray name(16, 0);
    qsnprintf(name.data(), name.size(), "[0x%08X]", offset);
    Kwave::RIFFChunk *chunk = addChunk(parent, name, "", length, offset,
                                length, Kwave::RIFFChunk::Garbage);
    return (chunk);
}

//***************************************************************************
bool Kwave::RIFFParser::addEmptyChunk(Kwave::RIFFChunk *parent,
                                      const QByteArray &name,
                                      quint32 offset)
{
    // create the new chunk first
    Kwave::RIFFChunk *chunk = addChunk(parent, name, "----", 0, offset,
            0, Kwave::RIFFChunk::Empty);
    return (chunk);
}

//***************************************************************************
bool Kwave::RIFFParser::parse(Kwave::RIFFChunk *parent,
                              quint32 offset, quint32 length)
{
    bool error = false;
    Kwave::RIFFChunkList found_chunks;

    Q_ASSERT(parent);
    if (m_dev.isSequential()) return false;
    if (!parent) return false;

    // be more robust if the file has not correctly padded
    if (length & 1) length++;

    do {
//      qDebug("RIFFParser::parse(offset=0x%08X, length=0x%08X)",
//          offset, length);

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
        const Kwave::RIFFChunk *prev = chunkAt(offset);
        if (prev && (m_root.subChunks().count())) break;

        // chunks with less than 4 bytes are not possible
        if (length < 4) {
            qWarning("chunk with less than 4 bytes at offset 0x%08X, "\
                    "length=%u bytes!", offset, length);
            // too short stuff is "garbage"
            addGarbageChunk(parent, offset, length);
            error = true;
            break;
        }

        m_dev.seek(offset);

        // get the chunk name
        QByteArray name = read4ByteString(m_dev.pos());

        // check if the name really contains only ASCII characters
        if (!isValidName(name)) {
            qWarning("invalid chunk name at offset 0x%08X", offset);
            // unreadable name -> make it a "garbage" chunk
            qDebug("addGarbageChunk(offset=0x%08X, length=0x%08X)",
                    offset, length);
            addGarbageChunk(parent, offset, length);
            error = true;
            break;
        }

        // get the length stored in the chunk itself
        quint32 len = 0;
        if (length >= 8) {
            // length information present
            m_dev.read(reinterpret_cast<char *>(&len), 4);
            if (m_endianness != SYSTEM_ENDIANNES) len = qbswap<quint32>(len);
        }
        if (len == 0) {
            // valid name but no length information -> badly truncated
            // -> make it a zero-length chunk
            qDebug("empty chunk '%s' at 0x%08X", name.data(), offset);
            addEmptyChunk(parent, name, offset);

            if (length > 8) {
                // there's some garbage behind
                offset += 8;
                length -= 8;
            }
            error = true;
            continue;
        }

        // read the format if present
        QByteArray format = read4ByteString(m_dev.pos());

        // calculate the physical length of the chunk
        quint32 phys_len = (length - 8 < len) ? (length - 8) : len;
        if (phys_len & 1) phys_len++;

        // now create a new chunk, per default type is "sub-chunk"
/*      qDebug("new chunk, name='%s', len=0x%08X, ofs=0x%08X, "\
            "phys_len=0x%08X (next=0x%08X)",
            name.data(),
            len,offset,phys_len, offset+phys_len+8); */
        Kwave::RIFFChunk *chunk = addChunk(parent, name, format, len,
            offset, phys_len, Kwave::RIFFChunk::Sub);
        if (!chunk) break;
        found_chunks.append(chunk);

        // if not at the end of the file, parse all further chunks
        length -= chunk->physLength() + 8;
        offset  = chunk->physEnd() + 1;
//      qDebug("   parse loop end: offset=0x%08X, length=0x%08X",offset,length);
    } while (length && !m_cancel);

    // parse for sub-chunks in the chunks we newly found
    foreach (Kwave::RIFFChunk *chunk, found_chunks) {
        if (!chunk) continue;
        if ( (guessType(chunk->name()) == Kwave::RIFFChunk::Main) &&
            (chunk->dataLength() >= 4) )
        {
            chunk->setType(Kwave::RIFFChunk::Main);

/*          QByteArray path = (parent ? parent->path() : QByteArray("")) +
                            '/' + chunk->name();
            qDebug("scanning for chunks in '%s' (format='%s'), "\
                "offset=0x%08X, length=0x%08X",
                path.data(), chunk->format().data(),
                chunk->dataStart(), chunk->dataLength());*/
            if (!parse(chunk, chunk->dataStart(), chunk->dataLength())) {
                error = true;
            }

        }
    }

    return (!error && !m_cancel);
}

//***************************************************************************
void Kwave::RIFFParser::dumpStructure()
{
    m_root.dumpStructure();
}

//***************************************************************************
bool Kwave::RIFFParser::isSane()
{
    return m_root.isSane();
}

//***************************************************************************
Kwave::RIFFChunk *Kwave::RIFFParser::findChunk(const QByteArray &path)
{
    Kwave::RIFFChunkList chunks;
    listAllChunks(m_root, chunks);

    foreach (Kwave::RIFFChunk *chunk, chunks) {
        if (!chunk) continue;
        if (path.contains("/")) {
            // search for full path
            if (chunk->path() == path) return chunk;
        } else {
            // search for name only
            if (chunk->name() == path) return chunk;
        }
    }

    return Q_NULLPTR;
}

//***************************************************************************
unsigned int Kwave::RIFFParser::chunkCount(const QByteArray &path)
{
    unsigned int count = 0;
    Kwave::RIFFChunkList chunks;
    listAllChunks(m_root, chunks);

    foreach (const Kwave::RIFFChunk *chunk, chunks) {
        if (!chunk) continue;
        if (path.contains("/")) {
            // search for full path
            if (chunk->path() == path) ++count;
        } else {
            // search for name only
            if (chunk->name() == path) ++count;
        }
    }

    return count;
}

//***************************************************************************
QList<quint32> Kwave::RIFFParser::scanForName(const QByteArray &name,
    quint32 offset, quint32 length,
    int progress_start, int progress_count)
{
    QList<quint32> matches;
    if (length < 4) return matches;
    quint32 end = offset + ((length > 4) ? (length - 4) : 0);
    char buffer[5];
    memset(buffer, 0x00, sizeof(buffer));

    m_dev.seek(offset);
    m_dev.read(&buffer[0], 4);

    qDebug("scannig for '%s' at [0x%08X...0x%08X] ...", name.data(),
        offset, end);
    quint32 pos;
    int next = 1;
    for (pos = offset; (pos <= end) && !m_cancel; ++pos) {
        if (name == buffer) {
            // found the name
            matches.append(pos);
        }
        // try the next offset
        buffer[0] = buffer[1];
        buffer[1] = buffer[2];
        buffer[2] = buffer[3];
        m_dev.getChar(&(buffer[3]));

        // update progress bar
        if (!--next && progress_count && (end > offset)) {
            int percent = (((100*progress_start + (100*(pos-offset)) /
                    (end-offset))) / progress_count);
            emit progress(percent);
            next = (end-offset)/100;
        }
    }

    return matches;
}

//***************************************************************************
void Kwave::RIFFParser::listAllChunks(Kwave::RIFFChunk &parent,
                                      Kwave::RIFFChunkList &list)
{
    list.append(&parent);
    foreach (Kwave::RIFFChunk *chunk, parent.subChunks())
        if (chunk) listAllChunks(*chunk, list);
}

//***************************************************************************
Kwave::RIFFChunk *Kwave::RIFFParser::chunkAt(quint32 offset)
{
    Kwave::RIFFChunkList list;
    listAllChunks(m_root, list);
    foreach (Kwave::RIFFChunk *chunk, list)
        if (chunk && chunk->physStart() == offset) return chunk;
        return Q_NULLPTR;
}

//***************************************************************************
Kwave::RIFFChunk *Kwave::RIFFParser::findMissingChunk(const QByteArray &name)
{
    emit action(i18n("Searching for missing chunk '%1'...",
                QLatin1String(name)));
    emit progress(0);

    bool found_something = false;

    // first search in all garbage areas
    Kwave::RIFFChunkList all_chunks;
    listAllChunks(m_root, all_chunks);

    int index = 0;
    int count = all_chunks.count();
    foreach (Kwave::RIFFChunk *chunk, all_chunks) {
        if (m_cancel) break;
        if (!chunk) continue;
        if (chunk->type() == Kwave::RIFFChunk::Garbage) {
            // search for the name
            qDebug("searching in garbage at 0x%08X", chunk->physStart());
            QList<quint32> offsets = scanForName(name,
                chunk->physStart(), chunk->physLength(),
                index, count);
            if (offsets.count()) found_something = true;

            // process the results -> convert them into chunks
            quint32 end = chunk->physEnd();
            foreach (quint32 pos, offsets) {
                if (m_cancel) break;
                quint32 len = end - pos + 1;
                qDebug("found at [0x%08X...0x%08X] len=%u", pos, end, len);
                parse(chunk, pos, len);
                qDebug("-------------------------------");
            }
        }
        ++index;
    }

    // not found in garbage? search over the rest of the file"
    if (!found_something && !m_cancel) {
        qDebug("brute-force search from 0x%08X to 0x%08X",
            0, m_root.physEnd());
        QList<quint32> offsets = scanForName(name, 0, m_root.physLength());

        // process the results -> convert them into chunks
        quint32 end = m_root.physEnd();
        foreach (quint32 pos, offsets) {
            if (m_cancel) break;
            quint32 len = end - pos + 1;
            qDebug("found at [0x%08X...0x%08X] len=%u", pos, end, len);
            parse(&m_root, pos, len);
            qDebug("-------------------------------");
        }
    }

    return Q_NULLPTR;
}

//***************************************************************************
void Kwave::RIFFParser::repair()
{
    bool one_more_pass = true;

    while (one_more_pass && !m_cancel) {
        // crawl in garbage for all known chunks and sub-chunks
        // crawlInGarbage();

        // clear all main chunks that contain only garbage and convert them
        // into garbage chunks. maybe they were only false hits in a previous
        // search in garbage
        collectGarbage();

        // join garbage to empty chunks
        if (joinGarbageToEmpty()) continue;

        // resolve overlaps of garbage with other chunks
        fixGarbageEnds();

        // throw away all remaining garbage
        qDebug("discarding garbage...");
        discardGarbage(m_root);

        // done, no more passes needed
        one_more_pass = false;
    }
}

//***************************************************************************
void Kwave::RIFFParser::collectGarbage()
{
    // clear all main chunks that contain only garbage and convert them
    // into garbage chunks
    bool start_over;
    do {
        start_over = false;
        Kwave::RIFFChunkList chunks;
        listAllChunks(m_root, chunks);

        foreach (Kwave::RIFFChunk *chunk, chunks) {
            if (!chunk) continue;
            if (start_over || m_cancel) break;

            // skip garbage chunks themselfes
            if (chunk->type() == Kwave::RIFFChunk::Garbage) continue;

            Kwave::RIFFChunkList &subchunks = chunk->subChunks();
            bool contains_only_garbage = true;
            foreach (const Kwave::RIFFChunk *sub, subchunks) {
                if (m_cancel) break;
                if (sub && (sub->type() != Kwave::RIFFChunk::Garbage)) {
                    contains_only_garbage = false;
                    break;
                }
            }

            if (subchunks.count() && contains_only_garbage) {
                quint32 start = chunk->physStart();
                quint32 end   = chunk->physEnd();

                qDebug("chunk at 0x%08X contains only garbage!", start);
                // -> convert into a garbage chunk !
                chunk->setType(Kwave::RIFFChunk::Garbage);
                chunk->setLength(end - start + 4 + 1);
                while (!subchunks.isEmpty()) {
                    Kwave::RIFFChunk *c = subchunks.takeLast();
                    if (c) delete c;
                }
                chunks.clear();

                // start over the scan...
                start_over = true;
                break;
            }
        }
    } while (start_over && !m_cancel);
}

//***************************************************************************
void Kwave::RIFFParser::fixGarbageEnds()
{
    qDebug("fixing ends of garbage chunks...");

    Kwave::RIFFChunkList chunks;
    listAllChunks(m_root, chunks);
    QListIterator<Kwave::RIFFChunk *> it1(chunks);
    QListIterator<Kwave::RIFFChunk *> it2(chunks);

    // try all combinations of chunks
    if (it1.hasNext()) it1.next();
    while (it1.hasNext() && !m_cancel) {
        Kwave::RIFFChunk *c1 = it1.next();
        it2 = it1;
        if (it2.hasNext()) it2.next();
        while (it2.hasNext() && !m_cancel) {
            Kwave::RIFFChunk *c2 = it2.next();

            // children always overlap their parents
            if (c2->isChildOf(c1)) continue;

            // get ranges
            quint32 s1 = c1->physStart();
            quint32 e1 = c1->physEnd();
            quint32 s2 = c2->physStart();
            quint32 e2 = c2->physEnd();

            // check for overlaps
            if ((s2 <= e1) && (e2 >= s1)) {
                qDebug("overlap detected:");
                qDebug("    at 0x%08X...0x%08X '%s'",
                    s1, e1, c1->name().data());
                qDebug("    at 0x%08X...0x%08X '%s'",
                    s2, e2, c2->name().data());

                if ((c1->type() == Kwave::RIFFChunk::Garbage) && (s1 < s2)) {
                    // shorten garbage
                    e1 = s2 - 1;
                    quint32 len = e1 - s1 + 1;
                    qDebug("shortening garbage to %u bytes", len);
                    c1->setLength(len);
                }
            }
        }
    }

}

//***************************************************************************
bool Kwave::RIFFParser::joinGarbageToEmpty()
{
    qDebug("joining garbage to empty chunks (and to garbage)...");

    Kwave::RIFFChunkList chunks;
    listAllChunks(m_root, chunks);
    QMutableListIterator<Kwave::RIFFChunk *> it1(chunks);
    QMutableListIterator<Kwave::RIFFChunk *> it2(chunks);

    // join garbage to empty chunks
    if (it2.hasNext()) it2.next();
    while (it2.hasNext() && it1.hasNext() && !m_cancel) {
        Kwave::RIFFChunk *chunk = it1.next();
        Kwave::RIFFChunk *next  = it2.next();
        if (!chunk || !next) continue;
        bool join = false;

        if ( ((chunk->type() == Kwave::RIFFChunk::Empty) ||
            (chunk->dataLength() == 0)) &&
            ((next->type() == Kwave::RIFFChunk::Garbage) ||
                (!isKnownName(next->name())) ) )
        {
            // join garbage and unknown stuff to empty
            join = true;
        }

        if ( (chunk->type() == Kwave::RIFFChunk::Garbage) &&
            (next->type() == Kwave::RIFFChunk::Garbage) )
        {
            // join garbage to garbage
            join = true;
        }

        if (join) {
            if ((next->type() == Kwave::RIFFChunk::Garbage) ||
                (!isKnownName(next->name())) )
            {
                quint32 len = next->physLength() + 4;
                qDebug("joining garbage to empty chunk '%s' "
                       "at 0x%08X, %u bytes",
                       chunk->name().data(), chunk->physStart(), len);
                chunk->setLength(len);
                chunk->setType(guessType(chunk->name()));

                // remove the garbage chunk, it's no longer needed
                it2.remove();
                if (next->parent())
                    next->parent()->subChunks().removeAll(next);
                delete next;

                if (chunk->type() == Kwave::RIFFChunk::Main) {
                    // was joined to a main chunk -> parse again!
                    chunk->setFormat(read4ByteString(chunk->physStart() + 8));
                    parse(chunk, chunk->dataStart(), chunk->dataLength());
                }

                // need_one_more_pass !
                return true;
            }
        }
    }

    return false;
}

//***************************************************************************
void Kwave::RIFFParser::discardGarbage(Kwave::RIFFChunk &chunk)
{
    QMutableListIterator<Kwave::RIFFChunk *> it(chunk.subChunks());
    while (it.hasNext()) {
        Kwave::RIFFChunk *ch = it.next();
        if (m_cancel) break;
        if (!ch) continue;
        if (ch->type() == Kwave::RIFFChunk::Garbage) {
            // garbage found -> deleting it
            it.remove();
            delete ch;
        } else {
            // recursively delete garbage
            discardGarbage(*ch);
        }
    }

}

//***************************************************************************
void Kwave::RIFFParser::cancel()
{
    qDebug("RIFFParser: --- cancel ---");
    m_cancel = true;
}

//***************************************************************************
//***************************************************************************
