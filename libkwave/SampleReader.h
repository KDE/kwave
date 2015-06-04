/***************************************************************************
         SampleReader.h  -  stream for reading samples from a track
			     -------------------
    begin                : Apr 25 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#ifndef SAMPLE_READER_H
#define SAMPLE_READER_H

#include "config.h"

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QTime>

#include <kdemacros.h>

#include "libkwave/InsertMode.h"
#include "libkwave/ReaderMode.h"
#include "libkwave/Sample.h"
#include "libkwave/Stripe.h"
#include "libkwave/SampleArray.h"
#include "libkwave/SampleSource.h"

namespace Kwave
{

    class Q_DECL_EXPORT SampleReader: public Kwave::SampleSource
    {
	Q_OBJECT
    public:

	/**
	 * Constructor. Creates a stream for reading samples from a track.
	 * @param mode the reader mode, see Kwave::ReaderMode
	 * @param stripes list of stripes which contains the desired range
	 * @see InsertMode
	 */
	SampleReader(Kwave::ReaderMode mode, Kwave::Stripe::List stripes);

	/** Destructor */
	virtual ~SampleReader();

	/** Resets the stream to it's start */
	void reset();

	/**
	 * Each KwaveSampleSource has to derive this method for producing
	 * sample data. It then should emit a signal like this:
	 * "output(SampleArray &data)"
	 */
	virtual void goOn();

	/** Checks if the last read operation has reached the end of input */
	inline bool eof() const {
	    return (pos() > m_last);
	}

	/**
	 * Returns true if the end of the source has been reached,
	 * e.g. at EOF of an input stream.
	 *
	 * @return true if it can produce more sample data, otherwise false
	 * @see eof()
	 */
	virtual bool done() const { return eof(); }

	/**
	 * Reads samples into a buffer.
	 * @param buffer the destination buffer to receive the samples
	 * @param dstoff offset within the destination buffer
	 * @param length number of samples to read
	 * @return number of read samples
	 */
	unsigned int read(Kwave::SampleArray &buffer, unsigned int dstoff,
	                  unsigned int length);

	/**
	 * Returns the minimum and maximum sample value within a range
	 * of samples.
	 * @param first index of the first sample
	 * @param last index of the last sample
	 * @param min receives the lowest value or 0 if no samples are in range
	 * @param max receives the highest value or 0 if no samples are in range
	 *
	 * @note min and max do not need to be initialized
	 */
	void minMax(sample_index_t first, sample_index_t last,
	            sample_t &min, sample_t &max);

	/** Skips a number of samples. */
	void skip(sample_index_t count);

	/** Seeks to a given position */
	void seek(sample_index_t pos);

	/**
	 * Returns the current read position.
	 */
	inline sample_index_t pos() const {
	    return (m_src_position + m_buffer_position - m_buffer_used);
	}

	/** Returns the position of the first sample */
	inline sample_index_t first() const {
	    return m_first;
	}

	/** Returns the position of the last sample */
	inline sample_index_t last() const {
	    return m_last;
	}

	/**
	* Reads one single sample.
	*/
	SampleReader& operator >> (sample_t &sample);

	/**
	 * Reads a full buffer of samples. If the buffer cannot be filled,
	 * it will be shrinked to the number of samples that were really
	 * read.
	 */
	SampleReader& operator >> (Kwave::SampleArray &sample);

    signals:

	/** Emitted when the internal buffer is filled or the reader is closed */
	void proceeded();

	/**
	 * Interface for the signal/slot based streaming API.
	 * @param data sample data that has been read
	 */
	void output(Kwave::SampleArray data);

    protected:

	/** Fills the sample buffer */
	void fillBuffer();

	/**
	 * Read a block of samples, with padding if necessary.
	 *
	 * @param offset position where to start the read operation
	 * @param buffer receives the samples
	 * @param buf_offset offset within the buffer
	 * @param length number of samples to read
	 * @return number of read samples
	 */
	unsigned int readSamples(sample_index_t offset,
	                         Kwave::SampleArray &buffer,
	                         unsigned int buf_offset,
	                         unsigned int length);

    private:

	/** operation mode of the reader, see Kwave::ReaderMode */
	Kwave::ReaderMode m_mode;

	/** list of stipes in range */
	QList<Kwave::Stripe> m_stripes;

	/**
	 * Current sample position, related to the source of the samples. Does
	 * not reflect the position of the next sample to be read out due to
	 * internal buffering.
	 * @see pos() for the output position
	 */
	sample_index_t m_src_position;

	/** first sample index */
	sample_index_t m_first;

	/** last sample index */
	sample_index_t m_last;

	/** intermediate buffer for the input data */
	Kwave::SampleArray m_buffer;

	/** number of used elements in the buffer */
	unsigned int m_buffer_used;

	/** read position within the buffer */
	unsigned int m_buffer_position;

	/** timer for limiting the number of progress signals per second */
	QTime m_progress_time;

	/** last seek position, needed in SinglePassReverse mode */
	sample_index_t m_last_seek_pos;

    };
}

#endif /* SAMPLE_READER_H */

//***************************************************************************
//***************************************************************************
