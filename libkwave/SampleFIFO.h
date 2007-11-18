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
#include <qvaluevector.h>
#include <qmutex.h>
#include "libkwave/KwaveSampleArray.h"

class SampleFIFO
{
public:
    /** Constructor */
    SampleFIFO();

    /** Destructor */
    virtual ~SampleFIFO();

    /**
     * Reset the FIFO. This destroys the content and sets
     * all pointers to their initial value.
     */
     virtual void flush();

    /**
     * puts samples into the FIFO
     *
     * @param buffer reference to an array of samples to feed in
     */
    virtual void put(const Kwave::SampleArray &source);

    /**
     * gets and removes samples from the FIFO
     *
     * @param buffer reference to an array of samples to be filled
     * @return number of received samples
     */
    virtual unsigned int get(Kwave::SampleArray &buffer);

    /**
     * Returns the number of samples that can be read out.
     * @see m_written
     */
    virtual unsigned int length();

private:

    /** list of buffers with sample data */
    QValueVector<Kwave::SampleArray> m_buffer;

    /**
     * number of samples that have already been read out
     * from the first buffer (head, first one to read out)
     */
    unsigned int m_read_offset;

    /** mutex for access to the FIFO (recursive) */
    QMutex m_lock;

};

#endif /* _SAMPLE_FIFO_H_ */
