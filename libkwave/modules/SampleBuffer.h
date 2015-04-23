/*************************************************************************
         SampleBuffer.h  -  simple buffer for sample arrays
                             -------------------
    begin                : Sun Oct 17 2010
    copyright            : (C) 2010 by Thomas Eschenbacher
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

#ifndef SAMPLE_BUFFER_H
#define SAMPLE_BUFFER_H

#include "config.h"

#include <QtCore/QObject>
#include <QtCore/QSemaphore>

#include "libkwave/SampleArray.h"
#include "libkwave/SampleSink.h"

//***************************************************************************
namespace Kwave
{

    class KDE_EXPORT SampleBuffer: public Kwave::SampleSink
    {
	Q_OBJECT
    public:

        /**
         * Constructor
         * @param parent a parent object, passed to QObject (optional)
         */
	SampleBuffer(QObject *parent = 0);

	/** Destructor */
	virtual ~SampleBuffer();

	/** returns true if no sample data is present */
	virtual bool isEmpty() const;

	/** returns a reference to the sample data */
	virtual Kwave::SampleArray &data();

	/** returns a const reference to the sample data */
	virtual const Kwave::SampleArray &constData() const;

	/** returns the number of samples that can be fetched with get() */
	virtual unsigned int available() const;

	/**
	 * returns a pointer to the raw data and advances the internal
	 * offset afterwards
	 * @param len maximum number of samples to get
	 * @return pointer to the raw data in the buffer
	 */
	virtual const sample_t *get(unsigned int len);

	/**
	 * Appends one samples to the buffer. If the buffer is full afterwards,
	 * the buffer will be emitted.
	 *
	 * @param sample a single sample
	 */
	void put(sample_t sample);

	/** emit the sample data stored in m_data */
	virtual void finished();

    public slots:

	/** slot for taking input data, stores it into m_data */
	virtual void input(Kwave::SampleArray data);

    protected:

	friend class BufferJob;

	/**
	 * Emit data from this object, used from a buffer thread
	 * @param data a sample array to emit
	 */
	void emitData(Kwave::SampleArray data);

    private:

	/** enqueue some data */
	void enqueue(Kwave::SampleArray data);

    signals:

	/** emits the data received via input() */
	void output(Kwave::SampleArray data);

    private:

	/** array with sample data */
	Kwave::SampleArray m_data;

	/** offset within the data, for reading */
	unsigned int m_offset;

	/** number of samples buffered, e.g. through put() */
	unsigned int m_buffered;

	/** semaphore for limiting queue depth to 1 */
	QSemaphore m_sema;
    };

}

#endif /* SAMPLE_BUFFER_H */

//***************************************************************************
//***************************************************************************
