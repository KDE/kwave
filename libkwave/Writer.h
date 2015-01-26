/***************************************************************************
    Writer.h - base class for writers, providing a C++ stream interface
			     -------------------
    begin                : Sun Aug 23 2009
    copyright            : (C) 2009 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _KWAVE_WRITER_H_
#define _KWAVE_WRITER_H_

#include <config.h>

#include <QtCore/QObject>

#include <kdemacros.h>

#include "libkwave/InsertMode.h"
#include "libkwave/SampleSink.h"

namespace Kwave
{
    class SampleArray;
    class SampleReader;

    class KDE_EXPORT Writer: public Kwave::SampleSink
    {
        Q_OBJECT
    public:

	/** default constructor */
	Writer();

	/**
	 * Constructor. Creates an input stream for writing.
	 *
	 * @param mode specifies where and how to insert
	 * @param left start of the input (only useful in insert and
	 *             overwrite mode)
	 * @param right end of the input (only useful with overwrite mode)
	 * @see InsertMode
	 */
	Writer(Kwave::InsertMode mode,
	       sample_index_t left = 0, sample_index_t right = 0);

	/** Destructor */
	virtual ~Writer();

	/** operator for inserting an array of samples */
	virtual Writer &operator << (const Kwave::SampleArray &samples);

	/** operator for inserting a single sample */
	virtual Writer &operator << (const sample_t &sample);

	/** operator for simple modifiers like flush() */
	inline Writer &operator << (Writer &(*modifier)(Writer &))
	{
	    return modifier(*this);
	}

	/**
	 * Fill the Writer with data from a SampleReader. If the reader
	 * reaches EOF the writer will be filled up with zeroes.
	 */
	Writer &operator << (Kwave::SampleReader &reader);

	/**
	 * Flush the content of a buffer. Normally the buffer is the
	 * internal intermediate buffer used for single-sample writes.
	 * When using block transfers, the internal buffer is bypassed
	 * and the written block is passed instead.
	 * @internal
	 * @param buffer reference to the buffer to be flushed
	 * @param count number of samples in the buffer to be flushed,
	 *              will be internally set to zero if successful
	 * @return true if successful, false if failed (e.g. out of memory)
	 */
	virtual bool write(const Kwave::SampleArray &buffer,
	                   unsigned int &count) = 0;

	/**
	 * Shortcut for flush(m_buffer, m_buffer_used)
	 * @internal
	 */
	inline bool flush() { return write(m_buffer, m_buffer_used); }

	/**
	 * Returns true if the end of the writeable area has been reached if the
	 * writer has been opened in "overwrite" mode. Note that this does not
	 * make sense in append or insert mode, so in these cases the return
	 * value will always be false.
	 */
	virtual bool eof() const;

	/** the same as eof(), needed for the Kwave::SampleSink interface */
	virtual bool done() const { return eof(); }

	/** Returns the index of the first sample of the range. */
	inline sample_index_t first() const { return m_first; }

	/**
	 * Returns the current index of the last sample in range or the
	 * index of the last written sample when in insert/append mode.
	 */
	inline sample_index_t last() const {
	    return ((m_mode == Kwave::Append) ?
		(m_last + m_buffer_used) : m_last);
	}

	/**
	 * Returns the current position
	 */
	inline sample_index_t position() const { return m_position; }

	/** Returns the insert mode */
	inline Kwave::InsertMode mode() const { return m_mode; }

    signals:

	/**
	 * Is emitted once immediately before the writer gets closed and tells
	 * the receiver the total number of written samples.
	 */
	void sigSamplesWritten(sample_index_t);

	/** Emitted when the internal buffer is flushed or the writer is closed */
	void proceeded();

    public slots:

	/**
	 * Interface for the signal/slot based streaming API.
	 * @param data sample data to write
	 */
	void input(Kwave::SampleArray data);

    protected:

	/** first sample */
	sample_index_t m_first;

	/** last sample */
	sample_index_t m_last;

	/** mode for input (insert, overwrite, ...) */
	Kwave::InsertMode m_mode;

	/** current position within the track */
	sample_index_t m_position;

	/** intermediate buffer for the input data */
	Kwave::SampleArray m_buffer;

	/** for speedup: cached buffer size */
	unsigned int m_buffer_size;

	/** number of used elements in the buffer */
	unsigned int m_buffer_used;

    };

}

/** modifier for flushing */
Kwave::Writer &flush(Kwave::Writer &s) KDE_EXPORT;

#endif /* _KWAVE_WRITER_H_ */

//***************************************************************************
//***************************************************************************
