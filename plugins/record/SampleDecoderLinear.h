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
#include "SampleDecoder.h"

class SampleDecoderLinear: public SampleDecoder
{
public:
    /**
     * Constructor
     * @param sample_format index of the sample format (signed/unsigned)
     * @param bits_per_sample number of bits per sample in the raw data
     */
    SampleDecoderLinear(int sample_format, unsigned int bits_per_sample);

    /** Destructor */
    virtual ~SampleDecoderLinear();

    /**
     * Decodes the given buffer (byte array) by splitting it into
     * it's tracks, decoding all samples and writing the result to
     * the corresponding SampleWriters.
     * @param raw_data array with raw undecoded audio data
     * @param decoded array with decoded samples
     */
    virtual void decode(QByteArray &raw_data,
                        QMemArray<sample_t> &decoded);

    /** Returns the number of bytes per sample in raw (not encoded) form */
    virtual unsigned int rawBytesPerSample();

private:

    /** number of bytes per raw sample */
    unsigned int m_bytes_per_sample;

    /** optimized function used for decoding the given format */
    void(*m_decoder)(char *, sample_t*, unsigned int);

};

#endif /* _SAMPLE_DECODER_LINEAR_H_ */
