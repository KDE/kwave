/*************************************************************************
 * SampleDecoderFloat.cpp  -  decoder for IEEE float samples
 *                             -------------------
 *    begin                : Thu Sep 04 2026
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
#include <cstring>

#include <qendian.h>
#include <QtGlobal>

#include "libkwave/Sample.h"

#include "SampleDecoderFloat.h"

//***************************************************************************
Kwave::SampleDecoderFloat::SampleDecoderFloat(Kwave::byte_order_t endianness)
    :Kwave::SampleDecoder(),
     m_swap(false)
{
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    const Kwave::byte_order_t cpu_endian = Kwave::BigEndian;
#else
    const Kwave::byte_order_t cpu_endian = Kwave::LittleEndian;
#endif
    m_swap = (endianness != cpu_endian);
}

//***************************************************************************
Kwave::SampleDecoderFloat::~SampleDecoderFloat()
{
}

//***************************************************************************
void Kwave::SampleDecoderFloat::decode(QByteArray &raw_data,
                                       Kwave::SampleArray &decoded)
{
    unsigned int samples = static_cast<unsigned int>(raw_data.size() / 4);
    sample_t     *dst = decoded.data();

    Q_ASSERT(decoded.size() >= samples);
    static_assert(sizeof(float) == 4, "float must be 32-bit");

    if (m_swap)
    {
        // aligned 32-bit integer reads with hardware byte swap
        const quint32 *src = reinterpret_cast<const quint32 *>(
            raw_data.constData());
        for (unsigned int i = 0; i < samples; ++i)
        {
            quint32 raw_int = qbswap(*src++);
            float val = *reinterpret_cast<const float *>(&raw_int);
            *dst++ = float2sample(val);
        }
    }
    else
    {
        // direct aligned float read
        const float *src_float = reinterpret_cast<const float *>(
            raw_data.constData());
        for (unsigned int i = 0; i < samples; ++i)
            *dst++ = float2sample(*src_float++);
    }
}

//***************************************************************************
unsigned int Kwave::SampleDecoderFloat::rawBytesPerSample()
{
    return 4;
}

//***************************************************************************
//***************************************************************************
