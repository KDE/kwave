/***************************************************************************
             BandPass.h  -  simple band pass
                             -------------------
    begin                : Sun Nov 18 2007
    copyright            : (C) 2007 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de

    filter functions:
    Copyright (C) 1998 Juhana Sadeharju <kouhia@nic.funet.fi>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef BAND_PASS_H
#define BAND_PASS_H

#include "config.h"

#include <QtCore/QObject>
#include <QtCore/QVariant>

#include "libkwave/SampleArray.h"
#include "libkwave/SampleSource.h"
#include "libkwave/TransmissionFunction.h"

namespace Kwave
{

    class BandPass: public Kwave::SampleSource,
                    public Kwave::TransmissionFunction
    {
	Q_OBJECT
    public:

	/** Constructor */
	BandPass();

	/** Destructor */
	virtual ~BandPass();

	/** does the calculation */
	virtual void goOn();

	/** @see TransmissionFunction::at() */
	virtual double at(double f);

    signals:

	/** emits a block with the filtered data */
	void output(Kwave::SampleArray data);

    public slots:

	/** receives input data */
	void input(Kwave::SampleArray data);

	/**
	 * Sets the center frequency, normed to [0...2Pi]. The calculation is:
	 * fc = frequency [Hz] * 2 * Pi / f_sample [Hz].
	 * The default setting is 0.5.
	 */
	void setFrequency(const QVariant fc);

	/**
	 * Sets the bandwidth, normed to [0...2Pi]. The calculation is:
	 * bw = bandwidth [Hz] * 2 * Pi / f_sample [Hz].
	 * The default setting is 0.1.
	 */
	void setBandwidth(const QVariant bw);

    private:

	/** reset/initialize the filter coefficients */
	void initFilter();

	/**
	 * set the coefficients for a given frequency
	 * @param freq normed frequency
	 * @param R normed bandwidth
	 */
	void setfilter_2polebp(double freq, double R);

    private:

	/** buffer for input */
	Kwave::SampleArray m_buffer;

	/** center frequency */
	double m_frequency;

	/** bandwidth */
	double m_bandwidth;

	/** structure with the filter coefficients */
	struct {
	    double cx,cx1,cx2,cy1,cy2;
	    double x,x1,x2,y,y1,y2;
	} m_filter;

    };
}

#endif /* BAND_PASS_H */

//***************************************************************************
//***************************************************************************
