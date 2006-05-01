/*************************************************************************
 SampleEncoderLinear.cpp  -  encoder for all non-compressed linear formats
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

#include "config.h"

#include <stdio.h>
#include <sys/types.h>

#include "libkwave/CompressionType.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleFormat.h"

#include "SampleEncoderLinear.h"

//***************************************************************************
void encode_NULL(const sample_t *src, u_int8_t *dst, unsigned int count)
{
    (void)src;
    (void)dst;
    (void)count;
    qWarning("call to encode_NULL");
}

//***************************************************************************
/**
 * Template for encoding a buffer with linear samples. The tricky part is
 * done in the compiler which optimizes away all unused parts of current
 * variant and does nice loop optimizing!
 * @param src array with samples in Kwave's format
 * @param dst array that receives the raw data
 * @param count the number of samples to be encoded
 */
template<const unsigned int bits, const bool is_signed,
         const bool is_little_endian>
void encode_linear(const sample_t *src, u_int8_t *dst, unsigned int count)
{
    const u_int8_t sign = (is_signed) ? 128 : 0;

    while (count--) {
	// read from source buffer
	register sample_t s = *(src++);

	// convert raw values with less than 24 bits to unsigned first
	if (bits < SAMPLE_BITS)
	    s += 1 << (SAMPLE_BITS-1);

	if (is_little_endian) {
	    // little endian
	    if (bits > 32)
		*(dst++) = 0x00;
	    if (bits >= 24)
		*(dst++) = (s & 0xFF);
	    if (bits >= 16)
		*(dst++) = (s >> 8);
	    if (bits >= 8)
		*(dst++) = (s >> 16) + sign;
	} else {
	    // big endian
	    if (bits >= 8)
		*(dst++) = (s >> 16) + sign;
	    if (bits >= 16)
		*(dst++) = (s >> 8);
	    if (bits >= 24)
		*(dst++) = (s & 0xFF);
	    if (bits >= 32)
		*(dst++) = 0x00;
	}
    }
}

//***************************************************************************
#define MAKE_ENCODER(bits)                             \
if (sample_format != SampleFormat::Unsigned) {         \
    if (endianness != BigEndian) {                     \
	m_encoder = encode_linear<bits, true, true>;   \
    } else {                                           \
	m_encoder = encode_linear<bits, true, false>;  \
    }                                                  \
} else {                                               \
    if (endianness != BigEndian) {                     \
	m_encoder = encode_linear<bits, false, true>;  \
    } else {                                           \
	m_encoder = encode_linear<bits, false, false>; \
    }                                                  \
}

//***************************************************************************
SampleEncoderLinear::SampleEncoderLinear(
    SampleFormat sample_format,
    unsigned int bits_per_sample,
    byte_order_t endianness
)
    :SampleEncoder(),
    m_bytes_per_sample((bits_per_sample + 7) >> 3),
    m_encoder(encode_NULL)
{
    // sanity checks: we support only signed/unsigned and big/little endian
    Q_ASSERT((sample_format == SampleFormat::Signed) ||
             (sample_format == SampleFormat::Unsigned));
    if ((sample_format != SampleFormat::Signed) &&
        (sample_format != SampleFormat::Unsigned)) return;

    // allow unknown endianness only with 8 bits
    Q_ASSERT((endianness != UnknownEndian) || (m_bytes_per_sample == 1));
    if ((endianness == UnknownEndian) && (m_bytes_per_sample != 1)) return;

    // map cpu endianness to little or big
#if defined(ENDIANESS_BIG)
    if (endianness == CpuEndian) endianness = BigEndian;
#else
    if (endianness == CpuEndian) endianness = LittleEndian;
#endif

    switch (m_bytes_per_sample) {
	case 1:
	    MAKE_ENCODER(8);
	    break;
	case 2:
	    MAKE_ENCODER(16);
	    break;
	case 3:
	    MAKE_ENCODER(24);
	    break;
	case 4:
	    MAKE_ENCODER(32);
	    break;
    }
}

//***************************************************************************
SampleEncoderLinear::~SampleEncoderLinear()
{
}

//***************************************************************************
void SampleEncoderLinear::encode(const QMemArray<sample_t> &samples,
                                 unsigned int count,
                                 QByteArray &raw_data)
{
    Q_ASSERT(m_encoder);
    if (!m_encoder) return;

    Q_ASSERT(count * m_bytes_per_sample <= raw_data.size());
    if (count * m_bytes_per_sample > raw_data.size()) return;

    sample_t *src = samples.data();
    u_int8_t *dst = reinterpret_cast<u_int8_t *>(raw_data.data());

    m_encoder(src, dst, count);
}

//***************************************************************************
unsigned int SampleEncoderLinear::rawBytesPerSample()
{
    return m_bytes_per_sample;
}

//***************************************************************************
//***************************************************************************
