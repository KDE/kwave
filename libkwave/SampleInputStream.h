/***************************************************************************
    SampleInputStream.h  -  stream for inserting samples into a track
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

#ifndef _SAMPLE_INPUT_STREAM_H_
#define _SAMPLE_INPUT_STREAM_H_

#include <qarray.h>
#include <qlist.h>

#include "mt/MutexSet.h"
#include "libkwave/InsertMode.h"
#include "libkwave/Sample.h"

class Stripe;
class Track;

/**
 * \class SampleInputStream
 * Input stream for transferring samples into a Track. Internally holds a
 * list of locks for all affected stripes.
 *
 * THIS CLASS IS NOT THREADSAFE! It is intended to be owned by and used
 * from only one thread.
 */
class SampleInputStream
{
public:
    /**
     * Constructor. Creates an input stream an locks all
     * necessary stripes within the track.
     * @param track
     * @param stripes list of stripes, already locked for us
     * @param locks set of locks for the stripes
     * @param mode specifies where and how to insert
     * @param left start of the input (only useful in insert and
     *             overwrite mode)
     * @param right end of the input (only useful with overwrite mode)
     * @see InsertMode
     */
    SampleInputStream(Track &track, QList<Stripe> &stripes,
	MutexSet &locks, InsertMode mode,
	unsigned int left = 0, unsigned int right = 0);

    /**
     * Destructor. Unlocks all stripes.
     */
    virtual ~SampleInputStream();

    /** operator for inserting an array of samples */
    SampleInputStream &operator << (const QArray<sample_t> &samples);

    /** operator for inserting a single sample */
    SampleInputStream &operator << (const sample_t &sample);

    /** operator for simple modifiers like flush() */
    SampleInputStream &operator << (
	SampleInputStream &(*modifier)(SampleInputStream &))
    {
	return modifier(*this);
    }

    /** flush the content of the intermediate buffer */
    SampleInputStream &flush();

private:
    /** mode for input (insert, overwrite, ...) */
    InsertMode m_mode;

    /** the track that receives our data */
    Track &m_track;

    /** array with our stripes */
    QList<Stripe> m_stripes;

    /** set of locks for our stripes */
    MutexSet m_locks;

    /** current position within the track */
    unsigned int m_position;

    /** intermediate buffer for the input data */
    QArray<sample_t> m_buffer;

    /** number of used elements in the buffer */
    unsigned int m_buffer_used;

};

/** modifier for flushing */
SampleInputStream &flush(SampleInputStream &s);

#endif /* _SAMPLE_INPUT_STREAM_H_ */
