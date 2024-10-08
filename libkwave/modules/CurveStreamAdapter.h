/***************************************************************************
   CurveStreamAdapter.h  -  converter from Curve to a Kwave::SampleSource
                             -------------------
    begin                : Thu Nov 01 2007
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

#ifndef CURVE_STREAM_ADAPTER_H
#define CURVE_STREAM_ADAPTER_H

#include "config.h"
#include "libkwave_export.h"

#include <QtGlobal>

#include "libkwave/Curve.h"
#include "libkwave/SampleSource.h"

namespace Kwave
{

    class LIBKWAVE_EXPORT CurveStreamAdapter: public Kwave::SampleSource
    {
        Q_OBJECT
    public:
        /**
        * Constructor.
        * @param curve the curve from which we take the interpolation
        * @param length number of samples of the interpolated range
        */
        CurveStreamAdapter(Kwave::Curve &curve, sample_index_t length);

        /** Destructor */
        ~CurveStreamAdapter() override;

        /** @see Kwave::KwaveSampleSource */
        void goOn() override;


    signals:

        /** emits a block with the interpolated curve */
        void output(Kwave::SampleArray data);

    private:

        /** position within the interpolation */
        sample_index_t m_position;

        /** number of samples of the interpolated area */
        sample_index_t m_length;

        /** the interpolation */
        Kwave::Interpolation &m_interpolation;

        /** array with the interpolated curve data */
        Kwave::SampleArray m_buffer;

    };

}
#endif /* CURVE_STREAM_ADAPTER_H */

//***************************************************************************
//***************************************************************************
