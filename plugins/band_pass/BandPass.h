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

#ifndef _BAND_PASS_H_
#define _BAND_PASS_H_

#include "config.h"

#include <QObject>
#include <QVariant>

#include "libkwave/KwaveSampleArray.h"
#include "libkwave/KwaveSampleSource.h"
#include "libkwave/TransmissionFunction.h"

class BandPass: public Kwave::SampleSource,
                public TransmissionFunction
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
    virtual qreal at(qreal f);

signals:

    /** emits a block with the filtered data */
    void output(Kwave::SampleArray &data);

public slots:

    /** receives input data */
    void input(Kwave::SampleArray &data);

    /**
     * Sets the center frequency, normed to [0...2Pi]. The calculation is:
     * fc = frequency [Hz] * 2 * Pi / f_sample [Hz].
     * The default setting is 0.5.
     */
    void setFrequency(const QVariant &fc);

    /**
     * Sets the bandwidth, normed to [0...2Pi]. The calculation is:
     * bw = bandwidth [Hz] * 2 * Pi / f_sample [Hz].
     * The default setting is 0.1.
     */
    void setBandwidth(const QVariant &bw);

private:

    /** reset/initialize the filter coefficients */
    void initFilter();

    /**
     * set the coefficients for a given frequency
     * @param freq normed frequency
     * @param R normed bandwidth
     */
    void setfilter_2polebp(qreal freq, qreal R);

private:

    /** buffer for input */
    Kwave::SampleArray m_buffer;

    /** center frequency */
    qreal m_frequency;

    /** bandwidth */
    qreal m_bandwidth;

    /** structure with the filter coefficients */
    struct {
	qreal cx,cx1,cx2,cy1,cy2;
	qreal x,x1,x2,y,y1,y2;
    } m_filter;

};

#endif /* _BAND_PASS_H_ */
