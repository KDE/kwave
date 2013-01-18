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

#ifndef _SAMPLE_BUFFER_H_
#define _SAMPLE_BUFFER_H_

#include "config.h"

#include <QtCore/QObject>

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
	virtual const Kwave::SampleArray &data() const;

	/** returns the number of samples that can be fetched with get() */
	virtual unsigned int available() const;

	/**
	 * returns a pointer to the raw data and advances the internal
	 * offset afterwards
	 * @param length maximum number of samples to get
	 * @return pointer to the raw data in the buffer
	 */
	virtual const sample_t *get(unsigned int len);

	/** emit the sample data stored in m_data */
	virtual void finished();

    public slots:

	/** slot for taking input data, stores it into m_data */
	virtual void input(Kwave::SampleArray data);

    signals:

	/** emits the data received via input() */
	void output(Kwave::SampleArray data);

    private:

	/** array with sample data */
	Kwave::SampleArray m_data;

	/** offset within the data, for reading */
	unsigned int m_offset;
    };

}

#endif /* _SAMPLE_BUFFER_H_ */

//***************************************************************************
//***************************************************************************
