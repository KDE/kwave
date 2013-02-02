/***************************************************************************
   PitchShiftFilter.cpp  -  filter for modifying the "pitch_shift"
                             -------------------
    begin                : Wed Nov 28 2007
    copyright            : (C) 2007 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de

    based on synth_pitch_shift_impl.cc from the aRts project

    copyright (C) 2000 Jeff Tranter <tranter@pobox.com>
              (C) 1999 Stefan Westerfeld <stefan@space.twc.de>

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

#include "libkwave/Sample.h"
#include "PitchShiftFilter.h"

//***************************************************************************
Kwave::PitchShiftFilter::PitchShiftFilter()
    :Kwave::SampleSource(0), m_buffer(blockSize()),
     m_speed(1.0), m_frequency(0.5), m_dbuffer(),
     m_lfopos(0), m_b1pos(0), m_b2pos(0), m_b1inc(0), m_b2inc(0),
     m_b1reset(false), m_b2reset(false), m_dbpos(0)
{
    initFilter();
}

//***************************************************************************
Kwave::PitchShiftFilter::~PitchShiftFilter()
{
}

//***************************************************************************
void Kwave::PitchShiftFilter::goOn()
{
    emit output(m_buffer);
}

//***************************************************************************
void Kwave::PitchShiftFilter::initFilter()
{
    m_dbuffer.resize(MAXDELAY);
    for (m_dbpos = 0; m_dbpos < MAXDELAY; m_dbpos++)
	m_dbuffer[m_dbpos] = 0;

    m_dbpos = 0;
    m_lfopos = 0;

    if (m_speed <= 1.0) {
	m_b1pos = m_b2pos = 0.0;
	m_b1inc = m_b2inc = 1.0 - m_speed;
    } else {
	/* not yet sure what would be a nice initialization here? */
	m_b1pos = m_b2pos = 0.0;
	m_b1inc = m_b2inc = 0.0;
    }
}

//***************************************************************************
void Kwave::PitchShiftFilter::input(Kwave::SampleArray data)
{
    Q_ASSERT(static_cast<int>(data.size()) <= m_dbuffer.size());
    if (m_buffer.size() != data.size()) m_buffer.resize(data.size());

    float pi2 = 2*M_PI;
    float lfo, b1value, b2value;
    float lfoposinc = m_frequency;

    for (unsigned int pos = 0; pos < m_buffer.size(); pos++) {
	/*
	 * fill delay buffer with the input signal
	 */
	m_dbuffer[m_dbpos] = sample2float(data[pos]);

	m_lfopos += lfoposinc;
	m_lfopos -= floor(m_lfopos);

	if (m_lfopos < 0.25) {
	    m_b1reset = m_b2reset = false;
	}

	/*
	 * _speed < 1.0 (downpitching)
	 *
	 *  start with current sample and increase delay slowly
	 *
	 * _speed > 1.0 (uppitching)
	 *
	 *  start with a sample from long ago and slowly decrease delay
	 */
	if (!m_b1reset && m_lfopos > 0.25) {
	    if (m_speed <= 1.0) {
		m_b1pos = 0;
		m_b1inc = 1 - m_speed;
	    } else {
		m_b1inc = 1 - m_speed;
		m_b1pos = 10 + ((-m_b1inc) * (1 / lfoposinc));
		/* 10+ are not strictly necessary */
	    }
	    m_b1reset = true;
	}

	if (!m_b2reset && m_lfopos > 0.75) {
	    if (m_speed <= 1.0) {
		m_b2pos = 0;
		m_b2inc = 1 - m_speed;
	    } else{
		m_b2inc = 1 - m_speed;
		m_b2pos = 10 + ((-m_b2inc) * (1/lfoposinc));
		/* 10+ are not strictly necessary */
	    }
	    m_b2reset = true;
	}

	m_b1pos += m_b1inc;
	m_b2pos += m_b2inc;

	int position, position1;
	double error, int_pos;

	/*
	 * Interpolate value from buffer position 1
	 */
	error = modf(m_b1pos, &int_pos);

	position = m_dbpos - static_cast<int>(int_pos);
	if (position < 0)
	    position += MAXDELAY;
	position1 = position - 1;
	if (position1 < 0)
	    position1 += MAXDELAY;

	b1value = m_dbuffer[position] * (1 - error) +
	          m_dbuffer[position1] * error;

	/*
	 * Interpolate value from buffer position 2
	 */
	error = modf(m_b2pos,&int_pos);

	position = m_dbpos - static_cast<int>(int_pos);
	if (position < 0)
	    position += MAXDELAY;
	position1 = position-1;
	if ( position1 < 0)
	    position1 += MAXDELAY;

	b2value = m_dbuffer[position] * (1 - error) +
	          m_dbuffer[position1] * error;

	/*
	 * Calculate output signal from these two buffers
	 */

	lfo = (sin(pi2 * m_lfopos) + 1) / 2;

	/*             position    sin   lfo variable
	 *------------------------------------------------------------------
	 * lfo value:    0.25       1         1        => buffer 2 is used
	 *               0.75      -1         0        => buffer 1 is used
	 */

	m_buffer[pos] = float2sample(b1value * (1.0 - lfo) + b2value * lfo);

	/*
	 * increment delay buffer position
	 */
	m_dbpos++;
	if (m_dbpos == MAXDELAY)
	    m_dbpos = 0;
    }

}

//***************************************************************************
void Kwave::PitchShiftFilter::setSpeed(const QVariant speed)
{
    double new_speed = QVariant(speed).toDouble();
    if (new_speed == m_speed) return; // nothing to do

    m_speed = new_speed;
    initFilter();
}

//***************************************************************************
void Kwave::PitchShiftFilter::setFrequency(const QVariant freq)
{
    double new_freq = QVariant(freq).toDouble();
    if (new_freq == m_frequency) return; // nothing to do

    m_frequency = new_freq;
    initFilter();
}

//***************************************************************************
#include "PitchShiftFilter.moc"
//***************************************************************************
//***************************************************************************
