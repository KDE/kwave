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

#include <QtCore/QMutex>
#include <QtCore/QQueue>

#include <kdemacros.h>

#include "libkwave/SampleArray.h"

namespace Kwave
{

    class KDE_EXPORT SampleFIFO
    {
    public:
	/** Constructor */
	SampleFIFO();

	/** copy constructor */
	SampleFIFO(const SampleFIFO &other);

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
	 * @param source reference to an array of samples to feed in
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

	/**
	 * sets the maximum size of the content
	 */
	virtual void setSize(unsigned int size);

	/**
	 * discards all superflous content until the size
	 * condition is met.
	 */
	virtual void crop();

    private:

	/** internal version of length(), without locking */
	unsigned int unlockedLength();

    private:

	/** list of buffers with sample data */
	QQueue<Kwave::SampleArray> m_buffer;

	/** maximum number of samples of the content */
	sample_index_t m_size;

	/**
	 * number of samples that have already been read out
	 * from the first buffer (head, first one to read out)
	 */
	sample_index_t m_read_offset;

	/** mutex for access to the FIFO (recursive) */
	QMutex m_lock;

    };
}

#endif /* _SAMPLE_FIFO_H_ */

//***************************************************************************
//***************************************************************************
