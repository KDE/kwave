/***************************************************************************
 CurveStreamAdapter_impl.h  -  converter from Curve to aRts sample stream
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

#ifndef _CURVE_STREAM_ADAPTER_H_
#define _CURVE_STREAM_ADAPTER_H_

#include "config.h"
#include <arts/artsflow.h>
#include <arts/stdsynthmodule.h>

#include "libkwave/Curve.h"
#include "libkwave/CurveStreamAdapter.h"

class CurveStreamAdapter_impl
    :virtual public CurveStreamAdapter_skel,
     virtual public Arts::StdSynthModule
{
public:

    /**
     * Constructor.
     * @param curve the curve from which we take the interpolation
     * @param length number of samples of the interpolated range
     */
    CurveStreamAdapter_impl(Curve &curve, unsigned int length);

    /** @see aRts documentation */
    void calculateBlock(unsigned long samples);

    /** @see aRts documentation */
    virtual void streamInit();

protected:

    /** position within the interpolation */
    unsigned int m_position;

    /** number of samples of the interpolated area */
    unsigned int m_length;

    /** the interpolation */
    Interpolation &m_interpolation;

};

#endif /* _CURVE_STREAM_ADAPTER_H_ */

