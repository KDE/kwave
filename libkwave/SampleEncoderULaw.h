/*************************************************************************
    SampleEncoderULaw.h  -  encoder for G.711 U-Law compressed samples
                             -------------------
    begin                : Sun Sep 06 2026
    copyright            : (C) 2026 by Thomas Eschenbacher
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

#ifndef SAMPLE_ENCODER_ULAW_H
#define SAMPLE_ENCODER_ULAW_H

#include "config.h"
#include "libkwave_export.h"

#include <QtGlobal>

#include "libkwave/ByteOrder.h"
#include "libkwave/SampleEncoder.h"

namespace Kwave
{
    class LIBKWAVE_EXPORT SampleEncoderULaw: public Kwave::SampleEncoder
    {
    public:

        /**
         * Constructor
         */
        SampleEncoderULaw();

        /** Destructor */
        ~SampleEncoderULaw() override;

        /**
         * Encodes a buffer with samples into a buffer with raw data.
         * @param samples array with samples
         * @param count number of samples
         * @param raw_data array with raw encoded audio data
         */
        void encode(const Kwave::SampleArray &samples,
                unsigned int count,
                QByteArray &raw_data) override;

        /** Returns the number of bytes per sample in raw (encoded) form */
        unsigned int rawBytesPerSample() override;
    };
}

#endif /* SAMPLE_ENCODER_ULAW_H */

//***************************************************************************
//***************************************************************************
