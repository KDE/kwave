/*************************************************************************
    SampleDecoderLinear.h  -  decoder for all non-compressed linear formats
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

#ifndef _SAMPLE_DECODER_LINEAR_H_
#define _SAMPLE_DECODER_LINEAR_H_

#include "config.h"
#include "libkwave/ByteOrder.h"
#include "libkwave/SampleFormat.h"
#include "SampleDecoder.h"

namespace Kwave
{
    class SampleDecoderLinear: public Kwave::SampleDecoder
    {
    public:

	/**
	 * Constructor
	 * @param sample_format index of the sample format (signed/unsigned)
	 * @param bits_per_sample number of bits per sample in the raw data
	 * @param endianness either SOURCE_LITTLE_ENDIAN or SOURCE_BIG_ENDIAN
	 */
	SampleDecoderLinear(Kwave::SampleFormat sample_format,
	                    unsigned int bits_per_sample,
	                    byte_order_t endianness);

	/** Destructor */
	virtual ~SampleDecoderLinear();

	/**
	 * Decodes the given buffer (byte array) by splitting it into
	 * it's tracks, decoding all samples and writing the result to
	 * the corresponding Writers.
	 * @param raw_data array with raw undecoded audio data
	 * @param decoded array with decoded samples
	 */
	virtual void decode(QByteArray &raw_data,
	                    Kwave::SampleArray &decoded);

	/** Returns the number of bytes per sample in raw (not encoded) form */
	virtual unsigned int rawBytesPerSample();

    private:

	/** number of bytes per raw sample */
	unsigned int m_bytes_per_sample;

	/** optimized function used for decoding the given format */
	void(*m_decoder)(const u_int8_t *, sample_t*, unsigned int);

    };
}

#endif /* _SAMPLE_DECODER_LINEAR_H_ */

//***************************************************************************
//***************************************************************************
