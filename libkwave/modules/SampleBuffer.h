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

#include <QObject>

#include "libkwave/KwaveSampleArray.h"
#include "libkwave/modules/KwaveStreamObject.h"

//***************************************************************************
namespace Kwave {

    class KDE_EXPORT SampleBuffer: public Kwave::StreamObject
    {
	Q_OBJECT
    public:

	/** Constructor */
	SampleBuffer();

	/** Destructor */
	virtual ~SampleBuffer();

	/** returns true if no sample data is present */
	virtual bool isEmpty() const;

	/** returns a reference to the sample data */
	virtual Kwave::SampleArray &data();

	/** emit the sample data stored in m_data */
	virtual void done();

    public slots:

	/** slot for taking input data, stores it into m_data */
	virtual void input(Kwave::SampleArray data);

    signals:

	/** emits the data received via input() */
	void output(Kwave::SampleArray data);

    private:

	/** array with sample data */
	Kwave::SampleArray m_data;

    };

}

#endif /* _SAMPLE_BUFFER_H_ */
