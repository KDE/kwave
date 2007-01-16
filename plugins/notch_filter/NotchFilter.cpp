/***************************************************************************
        NotchFilter.cpp  - transmission function of a notch filter
			     -------------------
    begin                : Thu Jun 19 2003
    copyright            : (C) 2003 by Dave Flogeras
    email                : d.flogeras@unb.ca
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "config.h"
#include <math.h>
#include <complex>

#include "NotchFilter.h"

//***************************************************************************
NotchFilter::NotchFilter()
    :TransmissionFunction(), m_f_cutoff(M_PI)
{
    initfilter(&m_filter);
}

//***************************************************************************
NotchFilter::~NotchFilter()
{
}


//***************************************************************************
double NotchFilter::at(double f)
{
    /*
     * filter function as extracted from the aRts code:
     *
     * y[t] = cx*x[t] + cx1*x[t-1] + cx2*x[t-2]
     *                + cy1*y[t-1] + cy2*y[t-2];
     *
     * convert filter coefficients to our notation:
     */
    double a0, a1, a2, b1, b2;
    a0 = m_filter.cx;
    a1 = m_filter.cx1;
    a2 = m_filter.cx2;
    b1 = m_filter.cy1;
    b2 = m_filter.cy2;

    /*
     *        a0*z^2 + a1*z + a2
     * H(z) = ------------------   | z = e ^ (j*2*pi*f)
     *          z^2 - b1*z - b0
     */
    std::complex<double> h;
    std::complex<double> w;
    std::complex<double> j(0.0,1.0);
    std::complex<double> z;

    w = f;
    z = std::exp(j*w);

    // get h[z] at z=e^jw
    h = 0.95 * (a0 * (z*z) + (a1*z) + a2) / ((z*z) - (b1*z) - b2);

    return sqrt(std::norm(h));
}

//***************************************************************************
void NotchFilter::setFrequency(double f, double bw)
{
    m_f_cutoff = f;
    m_f_bw = bw;

    initfilter(&m_filter);
    setfilter_peaknotch2(&m_filter, m_f_cutoff, -100, m_f_bw);

    emit changed();
}

//***************************************************************************
//***************************************************************************
