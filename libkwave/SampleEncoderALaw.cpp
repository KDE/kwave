/*************************************************************************
 SampleEncoderALaw.cpp  -  encoder for A-Law samples
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
#include "libkwave/SampleEncoderALaw.h"
#include "libkwave/SampleFormat.h"
#include "libkwave/Utils.h"

namespace {
    /**
     * calculate A-Law byte from 16-bit PCM input
     * @param pcm16 signed 16-bit PCM input
     * @return encoded A-Law byte
     */
    static quint8 calc_alaw_entry(int16_t pcm16)
    {
        int pcm = static_cast<int>(pcm16);
        int mask = 0xD5;

        if (pcm < 0) {
            mask = 0x55;
            pcm = ((-pcm) - 1);
            if (pcm < 0)
                pcm = 0;
        }

        // convert 16-bit PCM to 13-bit magnitude
        int pcm13 = (pcm >> 3);
        if (pcm13 > 4095)
            pcm13 = 4095;

        // convert 13-bit PCM to 8-bit A-Law byte (ITU-T G.711)
        quint8 aval = 0;
        if (pcm13 < 32)
            aval = static_cast<quint8>(pcm13 >> 1);
        else if (pcm13 < 64)
            aval = static_cast<quint8>(0x10 | ((pcm13 >> 1) & 0x0F));
        else if (pcm13 < 128)
            aval = static_cast<quint8>(0x20 | ((pcm13 >> 2) & 0x0F));
        else if (pcm13 < 256)
            aval = static_cast<quint8>(0x30 | ((pcm13 >> 3) & 0x0F));
        else if (pcm13 < 512)
            aval = static_cast<quint8>(0x40 | ((pcm13 >> 4) & 0x0F));
        else if (pcm13 < 1024)
            aval = static_cast<quint8>(0x50 | ((pcm13 >> 5) & 0x0F));
        else if (pcm13 < 2048)
            aval = static_cast<quint8>(0x60 | ((pcm13 >> 6) & 0x0F));
        else
            aval = static_cast<quint8>(0x70 | ((pcm13 >> 7) & 0x0F));

        return static_cast<quint8>(aval ^ mask);
    }

    /**
     * struct holding 64 KiB lookup table for A-Law
     */
    struct ALawLUT {
        quint8 table[65536];

        ALawLUT()
        {
            for (int i = 0; (i < 65536); ++i) {
                int16_t pcm16 = static_cast<int16_t>(i);
                table[i] = calc_alaw_entry(pcm16);
            }
        }
    };

    /**
     * fast A-Law encoder using 64 KiB lookup table
     * @param sample input sample_t (24 bit)
     * @return encoded A-Law byte
     */
    inline static quint8 sample_to_alaw(sample_t sample)
    {
        static const ALawLUT alaw_lut;
        int16_t pcm16 = static_cast<int16_t>(sample >> (SAMPLE_BITS - 16));
        return alaw_lut.table[static_cast<uint16_t>(pcm16)];
    }
}

//***************************************************************************
Kwave::SampleEncoderALaw::SampleEncoderALaw()
    :SampleEncoder()
{
}

//***************************************************************************
Kwave::SampleEncoderALaw::~SampleEncoderALaw()
{
}

//***************************************************************************
void Kwave::SampleEncoderALaw::encode(const Kwave::SampleArray &samples,
                                      unsigned int count,
                                      QByteArray &raw_data)
{
    raw_data.resize(count * rawBytesPerSample());

    const sample_t *src = samples.constData();
    quint8 *dst = reinterpret_cast<quint8 *>(raw_data.data());

    // convert samples to A-Law bytes
    for (unsigned int i = 0; i < count; ++i)
        *dst++ = sample_to_alaw(*src++);
}

//***************************************************************************
unsigned int Kwave::SampleEncoderALaw::rawBytesPerSample()
{
    return 1;
}

//***************************************************************************
//***************************************************************************
