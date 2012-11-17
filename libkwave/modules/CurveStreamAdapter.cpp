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

#include "libkwave/modules/CurveStreamAdapter.h"

/***************************************************************************/
Kwave::CurveStreamAdapter::CurveStreamAdapter(Kwave::Curve &curve,
                                              unsigned int length)
    :Kwave::SampleSource(),
     m_position(0), m_length(length),
     m_interpolation(curve.interpolation()),
     m_buffer(blockSize())
{
}

/***************************************************************************/
Kwave::CurveStreamAdapter::~CurveStreamAdapter()
{
}

/***************************************************************************/
void Kwave::CurveStreamAdapter::goOn()
{
    unsigned int offset;
    double x;
    double y;
    double x_max = static_cast<double>(m_length);
    const unsigned int samples = blockSize();

    // fill with interpolated points
    for (offset=0; offset < samples; ++offset) {
	x = static_cast<double>(m_position) / x_max; // x is [0.0 ... 1.0]
	y = m_interpolation.singleInterpolation(x);
	m_buffer[offset] = float2sample(y);
	m_position++;

	// wrap-around, for periodic signals
	if (m_position > m_length)
	    m_position = 0;
    }

    emit output(m_buffer);
}

//***************************************************************************
#include "CurveStreamAdapter.moc"
/***************************************************************************/
/***************************************************************************/
