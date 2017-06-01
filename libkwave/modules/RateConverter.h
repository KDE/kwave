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

#ifndef RATE_CONVERTER_H
#define RATE_CONVERTER_H

#include "config.h"

#include <QtGlobal>
#include <QObject>
#include <QVarLengthArray>
#include <QVariant>

#include <samplerate.h>

#include "libkwave/SampleArray.h"
#include "libkwave/SampleSource.h"

namespace Kwave
{

    class Q_DECL_EXPORT RateConverter: public Kwave::SampleSource
    {
	Q_OBJECT
    public:

	/** Constructor */
	RateConverter();

	/** Destructor */
	virtual ~RateConverter();

	/** does nothing, processing is done in input() */
	void goOn() Q_DECL_OVERRIDE;

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

#endif /* RATE_CONVERTER_H */

//***************************************************************************
//***************************************************************************
