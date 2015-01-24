/***************************************************************************
             Normalizer  -  simple normalizer with limiter
                             -------------------
    begin                : Sat May 09 2009
    copyright            : (C) 2009 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de

    limiter function     : (C) 1999-2005 Chris Vaill <chrisvaill at gmail>
                           taken from "normalize-0.7.7"
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _NORMALIZER_H_
#define _NORMALIZER_H_

#include <config.h>

#include <QtCore/QObject>
#include <QtCore/QVariant>

#include "libkwave/SampleArray.h"
#include "libkwave/SampleSource.h"

namespace Kwave
{
    class Normalizer: public Kwave::SampleSource
    {
	Q_OBJECT
    public:

	/** Constructor */
	Normalizer();

	/** Destructor */
	virtual ~Normalizer();

	/** does the calculation */
	virtual void goOn();

    signals:

	/** emits a block with the filtered data */
	void output(Kwave::SampleArray data);

    public slots:

	/** receives input data */
	void input(Kwave::SampleArray data);

	/**
	 * Sets the gain of the amplifier [0...1]
	 */
	void setGain(const QVariant g);

	/**
	 * Sets the limiter level [linear, 0...1]
	 */
	void setLimiterLevel(const QVariant l);

    private:

	/** buffer for input */
	Kwave::SampleArray m_buffer;

	/** gain */
	double m_gain;

	/** limiter level */
	double m_limit;

    };
}

#endif /* _NORMALIZER_H_ */

//***************************************************************************
//***************************************************************************
