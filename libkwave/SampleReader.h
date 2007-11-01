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

#ifndef _SAMPLE_READER_H_
#define _SAMPLE_READER_H_

#include "config.h"
#include <qmemarray.h>
#include <qptrlist.h>
#include <qobject.h>

#include "libkwave/InsertMode.h"
#include "libkwave/Sample.h"
#include "libkwave/KwaveSampleSource.h"

class SampleLock;
class Stripe;
class Track;

class SampleReader: public Kwave::SampleSource
{
    Q_OBJECT
public:

    /**
     * Constructor. Creates a stream for reading samples from a track
     * and locks all necessary stripes.
     * @param track
     * @param stripes list of stripes, already locked for us
     * @param lock a lock for the needed range of samples (ReadShared)
     * @param left start of the input (only useful in insert and
     *             overwrite mode)
     * @param right end of the input (only useful with overwrite mode)
     * @see InsertMode
     */
    SampleReader(Track &track, QPtrList<Stripe> &stripes,
	SampleLock *lock, unsigned int left, unsigned int right);

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
    inline bool eof() {
	return (pos() > m_last);
    };

    /**
     * Returns true if the end of the source has been reached,
     * e.g. at EOF of an input stream.
     *
     * @return true if it can produce more sample data, otherwise false
     * @see eof()
     */
    virtual bool done() { return eof(); };

    /**
     * Reads samples into a buffer.
     * @param buffer the destination buffer to receive the samples
     * @param dstoff offset within the destination buffer
     * @param length number of samples to read
     * @return number of read samples
     */
    unsigned int read(QMemArray<sample_t> &buffer, unsigned int dstoff,
	unsigned int length);

    /** Skips a number of samples. */
    void skip(unsigned int count);

    /** Seeks to a given position */
    void seek(unsigned int pos);

    /**
     * Returns the current read position.
     */
    inline unsigned int pos() {
	return (m_src_position + m_buffer_position - m_buffer_used);
    };

    /** Returns the position of the first sample */
    inline unsigned int first() const {
	return m_first;
    };

    /** Returns the position of the last sample */
    inline unsigned int last() const {
	return m_last;
    };

    /**
     * Reads one single sample.
     */
    SampleReader& operator >> (sample_t &sample);

    /**
     * Reads a full buffer of samples. If the buffer cannot be filled,
     * it will be shrinked to the number of samples that were really
     * read.
     */
    SampleReader& operator >> (QMemArray<sample_t> &sample);

signals:

    /** Emitted when the internal buffer is filled or the reader is closed */
    void proceeded();

    /**
     * Interface for the signal/slot based streaming API.
     * @param data sample data that has been read
     */
    void output(Kwave::SampleArray &data);

protected:

    /** Fills the sample buffer */
    void fillBuffer();

private:

    /** the track to which we belong */
    Track &m_track;

    /** list of stripes with sample data */
    QPtrList<Stripe> m_stripes;

    /** lock for the needed range of samples */
    SampleLock* m_lock;

    /**
     * Current sample position, related to the source of the samples. Does
     * not reflect the position of the next sample to be read out due to
     * internal buffering.
     * @see pos() for the output position
     */
    unsigned int m_src_position;

    /** first sample index */
    unsigned int m_first;

    /** last sample index */
    unsigned int m_last;

    /** intermediate buffer for the input data */
    QMemArray<sample_t> m_buffer;

    /** number of used elements in the buffer */
    unsigned int m_buffer_used;

    /** read position within the buffer */
    unsigned int m_buffer_position;

};

#endif /* _SAMPLE_READER_H_ */
