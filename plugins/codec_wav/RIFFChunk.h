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

#ifndef RIFF_CHUNK_H
#define RIFF_CHUNK_H

#include "config.h"

#include <QByteArray>
#include <QList>
#include <QString>

namespace Kwave
{

    class RIFFChunk;

    /** shortcut for list of RIFF chunks */
    typedef QList<Kwave::RIFFChunk *> RIFFChunkList;

    /**
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
	    Sub,     /**< valid/sane sub-chunk */
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
	RIFFChunk(Kwave::RIFFChunk *parent, const QByteArray &name,
	          const QByteArray &format, quint32 length,
	          quint32 phys_offset, quint32 phys_length);

	/** Destructor */
	virtual ~RIFFChunk();

	/**
	 * Returns true if the file chunk no structural errors and
	 * no garbage or empty chunks.
	 */
	bool isSane() const;

	/**
	 * Returns the type of the chunk.
	 * @see ChunkType
	 */
	inline ChunkType type() const { return m_type; }

	/** Sets the type of the chunk */
	inline void setType(ChunkType type) { m_type = type; }

	/** Returns the 4-character name of the chunk */
	inline const QByteArray &name() const { return m_name; }

	/**
	 * Returns the chunk's format string.
	 * @note Only valid for main chunk
	 */
	inline const QByteArray &format() const { return m_format; }

	/** Sets the format to a new value, without any error checking */
	inline void setFormat(const QByteArray &format) { m_format = format; }

	/** Returns the pointer to the parent node */
	inline Kwave::RIFFChunk *parent() const { return m_parent; }

	/**
	 * Returns the full path of this node. If the node is a "Main" chunk
	 * and has a format parameter, the format is appended, separated with
	 * a ":". If the chunk name is not unique within it's parents the
	 * zero based index is appended within round brackets.
	 */
	const QByteArray path() const;

	/** Returns the offset where the chunk's data starts. */
	quint32 dataStart() const;

	/** Returns the physical length of the chunk's data */
	quint32 dataLength() const;

	/**
	 * Returns the length of the chunk in bytes, like stated in the
	 * head of the chunk. Includes the format when it's a main chunk.
	 */
	inline quint32 length() const { return m_chunk_length; }

	/**
	 * Sets the data and physical length of the chunk both to a
	 * new value.
	 */
	void setLength(quint32 length);

	/**
	 * Returns the offset in the source (file) where the
	 * chunk (name) starts.
	 */
	inline quint32 physStart() const { return m_phys_offset; }

	/**
	 * Returns the offset in the source (file) where the chunk ends.
	 */
	quint32 physEnd() const;

	/**
	 * Returns the length of the chunk in the file. For some dubious
	 * reason this seems always to be rounded up for even numbers!
	 */
	inline quint32 physLength() const { return m_phys_length; }

	/**
	 * Returns a reference to the list of sub-chunks (mutable).
	 */
	inline Kwave::RIFFChunkList &subChunks() { return m_sub_chunks; }

	/**
	 * Returns a reference to the list of sub-chunks (const).
	 */
	inline const Kwave::RIFFChunkList &subChunks() const {
	    return m_sub_chunks;
	}

	/**
	 * Returns true if the given chunk is a parent of us.
	 */
	bool isChildOf(Kwave::RIFFChunk *chunk);

	/**
	 * Fixes descrepancies in the size of the chunk. The new size will be
	 * computed as the size of all sub-chunks (that will be recursively
	 * fixed too) plus the own header.
	 */
	void fixSize();

	/**
	 * Dumps the structure of this chunks and all sub-chunks,
	 * useful for debugging.
	 */
	void dumpStructure();

    private:
	/** type of this chunk: main or sub chunk */
	ChunkType m_type;

	/** chunk name, always 4 bytes ASCII */
	QByteArray m_name;

	/** format of the chunk, only valid for main chunks */
	QByteArray m_format;

	/** path of the parent chunk */
	Kwave::RIFFChunk *m_parent;

	/** length of the chunk */
	quint32 m_chunk_length;

	/** offset within the source (file) */
	quint32 m_phys_offset;

	/** length used in the source (file) */
	quint32 m_phys_length;

	/** list of sub-chunks, empty if none known */
	Kwave::RIFFChunkList m_sub_chunks;

    };
}

#endif /* RIFF_CHUNK_H */

//***************************************************************************
//***************************************************************************
