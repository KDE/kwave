/*************************************************************************
    NoiseGenerator.h  -  simple noise generator, implemented as SampleSource
                             -------------------
    begin                : Sun Oct 07 2007
    copyright            : (C) 2007 by Thomas Eschenbacher
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

#ifndef _NOISE_GENERATOR_H_
#define _NOISE_GENERATOR_H_

#include "config.h"

#include <QtCore/QObject>
#include <QtCore/QVariant>

#include "libkwave/SampleArray.h"
#include "libkwave/SampleSource.h"

namespace Kwave
{
    class NoiseGenerator: public Kwave::SampleSource
    {
	Q_OBJECT
    public:

	/** Constructor */
	explicit NoiseGenerator(QObject *parent = 0);

	/** Destructor */
	virtual ~NoiseGenerator();

	/**
	 * produces a block of noise,
	 * @see Kwave::SampleSource::goOn()
	 */
	virtual void goOn();

    signals:

	/** emits a block with noise */
	void output(Kwave::SampleArray data);

    public slots:

	/** receives input data */
	void input(Kwave::SampleArray data);

	/**
	 * Sets the noise level as a factor [0 .. 1.0]
	 */
	void setNoiseLevel(const QVariant fc);

    private:

	/** buffer for input */
	Kwave::SampleArray m_buffer;

	/** noise level [0 .. 1.0] */
	double m_noise_level;

    };
}

#endif /* _NOISE_GENERATOR_H_ */

//***************************************************************************
//***************************************************************************
