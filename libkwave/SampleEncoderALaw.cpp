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
     * convert a 24-bit Kwave sample_t to an 8-bit G.711 A-law byte
     * @param sample input sample_t (24-bit)
     * @return encoded A-law byte
     */
    inline static quint8 sample_to_alaw(sample_t sample) {
        // scale 24-bit sample_t to 13-bit linear pcm
        int pcm = static_cast<int>(sample >> (SAMPLE_BITS - 13));
        int mask = 0;

        // ITU-T G.711 A-Law sign bit logic and even bits inversion (0x55)
        if (pcm >= 0) {
            mask = 0xD5;
        } else {
            mask = 0x55;
            pcm = ((-pcm) - 1);
            if (pcm < 0) {
                pcm = 0;
            }
        }

        // clip pcm to 13-bit maximum
        if (pcm > 4095) {
            pcm = 4095;
        }

        // convert 13-bit pcm to 8-bit a-law byte
        quint8 aval = 0;
        if (pcm < 32) {
            aval = static_cast<quint8>(pcm >> 1);
        } else if (pcm < 64) {
            aval = static_cast<quint8>(0x20 | ((pcm >> 1) & 0x0F));
        } else if (pcm < 128) {
            aval = static_cast<quint8>(0x20 | ((pcm >> 2) & 0x0F));
        } else if (pcm < 256) {
            aval = static_cast<quint8>(0x30 | ((pcm >> 3) & 0x0F));
        } else if (pcm < 512) {
            aval = static_cast<quint8>(0x40 | ((pcm >> 4) & 0x0F));
        } else if (pcm < 1024) {
            aval = static_cast<quint8>(0x50 | ((pcm >> 5) & 0x0F));
        } else if (pcm < 2048) {
            aval = static_cast<quint8>(0x60 | ((pcm >> 6) & 0x0F));
        } else {
            aval = static_cast<quint8>(0x70 | ((pcm >> 7) & 0x0F));
        }

        return static_cast<quint8>(aval ^ mask);
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

    // convert samples to a-law bytes
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
