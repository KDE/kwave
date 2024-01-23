/*************************************************************************
        SampleEncoder.h  -  encoder for converting samples to raw data
                             -------------------
    begin                : Tue Apr 18 2006
    copyright            : (C) 2006 by Thomas Eschenbacher
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

#ifndef SAMPLE_ENCODER_H
#define SAMPLE_ENCODER_H

#include "config.h"

#include <QtGlobal>
#include <QByteArray>

#include "libkwave/Sample.h"
#include "libkwave/SampleArray.h"

namespace Kwave
{
    class Q_DECL_EXPORT SampleEncoder
    {
    public:
        /** Constructor */
        SampleEncoder() {}

        /** Destructor */
        virtual ~SampleEncoder() {}

        /**
         * Encodes a buffer with samples into a buffer with raw data.
         * @param samples array with samples
         * @param count number of samples
         * @param raw_data array with raw encoded audio data
         */
        virtual void encode(const Kwave::SampleArray &samples,
                            unsigned int count,
                            QByteArray &raw_data) = 0;

        /** Returns the number of bytes per sample in raw (not encoded) form */
        virtual unsigned int rawBytesPerSample() = 0;

    };
}

#endif /* SAMPLE_ENCODER_H */

//***************************************************************************
//***************************************************************************
