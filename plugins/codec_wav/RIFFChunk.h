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

class RIFFChunk;

/** shortcut for list of RIFF chunks */
typedef QList<RIFFChunk> RIFFChunkList;

/**
 * @class RIFFChunk
 * Stores information about a RIFF chunk, containing name, type, position
 * in the source and length.
 */
class RIFFChunk
{
public:
    /**
     * State of a chunk. Most are a sub-chunks. If one is known to contain
     * further sub-chunks, it is marked as a main chunk. main and sub chunks
     * always have a valid name and some length information, but might be
     * truncated. If the name is not valid, the chunk is considered to
     * contain only garbage. If the name is valid but there is no length
     * information the chunk is marked as "empty".
     */
    typedef enum {
        Root,    /**< virtual root node of the RIFF structure */
        Main,    /**< contains sub-chunks */
        Sub,     /**< valid/sane */
        Garbage, /**< no or invalid name */
        Empty    /**< valid name, but no size */
    } ChunkType;

    /**
     * Constructor.
     * @param parent pointer to the parent node (or null if root node)
     * @param name the 4-byte name of the chunk
     * @param format the 4-byte format specifier, only valid for
     *               main chunks, contains invalid data in sub-chunks
     * @param length size of the chunk's data
     * @param phys_offset start of the chunk name in the source
     * @param phys_length length allocated in the source (file)
     */
    RIFFChunk(RIFFChunk *parent, const QCString &name,
              const QCString &format, u_int32_t length,
              u_int32_t phys_offset, u_int32_t phys_length);

    /**
     * Returns the type of the chunk.
     * @see ChunkType
     */
    inline ChunkType type() { return m_type; };

    /** Sets the type of the chunk */
    inline void setType(ChunkType type) { m_type = type;};

    /** Returns the 4-character name of the chunk */
    inline const QCString &name() { return m_name; };

    /**
     * Returns the chunk's format string.
     * @note Only valid for main chunk
     */
    inline const QCString &format() { return m_format; };

    /** Returns the pointer to the parent node */
    inline const RIFFChunk *parent() { return m_parent; };

    /** Returns the full path of this node. */
    inline const QCString path() {
        return ((m_parent) ? m_parent->path() : QCString("")) + "/" + m_name;
    };

    /** Returns the offset where the chunk's data starts. */
    u_int32_t dataStart();

    /** Returns the length of the chunk's data */
    u_int32_t dataLength();

    /**
     * Returns the length of the chunk in bytes, like stated in the
     * head of the chunk. Includes the format when it's a main chunk.
     */
    inline u_int32_t length() { return m_chunk_length; };

    /**
     * Returns the offset in the source (file) where the
     * chunk (name) starts.
     */
    inline u_int32_t physStart() { return m_phys_offset; };

    /**
     * Returns the offset in the source (file) where the chunk ends.
     */
    u_int32_t physEnd();

    /**
     * Returns the length of the chunk in the file. For some dubious
     * reason this seems always to be rounded up for even numbers!
     */
    inline u_int32_t physLength() { return m_phys_length; };

    /**
     * Returns a reference to the list of sub-chunks.
     */
    inline RIFFChunkList &subChunks() { return m_sub_chunks; };

    /**
     * Dumps the structure of this chunks and all sub-chunks,
     * useful for debugging.
     */
    void dumpStructure();

private:
    /** type of this chunk: main or sub chunk */
    ChunkType m_type;

    /** chunk name, always 4 bytes ASCII */
    QCString m_name;

    /** format of the chunk, only valid for main chunks */
    QCString m_format;

    /** path of the parent chunk */
    RIFFChunk *m_parent;

    /** length of the chunk */
    u_int32_t m_chunk_length;

    /** offset within the source (file) */
    u_int32_t m_phys_offset;

    /** length used in the source (file) */
    u_int32_t m_phys_length;

    /** list of sub-chunks, empty if none known */
    RIFFChunkList m_sub_chunks;

};

#endif /* _RIFF_CHUNK_H_ */
