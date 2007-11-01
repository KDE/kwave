/***************************************************************************
 CurveStreamAdapter_impl.cpp  -  converter from Curve to aRts sample stream
                             -------------------
    begin                : Wed Dec 12 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#include "libkwave/CurveStreamAdapter_impl.h"

/***************************************************************************/
CurveStreamAdapter_impl::CurveStreamAdapter_impl(Curve &curve,
    unsigned int length)
    :m_position(0), m_length(length),
     m_interpolation(curve.interpolation())
{
}

/***************************************************************************/
void CurveStreamAdapter_impl::calculateBlock(unsigned long samples)
{
    unsigned int offset;
    double x;
    double y;
    double x_max = (double)m_length;

    // fill with interpolated points
    for (offset=0; offset < samples; ++offset) {
	x = (double)m_position / x_max;    // x range is [0.0 ... 1.0]
	y = m_interpolation.singleInterpolation(x);
	output[offset] = y;
	m_position++;

	// wrap-around, for periodic signals
	if (m_position > m_length)
	    m_position = 0;
    }
}

/***************************************************************************/
void CurveStreamAdapter_impl::streamInit()
{
    m_position = 0;
}

/***************************************************************************/
/***************************************************************************/
