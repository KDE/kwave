/*************************************************************************
           SampleFIFO.h  -  simple FIFO, tuned for sample_t
                             -------------------
    begin                : Sun Apr 11 2004
    copyright            : (C) 2004 by Thomas Eschenbacher
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

#ifndef _SAMPLE_FIFO_H_
#define _SAMPLE_FIFO_H_

#include "config.h"
#include <qmemarray.h>
#include "libkwave/Sample.h"

class SampleFIFO
{
public:
    /**
     * Constructor
     *
     * @param size initial number of samples the FIFO can hold
     */
    SampleFIFO(unsigned int size = 0);

    /** Destructor */
    virtual ~SampleFIFO();

    /**
     * Set the size of the FIFO. This destroys the content and sets
     * all pointers to their initial value.
     *
     * @param size new depth of the FIFO in samples
     */
    virtual void resize(unsigned int size);

    /**
     * puts samples into the FIFO
     *
     * @param source reference to an array of samples to feed in
     * @return true if successful, false on overruns
     */
    virtual bool put(const QMemArray<sample_t> &source);

    /**
     * Align/shift the data in the FIFO so that the current read
     * position is at the start of the internal buffer.
     */
    virtual void align();

    /**
     * Returns the number of samples that can be read out.
     * @see m_written
     */
    virtual unsigned int length();

    /**
     * Returns a reference to the internal buffer. Normally called
     * after "align" to forward the internal buffer to a SampleWriter
     * or similar.
     */
    virtual QMemArray<sample_t> &data();

private:

    /** size of the FIFO in samples */
    unsigned int m_size;

    /** buffer for the sample data */
    QMemArray<sample_t> m_buffer;

    /** number of written samples [0...m_size-1] */
    unsigned int m_written;

    /**
     * index of the write position, where the next sample would
     * be written.
     */
    unsigned int m_write_pos;

};

#endif /* _SAMPLE_FIFO_H_ */
