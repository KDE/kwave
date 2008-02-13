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

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QStringList>

#include "RIFFChunk.h"

class QIODevice;
class RIFFChunk;

class RIFFParser: public QObject
{
    Q_OBJECT
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
     * Returns true if the source contains no structural errors and
     * no garbage or empty chunks.
     */
    bool isSane();

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
    RIFFChunk *findChunk(const QByteArray &path);

    /** Returns the number of times a chunk is present */
    unsigned int chunkCount(const QByteArray &path);

    /**
     * Tries very hard to find a missing chunk by stepping through
     * the whole file or source.
     * @param name the 4-byte name of the chunk
     * @return the found chunk or zero if nothing found
     */
    RIFFChunk *findMissingChunk(const QByteArray &name);

    /**
     * Tries to repair the RIFF file by solving inconsistencies
     * @see collectGarbage()
     * @see joinGarbageToEmpty()
     * @see fixGarbageEnds()
     * @see discardGarbage()
     */
    void repair();

public slots:

   /** Cancels the current action. */
   void cancel();

signals:

    /** emits the name of the currently performed action */
    void action(const QString &name);

    /** emits a progress in percent */
    void progress(int percent);

protected:

    /**
     * Reads a 4-byte string from the source device at a given
     * offset. The current source position will be set to 4 bytes
     * after the given offset afterwards.
     * @param offset position within the source, no range checks!
     * @return string with 4 bytes
     */
    QByteArray read4ByteString(u_int32_t offset);

    /**
     * Tries to find the chunk name in the list of known main
     * chunk names and returns RIFFChunk::Main if successful.
     * Otherwise returns RIFFChunk::Sub or RIFFChunk::Garbage
     * if the chunk name is not valid.
     * @param name the name of the chunk
     * @return RIFFChunk::Main or RIFFChunk::Sub or
     *         RIFFChunk::Garbage
     */
    RIFFChunk::ChunkType guessType(const QByteArray &name);

    /** Returns true if the given chunk name is valid */
    bool isValidName(const char *name);

    /** Returns true if the given chunk name is known as main or sub chunk */
    bool isKnownName(const QByteArray &name);

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
     * @return pointer to the new created chunk
     */
    RIFFChunk *addChunk(RIFFChunk *parent, const QByteArray &name,
                        const QByteArray &format, u_int32_t length,
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
    bool addEmptyChunk(RIFFChunk *parent, const QByteArray &name,
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
     * source. Also emits progress bar info from
     * [(start/count) ... ((start+1)/count-1)]
     * @param name the name of the chunk to be found
     * @param offset position for start of the scan
     * @param length number of bytes to scan
     * @param progress_start start of the progress [0..progress_count-1]
     * @param progress_count number of progress sections
     * @return list of positions of where the name exists
     */
    QList<u_int32_t> scanForName(const QByteArray &name, u_int32_t offset,
                                 u_int32_t length,
                                 int progress_start = 0,
                                 int progress_count = 1);

private:

    /**
     * clear all main chunks that contain only garbage and
     * convert them into garbage chunks
     */
    void collectGarbage();

    /**
     * joins garbage to previous empty chunks. If a chunk follows a previously
     * empty chunk with an unknown name it will be joined too.
     * @return true if it needs an additional pass
     */
    bool joinGarbageToEmpty();

    /** fixes the end of garbage chunks to no longer overlap valid chunks */
    void fixGarbageEnds();

    /** Discards all garbage sub-chunks */
    void discardGarbage(RIFFChunk &chunk);

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

    /** can be set to true in order to cancel a running operation */
    bool m_cancel;

};

#endif /* _RIFF_PARSER_H_ */
