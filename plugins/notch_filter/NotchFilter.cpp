/***************************************************************************
        NotchFilter.cpp  - transmission function of a notch filter
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

#include "config.h"
#include <math.h>
#include <complex>

#include "NotchFilter.h"

//***************************************************************************
Kwave::NotchFilter::NotchFilter()
    :Kwave::SampleSource(0), Kwave::TransmissionFunction(),
     m_buffer(blockSize()), m_f_cutoff(M_PI), m_f_bw(M_PI / 2)
{
    initFilter();
}

//***************************************************************************
Kwave::NotchFilter::~NotchFilter()
{
}

//***************************************************************************
void Kwave::NotchFilter::goOn()
{
    emit output(m_buffer);
}

//***************************************************************************
double Kwave::NotchFilter::at(double f)
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
void Kwave::NotchFilter::initFilter()
{
    m_filter.x1 = 0.0;
    m_filter.x2 = 0.0;
    m_filter.y1 = 0.0;
    m_filter.y2 = 0.0;
    m_filter.y  = 0.0;
}

//***************************************************************************
/*
 * Some JAES's article on ladder filter.
 * freq (Hz), gdb (dB), bw (Hz)
 */
void Kwave::NotchFilter::setfilter_peaknotch2(double freq, double bw)
{
    const double gdb = -100;
    double k, w, bwr, abw, gain;

    k = pow(10.0, gdb / 20.0);
    /* w   = 2.0 * PI * freq / (double)SR; */
    /* bwr = 2.0 * PI * bw / (double)SR; */
    w = freq;
    bwr = bw;
    abw = (1.0 - tan(bwr / 2.0)) / (1.0 + tan(bwr / 2.0));
    gain = 0.5 * (1.0 + k + abw - k * abw);
    m_filter.cx = 1.0 * gain;
    m_filter.cx1 = gain * (-2.0 * cos(w) * (1.0 + abw)) /
                   (1.0 + k + abw - k * abw);
    m_filter.cx2 = gain * (abw + k * abw + 1.0 - k) /
                   (abw - k * abw + 1.0 + k);
    m_filter.cy1 = 2.0 * cos(w) / (1.0 + tan(bwr / 2.0));
    m_filter.cy2 = -abw;
}

//***************************************************************************
void Kwave::NotchFilter::input(Kwave::SampleArray data)
{
    const Kwave::SampleArray &in = data;

    if (m_buffer.size() != in.size()) m_buffer.resize(in.size());

    setfilter_peaknotch2(m_f_cutoff, m_f_bw);

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
void Kwave::NotchFilter::setFrequency(const QVariant fc)
{
    double new_freq = QVariant(fc).toDouble();
    if (new_freq == m_f_cutoff) return; // nothing to do

    m_f_cutoff = new_freq;
    initFilter();
    setfilter_peaknotch2(m_f_cutoff, m_f_bw);
}

//***************************************************************************
void Kwave::NotchFilter::setBandwidth(const QVariant bw)
{
    double new_bw = QVariant(bw).toDouble();
    if (new_bw == m_f_bw) return; // nothing to do

    m_f_bw = new_bw;
    initFilter();
    setfilter_peaknotch2(m_f_cutoff, m_f_bw);
}

//***************************************************************************
#include "NotchFilter.moc"
//***************************************************************************
//***************************************************************************
