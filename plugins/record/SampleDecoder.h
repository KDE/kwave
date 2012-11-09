/*************************************************************************
        SampleDecoder.h  -  decoder for converting raw data to samples
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

#ifndef _SAMPLE_DECODER_H_
#define _SAMPLE_DECODER_H_

#include "config.h"

#include <QByteArray>

#include "libkwave/Sample.h"
#include "libkwave/SampleArray.h"

class SampleDecoder
{
public:
    /** Constructor */
    SampleDecoder() {};

    /** Destructor */
    virtual ~SampleDecoder() {};

    /**
     * Decodes the given buffer (byte array) by splitting it into
     * it's tracks, decoding all samples and writing the result to
     * the corresponding Writers.
     * @param raw_data array with raw undecoded audio data
     * @param decoded array with decoded samples
     */
    virtual void decode(QByteArray &raw_data,
                        Kwave::SampleArray &decoded) = 0;

    /** Returns the number of bytes per sample in raw (not encoded) form */
    virtual unsigned int rawBytesPerSample() = 0;

};

#endif /* _SAMPLE_DECODER_H_ */
