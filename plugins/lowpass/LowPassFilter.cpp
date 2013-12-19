/***************************************************************************
      LowPassFilter.cpp  - transmission function of a low pass filter
			     -------------------
    begin                : Mar 15 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>

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

#include "config.h"
#include <math.h>
#include <complex>

#include "LowPassFilter.h"

//***************************************************************************
Kwave::LowPassFilter::LowPassFilter()
    :Kwave::SampleSource(0), m_buffer(blockSize()),
    m_f_cutoff(M_PI)
{
    initFilter();
}

//***************************************************************************
Kwave::LowPassFilter::~LowPassFilter()
{
}

//***************************************************************************
void Kwave::LowPassFilter::goOn()
{
    emit output(m_buffer);
}

//***************************************************************************
void Kwave::LowPassFilter::initFilter()
{
    m_filter.x1 = 0.0;
    m_filter.x2 = 0.0;
    m_filter.y1 = 0.0;
    m_filter.y2 = 0.0;
    m_filter.y  = 0.0;
}

//***************************************************************************
/*
 * Presence and Shelve filters as given in
 *   James A. Moorer
 *   The manifold joys of conformal mapping:
 *   applications to digital filtering in the studio
 *   JAES, Vol. 31, No. 11, 1983 November
 */

/*#define SPN MINDOUBLE*/
#define SPN 0.00001

static void shelve(double cf, double boost,
                   double *a0, double *a1, double *a2,
                   double *b1, double *b2)
{
    double a, A, F, tmp, b0, recipb0, asq, F2, gamma2, siggam2, gam2p1;
    double gamman, gammad, ta0, ta1, ta2, tb0, tb1, tb2, aa1, ab1;

    a = tan(M_PI * (cf - 0.25));
    asq = a * a;
    A = pow(10.0, boost / 20.0);
    if ((boost < 6.0) && (boost > -6.0)) F = sqrt(A);
    else if (A > 1.0) F = A / sqrt(2.0);
    else F = A*sqrt(2.0);

    F2 = F * F;
    tmp = A * A - F2;
    if (fabs(tmp) <= SPN) gammad = 1.0;
    else gammad = pow((F2 - 1.0) / tmp, 0.25);
    gamman = sqrt(A) * gammad;

    gamma2 = gamman * gamman;
    gam2p1 = 1.0 + gamma2;
    siggam2 = 2.0 * sqrt(2.0) / 2.0 * gamman;
    ta0 = gam2p1 + siggam2;
    ta1 = -2.0 * (1.0 - gamma2);
    ta2 = gam2p1 - siggam2;

    gamma2 = gammad * gammad;
    gam2p1 = 1.0 + gamma2;
    siggam2 = 2.0 * sqrt(2.0) / 2.0 * gammad;
    tb0 = gam2p1 + siggam2;
    tb1 = -2.0 * (1.0 - gamma2);
    tb2 = gam2p1 - siggam2;

    aa1 = a * ta1;
    *a0 = ta0 + aa1 + asq * ta2;
    *a1 = 2.0 * a * (ta0 + ta2) + (1.0 + asq) * ta1;
    *a2 = asq * ta0 + aa1 + ta2;

    ab1 = a * tb1;
    b0 = tb0 + ab1 + asq * tb2;
    *b1 = 2.0 * a * (tb0 + tb2) + (1.0 + asq) * tb1;
    *b2 = asq * tb0 + ab1 + tb2;

    recipb0 = 1.0 / b0;
    *a0 *= recipb0;
    *a1 *= recipb0;
    *a2 *= recipb0;
    *b1 *= recipb0;
    *b2 *= recipb0;
}

//***************************************************************************
void Kwave::LowPassFilter::normed_setfilter_shelvelowpass(double freq)
{
    double gain;
    double boost = 80.0;

    gain = pow(10.0, boost / 20.0);
    shelve(freq / (2 * M_PI), boost,
	&m_filter.cx, &m_filter.cx1, &m_filter.cx2,
	&m_filter.cy1, &m_filter.cy2);
    m_filter.cx  /= gain;
    m_filter.cx1 /= gain;
    m_filter.cx2 /= gain;
    m_filter.cy1  = -m_filter.cy1;
    m_filter.cy2  = -m_filter.cy2;
}

//***************************************************************************
void Kwave::LowPassFilter::input(Kwave::SampleArray data)
{
    const Kwave::SampleArray &in = data;

    if (m_buffer.size() != in.size()) m_buffer.resize(in.size());

    normed_setfilter_shelvelowpass(m_f_cutoff);
    Q_ASSERT(in.size() == m_buffer.size());

    for (unsigned i = 0; i < in.size(); i++)
    {
	// do the filtering
	m_filter.x = sample2float(in[i]);
	m_filter.y =
	    m_filter.cx  * m_filter.x  +
	    m_filter.cx1 * m_filter.x1 +
	    m_filter.cx2 * m_filter.x2 +
	    m_filter.cy1 * m_filter.y1 +
	    m_filter.cy2 * m_filter.y2;
	m_filter.x2 = m_filter.x1;
	m_filter.x1 = m_filter.x;
	m_filter.y2 = m_filter.y1;
	m_filter.y1 = m_filter.y;
	m_buffer[i] = float2sample(0.95 * m_filter.y);
    }
}

//***************************************************************************
double Kwave::LowPassFilter::at(double f)
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
void Kwave::LowPassFilter::setFrequency(const QVariant fc)
{

    double new_freq = QVariant(fc).toDouble();
    if (new_freq == m_f_cutoff) return; // nothing to do

    m_f_cutoff = new_freq;
    initFilter();
    normed_setfilter_shelvelowpass(m_f_cutoff);
}

//***************************************************************************
#include "LowPassFilter.moc"
//***************************************************************************
//***************************************************************************
