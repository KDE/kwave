/*************************************************************************
 SampleEncoderULaw.cpp  -  encoder for U-Law samples
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

#include "config.h"

#include <QtGlobal>

#include "libkwave/Sample.h"
#include "libkwave/SampleEncoderULaw.h"
#include "libkwave/SampleFormat.h"
#include "libkwave/Utils.h"

namespace {
    /**
     * calculate U-Law byte from 16-bit PCM input
     * @param pcm16 signed 16 bit PCM input
     * @return encoded U-Law byte
     */
    static quint8 calc_ulaw_entry(int16_t pcm16)
    {
        int pcm = static_cast<int>(pcm16);
        int mask = 0xFF;

        if (pcm < 0) {
            pcm = (-pcm);
            mask = 0x7F;
        }

        // clip to max magnitude for U-Law
        if (pcm > 32635) {
            pcm = 32635;
        }

        // add G.711 U-Law bias
        pcm += 132;

        // determine exponent segment (0 to 7)
        int exponent = 7;
        for (int exp_mask = 0x4000;
             (((pcm & exp_mask) == 0) && (exponent > 0));
             exp_mask >>= 1) {
            exponent--;
        }

        int mantissa = ((pcm >> (exponent + 3)) & 0x0F);
        quint8 uval = static_cast<quint8>((exponent << 4) | mantissa);

        return static_cast<quint8>(uval ^ mask);
    }

    /**
     * struct holding 64 kB lookup table for U-Law
     */
    struct ULawLUT {
        quint8 table[65536];

        ULawLUT() {
            for (int i = 0; (i < 65536); ++i) {
                int16_t pcm16 = static_cast<int16_t>(i);
                table[i] = calc_ulaw_entry(pcm16);
            }
        }
    };

    /**
     * fast U-Law encoder using 64 kB lookup table
     * @param sample input sample_t (24 bit)
     * @return encoded U-Law byte
     */
    inline static quint8 sample_to_ulaw(sample_t sample)
    {
        static const ULawLUT ulaw_lut;
        int16_t pcm16 = static_cast<int16_t>(sample >> (SAMPLE_BITS - 16));
        return ulaw_lut.table[static_cast<uint16_t>(pcm16)];
    }
}

//***************************************************************************
Kwave::SampleEncoderULaw::SampleEncoderULaw()
    :SampleEncoder()
{
}

//***************************************************************************
Kwave::SampleEncoderULaw::~SampleEncoderULaw()
{
}

//***************************************************************************
void Kwave::SampleEncoderULaw::encode(const Kwave::SampleArray &samples,
                                      unsigned int count,
                                      QByteArray &raw_data)
{
    raw_data.resize(count * rawBytesPerSample());

    const sample_t *src = samples.constData();
    quint8 *dst = reinterpret_cast<quint8 *>(raw_data.data());

    // convert samples to u-law bytes
    for (unsigned int i = 0; i < count; ++i)
        *dst++ = sample_to_ulaw(*src++);
}

//***************************************************************************
unsigned int Kwave::SampleEncoderULaw::rawBytesPerSample()
{
    return 1;
}

//***************************************************************************
//***************************************************************************
