/*************************************************************************
 SampleEncoderFloat.cpp  -  encoder for 32 bit IEEE float samples
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

#include <qendian.h>
#include <QtGlobal>

#include "libkwave/Sample.h"
#include "libkwave/SampleEncoderFloat.h"
#include "libkwave/SampleFormat.h"
#include "libkwave/Utils.h"

//***************************************************************************
Kwave::SampleEncoderFloat::SampleEncoderFloat(Kwave::byte_order_t endianness)
    :SampleEncoder(),
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
Kwave::SampleEncoderFloat::~SampleEncoderFloat()
{
}

//***************************************************************************
void Kwave::SampleEncoderFloat::encode(const Kwave::SampleArray &samples,
                                       unsigned int count,
                                       QByteArray &raw_data)
{
    raw_data.resize(count * rawBytesPerSample());

    const sample_t *src = samples.constData();
    Q_CHECK_PTR(src);
    static_assert(sizeof(float) == 4, "float must be 32-bit");

    if (m_swap)
    {
        // aligned 32-bit integer writes with hardware byte swap
        quint32 *dst_int = reinterpret_cast<quint32 *>(raw_data.data());
        Q_CHECK_PTR(dst_int);
        for (unsigned int i = 0; i < count; ++i)
        {
            float val = sample2float(*src++);
            quint32 raw_int = *reinterpret_cast<const quint32 *>(&val);
            *dst_int++ = qbswap(raw_int);
        }
    }
    else
    {
        // direct aligned float write
        float *dst_float = reinterpret_cast<float *>(raw_data.data());
        for (unsigned int i = 0; i < count; ++i)
            *dst_float++ = sample2float(*src++);
    }
}

//***************************************************************************
unsigned int Kwave::SampleEncoderFloat::rawBytesPerSample()
{
    return 4;
}

//***************************************************************************
//***************************************************************************
