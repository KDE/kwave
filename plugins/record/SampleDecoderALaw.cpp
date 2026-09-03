/*************************************************************************
 *  SampleDecoderALaw.cpp  -  decoder for G.711 A-Law compressed samples
 *                             -------------------
 *    begin                : Thu Sep 03 2026
 *    copyright            : (C) 2026 by Thomas Eschenbacher
 *    email                : Thomas.Eschenbacher@gmx.de
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

#include <array>

#include <QtGlobal>

#include "libkwave/Sample.h"

#include "SampleDecoderALaw.h"

//***************************************************************************
// generate G.711 A-Law lookup table at compile time according to ITU-T G.711
static constexpr std::array<int16_t, 256> createALawTable()
{
    std::array<int16_t, 256> table{};
    for (int i = 0; i < 256; ++i) {
        const uint8_t a = static_cast<uint8_t>(i ^ 0x55);
        const int sign     = (a & 0x80) ? 1 : -1;
        const int exponent = (a >> 4) & 0x07;
        const int mantissa = a & 0x0F;
        const int sample   = (exponent == 0) ?
            ((mantissa << 4) + 8) :
            (((mantissa << 4) + 0x108) << (exponent - 1));
        table[i] = static_cast<int16_t>(sign * sample);
    }
    return table;
}

static constexpr std::array<int16_t, 256> _alaw_table = createALawTable();

//***************************************************************************
Kwave::SampleDecoderALaw::SampleDecoderALaw()
    :Kwave::SampleDecoder()
{
}

//***************************************************************************
Kwave::SampleDecoderALaw::~SampleDecoderALaw()
{
}

//***************************************************************************
void Kwave::SampleDecoderALaw::decode(QByteArray &raw_data,
                                      Kwave::SampleArray &decoded)
{
    unsigned int samples = static_cast<unsigned int>(raw_data.size());
    const quint8 *src = reinterpret_cast<const quint8 *>(raw_data.constData());
    sample_t     *dst = decoded.data();

    const int shift = (SAMPLE_BITS - 16);
    while (samples--) {
        const sample_t s = static_cast<sample_t>(_alaw_table[*(src++)]);
        *(dst++) = static_cast<sample_t>(s << shift);
    }
}

//***************************************************************************
unsigned int Kwave::SampleDecoderALaw::rawBytesPerSample()
{
    return 1;
}

//***************************************************************************
//***************************************************************************
