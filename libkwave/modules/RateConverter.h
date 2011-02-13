/***************************************************************************
        RateConverter.h  -  single channel sample rate converter
                             -------------------
    begin                : Sat Jul 11 2009
    copyright            : (C) 2009 by Thomas Eschenbacher
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

#ifndef _RATE_CONVERTER_H_
#define _RATE_CONVERTER_H_

#include "config.h"
#ifdef HAVE_SAMPLERATE_SUPPORT

#include <QObject>
#include <QVariant>
#include <QVarLengthArray>

#include <kdemacros.h>

#include <samplerate.h>

#include "libkwave/KwaveSampleArray.h"
#include "libkwave/KwaveSampleSource.h"

namespace Kwave {
    class KDE_EXPORT RateConverter: public Kwave::SampleSource
    {
	Q_OBJECT
    public:

	/** Constructor */
	RateConverter();

	/** Destructor */
	virtual ~RateConverter();

	/** does nothing, processing is done in input() */
	virtual void goOn();

    signals:

	/** emits a block with the filtered data */
	void output(Kwave::SampleArray data);

    public slots:

	/** receives input data and also directly does the calculation */
	void input(Kwave::SampleArray data);

	/**
	 * Sets the conversion ratio, ((new rate) / (old rate))
	 */
	void setRatio(const QVariant r);

    private:

	/** conversion ratio, ((new rate) / (old rate)) */
	double m_ratio;

	/** sample rate converter context for libsamplerate */
	SRC_STATE *m_converter;

	/** input values for the sample rate converter */
	QVarLengthArray<float, 65536> m_converter_in;

	/** output values for the sample rate converter */
	QVarLengthArray<float, 65536> m_converter_out;

    };
}

#endif /* HAVE_SAMPLERATE_SUPPORT */

#endif /* _RATE_CONVERTER_H_ */
