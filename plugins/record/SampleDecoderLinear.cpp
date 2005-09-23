/*************************************************************************
 SampleDecoderLinear.cpp  -  decoder for all non-compressed linear formats
                             -------------------
    begin                : Sat Nov 01 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
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

#include "SampleDecoderLinear.h"

//***************************************************************************
void decode_NULL(char *src, sample_t *dst, unsigned int count)
{
    while (count--) {
        printf("%02X ", (int)*src);
        *(dst++) = count % (1 << (SAMPLE_BITS-1));
    }
}

//***************************************************************************
// this little function is provided as inline code to avaid a compiler
// warning about negative shift value when included directly
static inline u_int32_t shl(const u_int32_t v, const int s)
{
    return (s > 0) ? (v << s) : (v >> s);
}

//***************************************************************************
/**
 * Template for decoding a buffer with linear samples. The tricky part is
 * done in the compiler which optimizes away all unused parts of current
 * variant and does nice loop optimizing!
 * @param src array with raw data
 * @param dst array that receives the samples in Kwave's format
 * @param count the number of samples to be decoded
 */
template<const unsigned int bits, const bool is_signed,
         const bool is_little_endian>
void decode_linear(char *src, sample_t *dst, unsigned int count)
{
    const int shift = (SAMPLE_BITS - bits);
    const u_int32_t sign = 1 << (SAMPLE_BITS-1);
    const u_int32_t negative = ~(sign - 1);
    const u_int32_t bytes = (bits+7) >> 3;

    while (count--) {
	// read from source buffer
	register u_int32_t s = 0;
	if (is_little_endian) {
	    // little endian
	    for (unsigned int byte=0; byte < bytes; ++byte) {
		s |= (unsigned char)(*(src++)) << (byte << 3);
	    }
	} else {
	    // big endian
	    for (int byte=bytes-1; byte >= 0; --byte) {
		s |= (unsigned char)(*(src++)) << (byte << 3);
	    }
	}

	// convert to signed
	if (!is_signed) s -= shl(1, bits-1)-1;

	// shift up to Kwave's bit count
	s = shl(s, shift);

	// sign correcture for negative values
	if (is_signed && (s & sign)) s |= negative;

	// write to destination buffer
	*(dst++) = static_cast<sample_t>(s);
    }
}

//***************************************************************************
#define MAKE_DECODER(bits)                             \
if (sample_format != SampleFormat::Unsigned) {           \
    if (endianness != BigEndian) {                     \
	m_decoder = decode_linear<bits, true, true>;   \
    } else {                                           \
	m_decoder = decode_linear<bits, true, false>;  \
    }                                                  \
} else {                                               \
    if (endianness != BigEndian) {                     \
	m_decoder = decode_linear<bits, false, true>;  \
    } else {                                           \
	m_decoder = decode_linear<bits, false, false>; \
    }                                                  \
}

//***************************************************************************
SampleDecoderLinear::SampleDecoderLinear(
    SampleFormat sample_format,
    unsigned int bits_per_sample,
    byte_order_t endianness
)
    :SampleDecoder(),
    m_bytes_per_sample((bits_per_sample + 7) >> 3),
    m_decoder(decode_NULL)
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
	    MAKE_DECODER(8);
	    break;
	case 2:
	    MAKE_DECODER(16);
	    break;
	case 3:
	    MAKE_DECODER(24);
	    break;
	case 4:
	    MAKE_DECODER(32);
	    break;
    }
}

//***************************************************************************
SampleDecoderLinear::~SampleDecoderLinear()
{
}

//***************************************************************************
void SampleDecoderLinear::decode(QByteArray &raw_data,
                                 QMemArray<sample_t> &decoded)
{
    Q_ASSERT(m_decoder);
    if (!m_decoder) return;

    unsigned int samples = raw_data.size() / m_bytes_per_sample;
    char *src = raw_data.data();
    sample_t *dst = decoded.data();

    m_decoder(src, dst, samples);
}

//***************************************************************************
unsigned int SampleDecoderLinear::rawBytesPerSample()
{
    return m_bytes_per_sample;
}

//***************************************************************************
//***************************************************************************
