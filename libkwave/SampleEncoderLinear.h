/*************************************************************************
    SampleEncoderLinear.h  -  encoder for all non-compressed linear formats
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

#ifndef SAMPLE_ENCODER_LINEAR_H
#define SAMPLE_ENCODER_LINEAR_H

#include "config.h"

#include <kdemacros.h>

#include "libkwave/ByteOrder.h"
#include "libkwave/SampleEncoder.h"
#include "libkwave/SampleFormat.h"

namespace Kwave
{
    class KDE_EXPORT SampleEncoderLinear: public Kwave::SampleEncoder
    {
    public:

	/**
	 * Constructor
	 * @param sample_format index of the sample format (signed/unsigned)
	 * @param bits_per_sample number of bits per sample in the raw data
	 * @param endianness either SOURCE_LITTLE_ENDIAN or SOURCE_BIG_ENDIAN
	 */
	SampleEncoderLinear(Kwave::SampleFormat::Format sample_format,
	                    unsigned int bits_per_sample,
	                    Kwave::byte_order_t endianness);

	/** Destructor */
	virtual ~SampleEncoderLinear();

	/**
	 * Encodes a buffer with samples into a buffer with raw data.
	 * @param samples array with samples
	 * @param count number of samples
	 * @param raw_data array with raw encoded audio data
	 */
	virtual void encode(const Kwave::SampleArray &samples,
	                    unsigned int count,
	                    QByteArray &raw_data);

	/** Returns the number of bytes per sample in raw (encoded) form */
	virtual unsigned int rawBytesPerSample();

    private:

	/** number of bytes per raw sample */
	unsigned int m_bytes_per_sample;

	/** optimized function used for encoding the given format */
	void (*m_encoder)(const sample_t *, quint8 *, unsigned int);

    };
}

#endif /* SAMPLE_ENCODER_LINEAR_H */

//***************************************************************************
//***************************************************************************
