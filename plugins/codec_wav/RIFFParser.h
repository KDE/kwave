/*************************************************************************
           RIFFParser.h  -  parser for the RIFF format
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

#ifndef _RIFF_PARSER_H_
#define _RIFF_PARSER_H_

#include "config.h"
#include <qlist.h>
#include <qstringlist.h>
#include <qvaluelist.h>

#include "RIFFChunk.h"

class QIODevice;
class QCString;
class RIFFChunk;

class RIFFParser
{
public:

    /** endianness of the RIFF's chunk format */
    typedef enum { Unknown, LittleEndian, BigEndian } Endianness;

    /**
     * Constructor.
     * @param device QIODevice to parse through.
     * @param main_chunks list of known names of main chunks
     * @param known_subchunks list of known subchunks
     */
    RIFFParser(QIODevice &device, const QStringList &main_chunks,
               const QStringList &known_subchunks);

    /** Destructor */
    virtual ~RIFFParser();

    /**
     * Parses the whole source.
     * @return true if passed without any error
     */
    bool parse();

    /*
     * Parses a range of the source and adds all found chunks to a
     * list of chunks. Also parses recursively for sub-chunks and
     * builds a tree.
     * @param parent pointer to the parent node
     * @param offset start of the chunk name in the source
     * @param length size of the area where to search
     * @return true if passed without any error
     */
    bool parse(RIFFChunk *parent, u_int32_t offset, u_int32_t length);

    /**
     * Dumps the structure of all chunks, useful for debugging.
     */
    void dumpStructure();

    /**
     * Tries to find a chunk in the tree of parsed chunks, only
     * accepting a full match.
     * @param path the full path of the chunk to be searched
     * @return the found chunk or zero if nothing found
     */
    RIFFChunk *findChunk(const QCString &path);

    /**
     * Tries very hard to find a missing chunk by stepping through
     * the whole file or source.
     * @param name the 4-byte name of the chunk
     * @return the found chunk or zero if nothing found
     */
    RIFFChunk *findMissingChunk(const QCString &name);

    /**
     * Tries to repair the RIFF file by solving inconsistencies
     * @see resolveOverlaps()
     * @see joinGarbage()
     * @see discardGarbage()
     */
    void repair();

protected:

    /** Returns true if the given chunk name is valid */
    bool isValidName(const char *name);

    /**
     * Tries to detect the endianness of the source. If successful, the
     * endianness will be set. If failed, will be set to "Unknown".
     */
    void detectEndianness();

    /**
     * Creates and adds a new chunk. Will be parented to the first
     * non-garbage chunk.
     * @param parent pointer to the parent node (or null if root node)
     * @param name the 4-byte name of the chunk
     * @param format the 4-byte format specifier, only valid for
     *               main chunks, contains invalid data in sub-chunks
     * @param length size of the chunk's data
     * @param phys_offset start of the chunk name in the source
     * @param phys_length length allocated in the source (file)
     * @param type chunk type, @see RIFFChunk::ChunkType
     * @param RIFFChunk::RIFFChunk()
     * @return pointer to the new created chunk
     */
    RIFFChunk *addChunk(RIFFChunk *parent, const QCString &name,
                        const QCString &format, u_int32_t length,
                        u_int32_t phys_offset, u_int32_t phys_length,
                        RIFFChunk::ChunkType type);

    /**
     * Adds a chunk that has no valid name and thus is not recognized.
     * It is assumed that it only contains undecodeable garbage without
     * any valid name, length or other header information.
     * @param parent pointer to the parent node
     * @param offset start of the chunk name in the source
     * @param length length of the garbage area in bytes
     * @return true if creation succeeded, false if out of memory
     */
    bool addGarbageChunk(RIFFChunk *parent, u_int32_t offset,
                         u_int32_t length);

    /**
     * Adds a chunk with a valid name and no length information.
     * @param parent pointer to the parent node
     * @param name the valid name of the chunk
     * @param offset start of the chunk name in the source
     * @return true if creation succeeded, false if out of memory
     */
    bool addEmptyChunk(RIFFChunk *parent, const QCString &name,
                       u_int32_t offset);

    /**
     * Recursively creates a "flat" list of all chunks.
     * @param parent node to start with
     * @param list receives references to the chunks
     */
    void listAllChunks(RIFFChunk &parent, RIFFChunkList &list);

    /**
     * Returns a pointer to a chunk that starts at a given offset
     * or zero if none found.
     * @param offset the start position (physical start)
     * @return pointer to the chunk or zero
     */
    RIFFChunk *chunkAt(u_int32_t offset);

    /**
     * Performs a scan for a 4-character chunk name over a range of the
     * source.
     * @param name the name of the chunk to be found
     * @param offset position for start of the scan
     * @param length number of bytes to scan
     * @return list of positions of where the name exists
     */
    QValueList<u_int32_t> scanForName(const QCString &name, u_int32_t offset,
                                      u_int32_t length);

private:

    /** Resolves overlapping areas */
    void resolveOverlaps();

    /** Joins garbage after truncated chunks to the chunk */
    void joinGarbage();

    /** Discards all garbage chunks */
    void discardGarbage();

    /** I/O device with the source of the file */
    QIODevice &m_dev;

    /** root chunk of the source */
    RIFFChunk m_root;

    /** list of known names of main chunks */
    QStringList m_main_chunk_names;

    /** list of known names of sub chunks */
    QStringList m_sub_chunk_names;

    /** endianness of the RIFF file, auto-detected */
    Endianness m_endianness;

};

#endif /* _RIFF_PARSER_H_ */
