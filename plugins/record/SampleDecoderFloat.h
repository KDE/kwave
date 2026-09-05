/*************************************************************************
   SampleDecoderFloat.h  -  decoder for 32 bit IEEE float samples
                             -------------------
    begin                : Thu Sep 04 2026
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

#ifndef SAMPLE_DECODER_FLOAT_H
#define SAMPLE_DECODER_FLOAT_H

#include "config.h"
#include "libkwave/ByteOrder.h"
#include "SampleDecoder.h"

namespace Kwave
{
    class SampleDecoderFloat: public Kwave::SampleDecoder
    {
    public:

        /**
         * Constructor
         * @param endianness either SOURCE_LITTLE_ENDIAN or SOURCE_BIG_ENDIAN
         */
        SampleDecoderFloat(Kwave::byte_order_t endianness);

        /** destructor */
        ~SampleDecoderFloat() override;

        /**
         * decodes the given buffer with IEEE float samples
         * @param raw_data array with raw undecoded audio data
         * @param decoded array with decoded samples
         */
        void decode(QByteArray &raw_data,
                    Kwave::SampleArray &decoded) override;

        /** returns the number of bytes per sample in raw form */
        unsigned int rawBytesPerSample() override;

    private:
        /** true if byte swap needed */
        bool m_swap;
    };
}

#endif /* SAMPLE_DECODER_FLOAT_H */

//***************************************************************************
//***************************************************************************
