/***************************************************************************
         SampleWriter.h  -  stream for writing samples into a track
			     -------------------
    begin                : Feb 11 2001
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

#ifndef _SAMPLE_WRITER_H_
#define _SAMPLE_WRITER_H_

#include <qarray.h>
#include <qlist.h>
#include <qobject.h>

#include "libkwave/InsertMode.h"
#include "libkwave/Sample.h"

class SampleLock;
class SampleReader;
class Stripe;
class Track;

/**
 * @class SampleWriter
 * Input stream for transferring samples into a Track. Internally holds a
 * list of locks for all affected stripes.
 *
 * @warning THIS CLASS IS NOT THREADSAFE! It is intended to be owned by
 *          and used from only one thread.
 * @note it is derived from QObject in order to get a <code>destroyed</code>
 *       signal if the writer is closed/deleted. This is needed for
 *       determining the end of the write access, e.g. for closing an
 *       undo transaction.
 */
class SampleWriter: public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor. Creates an input stream and locks all
     * necessary stripes within the track.
     * @param track
     * @param stripes list of stripes, already locked for us
     * @param lock the lock for the needed range of samples
     * @param mode specifies where and how to insert
     * @param left start of the input (only useful in insert and
     *             overwrite mode)
     * @param right end of the input (only useful with overwrite mode)
     * @see InsertMode
     */
    SampleWriter(Track &track, QList<Stripe> &stripes,
	SampleLock *lock, InsertMode mode,
	unsigned int left = 0, unsigned int right = 0);

    /**
     * Destructor. Unlocks all stripes.
     */
    virtual ~SampleWriter();

    /** operator for inserting an array of samples */
    SampleWriter &operator << (const QArray<sample_t> &samples);

    /** operator for inserting a single sample */
    SampleWriter &operator << (const sample_t &sample);

    /** operator for simple modifiers like flush() */
    inline SampleWriter &operator << (
	SampleWriter &(*modifier)(SampleWriter &))
    {
	return modifier(*this);
    }

    /**
     * Fill the SampleWriter with data from a SampleReader. If the reader
     * reaches EOF the writer will be filled up with zeroes.
     */
    SampleWriter &operator << (SampleReader &reader);

    /**
     * Flush the content of a buffer. Normally the buffer is the
     * internal intermediate buffer used for single-sample writes.
     * When using block transfers, the internal buffer is bypassed
     * and the written block is passed instead.
     * @internal
     * @param buffer reference to the buffer to be flushed
     * @param count number of samples in the buffer to be flushed,
     *              will be internally set to zero if successful
     */
    void flush(const QArray<sample_t> &buffer, unsigned int &count);

    /**
     * Shortcut for flush(m_buffer, m_buffer_used)
     * @internal
     */
    inline void flush() { flush(m_buffer, m_buffer_used); };

    /** Returns the index of the first sample of the range. */
    inline unsigned int first() { return m_first; };

    /**
     * Returns the current index of the last sample in range or the
     * index of the last written sample when in insert/append mode.
     */
    inline unsigned int last() { return m_last; };

signals:

    /**
     * Is emitted once immediately before the writer gets closed and tells
     * the receiver the total number of written samples.
     */
    void sigSamplesWritten(unsigned int);

private:

    /** first sample */
    unsigned int m_first;

    /** last sample */
    unsigned int m_last;

    /** mode for input (insert, overwrite, ...) */
    InsertMode m_mode;

    /** the track that receives our data */
    Track &m_track;

    /** array with our stripes */
    QList<Stripe> m_stripes;

    /** locks for our range of samples */
    SampleLock *m_lock;

    /** current position within the track */
    unsigned int m_position;

    /** intermediate buffer for the input data */
    QArray<sample_t> m_buffer;

    /** number of used elements in the buffer */
    unsigned int m_buffer_used;

};

/** modifier for flushing */
SampleWriter &flush(SampleWriter &s);

#endif /* _SAMPLE_WRITER_H_ */
