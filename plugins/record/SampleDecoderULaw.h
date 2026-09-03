/*************************************************************************
    SampleDecoderULaw.h  -  decoder for G.711 U-Law compressed samples
                             -------------------
    begin                : Thu Sep 03 2026
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

#ifndef SAMPLE_DECODER_ULAW_H
#define SAMPLE_DECODER_ULAW_H

#include "SampleDecoder.h"
#include "config.h"

namespace Kwave
{
    class SampleDecoderULaw: public Kwave::SampleDecoder
    {
    public:

        /** constructor */
        SampleDecoderULaw();

        /** destructor */
        ~SampleDecoderULaw() override;

        /**
         * decodes the given buffer with U-Law compressed samples
         * @param raw_data array with raw undecoded audio data
         * @param decoded array with decoded samples
         */
        void decode(QByteArray &raw_data,
                    Kwave::SampleArray &decoded) override;

        /** returns the number of bytes per sample in raw form */
        unsigned int rawBytesPerSample() override;

    };
}

#endif /* SAMPLE_DECODER_ULAW_H */

//***************************************************************************
//***************************************************************************
