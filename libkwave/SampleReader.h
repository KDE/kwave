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

#include <qarray.h>
#include <qlist.h>

#include "libkwave/InsertMode.h"
#include "libkwave/Sample.h"

class SampleLock;
class Stripe;
class Track;

class SampleReader
{
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
    SampleReader(Track &track, QList<Stripe> &stripes,
	SampleLock *lock, unsigned int left, unsigned int right);

    /** Destructor */
    virtual ~SampleReader();

    /**
     * Reads samples into a buffer.
     * @param buffer the destination buffer to receive the samples
     * @param dstoff offset within the destination buffer
     * @param length number of samples to read
     * @return number of read samples
     */
    unsigned int read(QArray<sample_t> &buffer, unsigned int dstoff,
	unsigned int length);

private:

    /** the track to which we belong */
    Track &m_track;

    /** list of stripes with sample data */
    QList<Stripe> m_stripes;

    /** lock for the needed range of samples */
    SampleLock* m_lock;

    /** current sample position */
    unsigned int m_position;

    /** last sample index */
    unsigned int m_last;

};

#endif /* _SAMPLE_READER_H_ */
