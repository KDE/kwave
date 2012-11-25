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
#include "libkwave/SampleSource.h"

namespace Kwave
{
    class NoiseGenerator: public Kwave::SampleSource
    {
	Q_OBJECT
    public:

	/** Constructor */
	NoiseGenerator(QObject *parent = 0);

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

    private:

	/** array with noise, will be re-used */
	Kwave::SampleArray m_noise;

    };
}

#endif /* _NOISE_GENERATOR_H_ */

//***************************************************************************
//***************************************************************************
