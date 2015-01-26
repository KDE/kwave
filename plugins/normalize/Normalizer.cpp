/***************************************************************************
         Normalizer.cpp  -  simple normalizer with limiter
                             -------------------
    begin                : Sat May 09 2009
    copyright            : (C) 2009 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de

    limiter function     : (C) 1999-2005 Chris Vaill <chrisvaill at gmail>
                           taken from "normalize-0.7.7"
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <config.h>

#include <math.h>

#include "libkwave/Sample.h"

#include "Normalizer.h"

//***************************************************************************
Kwave::Normalizer::Normalizer()
    :Kwave::SampleSource(0), m_gain(1.0), m_limit(0.5)
{
}

//***************************************************************************
Kwave::Normalizer::~Normalizer()
{
}

//***************************************************************************
void Kwave::Normalizer::goOn()
{
}

//***************************************************************************
/*
 * Limiter function:
 *
 *        / tanh((x + lev) / (1-lev)) * (1-lev) - lev        (for x < -lev)
 *        |
 *   x' = | x                                                (for |x| <= lev)
 *        |
 *        \ tanh((x - lev) / (1-lev)) * (1-lev) + lev        (for x > lev)
 *
 * With limiter level = 0, this is equivalent to a tanh() function;
 * with limiter level = 1, this is equivalent to clipping.
 */
static inline double limiter(const double x, const double lmtr_lvl)
{
    double xp;

    if (x < -lmtr_lvl)
	xp = tanh((x + lmtr_lvl) / (1-lmtr_lvl)) * (1-lmtr_lvl) - lmtr_lvl;
    else if (x <= lmtr_lvl)
	xp = x;
    else
	xp = tanh((x - lmtr_lvl) / (1-lmtr_lvl)) * (1-lmtr_lvl) + lmtr_lvl;

    return xp;
}

//***************************************************************************
void Kwave::Normalizer::input(Kwave::SampleArray data)
{
    const unsigned int len = data.size();
    const bool use_limiter = (m_gain > 1.0);

    for (unsigned int i = 0; i < len; i++) {
	double s = sample2double(data[i]);
	s *= m_gain;
	if (use_limiter) s = limiter(s, m_limit);
	data[i] = double2sample(s);
    }

    emit output(data);
}

//***************************************************************************
void Kwave::Normalizer::setGain(const QVariant g)
{
    m_gain = QVariant(g).toDouble();
}

//***************************************************************************
void Kwave::Normalizer::setLimiterLevel(const QVariant l)
{
    m_limit = QVariant(l).toDouble();
}

//***************************************************************************
#include "Normalizer.moc"
//***************************************************************************
//***************************************************************************
