/***************************************************************************
                Osc.cpp  -  simple sine oscillator
                             -------------------
    begin                : Tue Nov 06 2007
    copyright            : (C) 2007 by Thomas Eschenbacher
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

#include "config.h"

#include <math.h>

#include "libkwave/modules/Osc.h"

//***************************************************************************
Kwave::Osc::Osc()
    :Kwave::SampleSource(),
    m_buffer(blockSize()), m_omega_t(0.0), m_f(44.1), m_a(1.0)
{
}

//***************************************************************************
Kwave::Osc::~Osc()
{
}

//***************************************************************************
void Kwave::Osc::goOn()
{
    unsigned int samples = m_buffer.size();
    const double two_pi = 2.0 * M_PI;

    Q_ASSERT(!qFuzzyIsNull(m_f));
    if (qFuzzyIsNull(m_f)) return;

    double omega = two_pi / m_f;
    for (unsigned int sample = 0; sample < samples; sample++) {
	// calculate one sample as sin(w * t)
	m_buffer[sample] = double2sample(m_a * sin(m_omega_t));

	// next time, t++
	m_omega_t += omega;

	// limit the argument of sin() to [0 ... 4*Pi]
	while (m_omega_t > two_pi)
	    m_omega_t -= two_pi;
    }

    emit output(m_buffer);
}

//***************************************************************************
void Kwave::Osc::setFrequency(const QVariant &f)
{
    m_f = QVariant(f).toDouble();
}

//***************************************************************************
void Kwave::Osc::setPhase(const QVariant &p)
{
    m_omega_t = QVariant(p).toDouble();
}

//***************************************************************************
void Kwave::Osc::setAmplitude(const QVariant &a)
{
    m_a = QVariant(a).toDouble();
}

//***************************************************************************
//***************************************************************************
