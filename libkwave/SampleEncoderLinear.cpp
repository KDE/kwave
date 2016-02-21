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

#include <QtGlobal>

#include "libkwave/Sample.h"
#include "libkwave/SampleEncoderLinear.h"
#include "libkwave/SampleFormat.h"
#include "libkwave/Utils.h"

//***************************************************************************
static void encode_NULL(const sample_t *src, quint8 *dst, unsigned int count)
{
    (void)src;
    (void)dst;
    (void)count;
//     qWarning("call to encode_NULL");
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
void encode_linear(const sample_t *src, quint8 *dst, unsigned int count)
{
    for ( ; count; --count) {
	// read from source buffer
	sample_t s = *(src++);

	// convert to unsigned if necessary
	if (!is_signed)
	    s += 1 << (SAMPLE_BITS-1);

	// shrink 18/20 bits and similar down, otherwise it does not work
	// with ALSA for some dubious reason !?
	if (bits == 20)
	    s >>= 4;
	if (bits == 18) // don't ask me why... !!!???
	    s >>= 6;

	if (is_little_endian) {
	    // little endian
	    if (bits > 24)
		*(dst++) = 0x00;
	    if (bits > 16)
		*(dst++) = static_cast<quint8>(s & 0xFF);
	    if (bits > 8)
		*(dst++) = static_cast<quint8>(s >> 8);
	    if (bits >= 8)
		*(dst++) = static_cast<quint8>(s >> 16);
	} else {
	    // big endian
	    if (bits >= 8)
		*(dst++) = static_cast<quint8>(s >> 16);
	    if (bits > 8)
		*(dst++) = static_cast<quint8>(s >> 8);
	    if (bits > 16)
		*(dst++) = static_cast<quint8>(s & 0xFF);
	    if (bits > 24)
		*(dst++) = 0x00;
	}
    }
}

//***************************************************************************
#define MAKE_ENCODER(bits)                             \
if (sample_format != Kwave::SampleFormat::Unsigned) {  \
    if (endianness != Kwave::BigEndian) {              \
	m_encoder = encode_linear<bits, true, true>;   \
    } else {                                           \
	m_encoder = encode_linear<bits, true, false>;  \
    }                                                  \
} else {                                               \
    if (endianness != Kwave::BigEndian) {              \
	m_encoder = encode_linear<bits, false, true>;  \
    } else {                                           \
	m_encoder = encode_linear<bits, false, false>; \
    }                                                  \
}

//***************************************************************************
Kwave::SampleEncoderLinear::SampleEncoderLinear(
    Kwave::SampleFormat::Format sample_format,
    unsigned int bits_per_sample,
    Kwave::byte_order_t endianness
)
    :SampleEncoder(),
     m_bytes_per_sample((bits_per_sample + 7) >> 3),
     m_encoder(encode_NULL)
{
    // sanity checks: we support only signed/unsigned and big/little endian
    Q_ASSERT((sample_format == Kwave::SampleFormat::Signed) ||
             (sample_format == Kwave::SampleFormat::Unsigned));
    if ((sample_format != Kwave::SampleFormat::Signed) &&
        (sample_format != Kwave::SampleFormat::Unsigned)) return;

    // allow unknown endianness only with 8 bits
    Q_ASSERT((endianness != Kwave::UnknownEndian) || (m_bytes_per_sample == 1));
    if ( (endianness == Kwave::UnknownEndian) &&
         (m_bytes_per_sample != 1) ) return;

    // map cpu endianness to little or big
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    if (endianness == Kwave::CpuEndian) endianness = Kwave::BigEndian;
#else
    if (endianness == Kwave::CpuEndian) endianness = Kwave::LittleEndian;
#endif

//     qDebug("SampleEncoderLinear::SampleEncoderLinear(fmt=%s, "
//            "%u bit [%u bytes], endian=%s)",
//            (sample_format == Kwave::SampleFormat::Signed) ?
//             "signed" : "unsigned",
//            bits_per_sample, m_bytes_per_sample,
//            (endianness == Kwave::BigEndian) ? "BE" : "LE");

    switch (bits_per_sample) {
	case 8:
	    MAKE_ENCODER(8);
	    break;
	case 16:
	    MAKE_ENCODER(16);
	    break;
	case 18:
	    MAKE_ENCODER(18);
	    break;
	case 20:
	    MAKE_ENCODER(20);
	    break;
	case 24:
	    MAKE_ENCODER(24);
	    break;
	case 32:
	    MAKE_ENCODER(32);
	    break;
    }

    Q_ASSERT(m_encoder != encode_NULL);
}

//***************************************************************************
Kwave::SampleEncoderLinear::~SampleEncoderLinear()
{
}

//***************************************************************************
void Kwave::SampleEncoderLinear::encode(const Kwave::SampleArray &samples,
                                        unsigned int count,
                                        QByteArray &raw_data)
{
    Q_ASSERT(m_encoder);
    if (!m_encoder) return;

    Q_ASSERT(count * m_bytes_per_sample <= Kwave::toUint(raw_data.size()));
    if (count * m_bytes_per_sample > Kwave::toUint(raw_data.size())) return;

    const sample_t *src = samples.constData();
    quint8 *dst = reinterpret_cast<quint8 *>(raw_data.data());

    m_encoder(src, dst, count);
}

//***************************************************************************
unsigned int Kwave::SampleEncoderLinear::rawBytesPerSample()
{
    return m_bytes_per_sample;
}

//***************************************************************************
//***************************************************************************
