/***************************************************************************
        NotchFilter.h  - transmission function of a notch filter
                             -------------------
    begin                : Thu Jun 19 2003
    copyright            : (C) 2003 by Dave Flogeras
    email                : d.flogeras@unb.ca

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

#ifndef NOTCH_FILTER_H
#define NOTCH_FILTER_H

#include "config.h"

#include <QObject>
#include <QVariant>

#include "libkwave/SampleArray.h"
#include "libkwave/SampleSource.h"
#include "libkwave/TransmissionFunction.h"

namespace Kwave
{
    class NotchFilter: public Kwave::SampleSource,
                       public Kwave::TransmissionFunction
    {
        Q_OBJECT
    public:

        /** Constructor */
        NotchFilter();

        /** Destructor */
        virtual ~NotchFilter() Q_DECL_OVERRIDE;

        /** does the calculation */
        virtual void goOn() Q_DECL_OVERRIDE;

        /** @see TransmissionFunction::at() */
        virtual double at(double f) Q_DECL_OVERRIDE;

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
         * @param bw normed bandwidth
         */
        void setfilter_peaknotch2(double freq, double bw);

    private:

        /** buffer for input */
        Kwave::SampleArray m_buffer;

        /** cutoff frequency [0...PI] */
        double m_f_cutoff;

        /** bandwidth of the notch */
        double m_f_bw;

        /** structure with the filter coefficients */
        struct {
            double cx,cx1,cx2,cy1,cy2;
            double x,x1,x2,y,y1,y2;
        } m_filter;

    };
}

#endif /* NOTCH_FILTER_H */

//***************************************************************************
//***************************************************************************
