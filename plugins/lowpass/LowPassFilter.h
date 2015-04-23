/***************************************************************************
        LowPassFilter.h  - transmission function of a low pass filter
			     -------------------
    begin                : Mar 15 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LOW_PASS_FILTER_H
#define LOW_PASS_FILTER_H

#include "config.h"

#include <QtCore/QObject>
#include <QtCore/QVariant>

#include "libkwave/SampleArray.h"
#include "libkwave/SampleSource.h"
#include "libkwave/TransmissionFunction.h"

namespace Kwave
{
    class LowPassFilter: public Kwave::SampleSource,
                         public Kwave::TransmissionFunction
    {
	Q_OBJECT
    public:

	/** Constructor */
	LowPassFilter();

	/** Destructor */
	virtual ~LowPassFilter();

	/** @see TransmissionFunction::at() */
	virtual double at(double f);

	/** does the calculation */
	virtual void goOn();

    signals:

	/** emits a block with the filtered data */
	void output(Kwave::SampleArray data);

    public slots:

	/** receives input data */
	void input(Kwave::SampleArray data);

	/**
	 * Sets the cutoff frequency, normed to [0...2Pi]. The calculation is:
	 * fc = frequency [Hz] * 2 * Pi / f_sample [Hz].
	 * The default setting is 0.5.
	 */
	void setFrequency(const QVariant fc);

    private:

	/** reset/initialize the filter coefficients */
	void initFilter();

	/** calculate filter coefficients for a given frequency */
	void normed_setfilter_shelvelowpass(double freq);

    private:

	/** buffer for input */
	Kwave::SampleArray m_buffer;

	/** cutoff frequency [0...PI] */
	double m_f_cutoff;

	/** structure with the filter coefficients */
	struct {
	    double cx,cx1,cx2,cy1,cy2;
	    double x,x1,x2,y,y1,y2;
	} m_filter;

    };
}

#endif /* LOW_PASS_FILTER_H */

//***************************************************************************
//***************************************************************************
