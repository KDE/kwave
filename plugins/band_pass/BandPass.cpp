/***************************************************************************
           BandPass.cpp  -  simple band pass
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

#include "config.h"
#include <complex>
#include <math.h>

#include "BandPass.h"

//***************************************************************************
Kwave::BandPass::BandPass()
    :Kwave::SampleSource(Q_NULLPTR), m_buffer(blockSize()),
    m_frequency(0.5), m_bandwidth(0.1)
{
    initFilter();
    setfilter_2polebp(m_frequency, m_bandwidth);
}

//***************************************************************************
Kwave::BandPass::~BandPass()
{
}

//***************************************************************************
void Kwave::BandPass::goOn()
{
    emit output(m_buffer);
}

//***************************************************************************
double Kwave::BandPass::at(double f)
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
void Kwave::BandPass::initFilter()
{
    m_filter.x1 = 0.0;
    m_filter.x2 = 0.0;
    m_filter.y1 = 0.0;
    m_filter.y2 = 0.0;
    m_filter.y  = 0.0;
}

//***************************************************************************
/*
 * As in ''An introduction to digital filter theory'' by Julius O. Smith
 * and in Moore's book; I use the normalized version in Moore's book.
 */
void Kwave::BandPass::setfilter_2polebp(double freq, double R)
{
    m_filter.cx  = 1.0 - R;
    m_filter.cx1 = 0.0;
    m_filter.cx2 = - (1.0 - R) * R;
    m_filter.cy1 = 2.0 * R * cos(freq);
    m_filter.cy2 = -R * R;
}

//***************************************************************************
void Kwave::BandPass::input(Kwave::SampleArray data)
{
    const Kwave::SampleArray &in = data;

    bool ok = m_buffer.resize(in.size());
    Q_ASSERT(ok);
    Q_UNUSED(ok)

    setfilter_2polebp(m_frequency, m_bandwidth);

    Q_ASSERT(in.size() == m_buffer.size());

    for (unsigned i = 0; i < in.size(); ++i)
    {
        // do the filtering
        m_filter.x = sample2double(in[i]);
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
        m_buffer[i] = double2sample(0.95 * m_filter.y);
    }
}

//***************************************************************************
void Kwave::BandPass::setFrequency(const QVariant fc)
{
    double new_freq = QVariant(fc).toDouble();
    if (qFuzzyCompare(new_freq, m_frequency)) return; // nothing to do

    m_frequency = new_freq;
    initFilter();
    setfilter_2polebp(m_frequency, m_bandwidth);
}

//***************************************************************************
void Kwave::BandPass::setBandwidth(const QVariant bw)
{
    double new_bw = QVariant(bw).toDouble();
    if (qFuzzyCompare(new_bw, m_bandwidth)) return; // nothing to do

    m_bandwidth = new_bw;
    initFilter();
    setfilter_2polebp(m_frequency, m_bandwidth);
}

//***************************************************************************
//***************************************************************************

#include "moc_BandPass.cpp"
