/*************************************************************************
         WavEncoder.cpp  -  encoder for wav data
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
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
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <math.h>

#include "libkwave/FileInfo.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"

#include "WavEncoder.h"
#include "WavFileFormat.h"

/***************************************************************************/
WavEncoder::WavEncoder()
    :Encoder()
{
    LOAD_MIME_TYPES;
}

/***************************************************************************/
WavEncoder::~WavEncoder()
{
}

/***************************************************************************/
Encoder *WavEncoder::instance()
{
    return new WavEncoder();
}

/***************************************************************************/
bool WavEncoder::encode(QWidget *widget, MultiTrackReader &src,
                        QIODevice &dst, FileInfo &info)
{
    /* first get and check some header information */
    const unsigned int tracks = info.tracks();
    const unsigned int length = info.length();
    unsigned int bits         = info.bits();
    const unsigned int rate   = static_cast<unsigned int>(
	rint(info.rate()));

    // use default bit resolution if missing
    ASSERT(bits);
    if (!bits) bits = 16;

    // check for a valid source
    ASSERT(tracks);
    ASSERT(length);
    if ((!tracks) || (!length)) return false;
    ASSERT(src.count() == tracks);
    if (src.count() != tracks) return false;

    // open the output device
    if (!dst.open(IO_WriteOnly)) {
	KMessageBox::error(widget,
	    i18n("Unable to open the file for saving!"));
	return false;
    }

    const int bytes = bits >> 3;
    const unsigned int datalen = bytes * length * tracks;

    /* create the wav header */
    wav_header_t header;
    strncpy((char*)&(header.riffid), "RIFF", 4);
    strncpy((char*)&(header.wavid), "WAVE", 4);
    strncpy((char*)&(header.fmtid), "fmt ", 4);
    header.filelength = datalen + sizeof(wav_header_t);
    header.fmtlength = 16;
    header.mode = 1; // WAVE_FORMAT_PCM
    header.channels = tracks;
    header.rate = rate;
    header.AvgBytesPerSec = rate * (bits >> 3) * tracks;
    header.BlockAlign = bytes * tracks;
    header.bitspersample = bits;

#if defined(IS_BIG_ENDIAN)
    header.filelength = bswap_32(header.filelength);
    header.fmtlength = bswap_32(header.fmtlength);
    header.mode = bswap_16(header.mode);
    header.channels = bswap_16(header.channels);
    header.rate = bswap_32(header.rate);
    header.AvgBytesPerSec = bswap_32(header.AvgBytesPerSec);
    header.BlockAlign = bswap_16(header.BlockAlign);
    header.bitspersample = bswap_16(header.bitspersample);
    datalen = bswap_32(datalen);
#endif

    dst.writeBlock((char*)&header, sizeof(wav_header_t));
    dst.putch('d');
    dst.putch('a');
    dst.putch('t');
    dst.putch('a');
    dst.putch( datalen        & 0xFF);
    dst.putch((datalen >>  8) & 0xFF);
    dst.putch((datalen >> 16) & 0xFF);
    dst.putch((datalen >> 24) & 0xFF);

    // encode and write wav data
    const unsigned int shift = 24-bits;

    // loop for writing data
    for (unsigned int pos = 0; pos < length; pos++) {
	for (unsigned int track = 0; track < tracks; track++) {
	    SampleReader *stream = src[track];
	    sample_t sample;
	    (*stream) >> sample;
	
	    // the following cast is only necessary if
	    // sample_t is not equal to a 32bit int
	    __uint32_t act = static_cast<__uint32_t>(sample);
	
	    act >>= shift;
	    if (bytes == 1) {
		// 8 bits -> unsigned
		dst.putch((char)((act - 128) & 0xFF));
	    } else {
		// >= 16 bits -> signed
		for (register int byte = bytes; byte; byte--) {
		    dst.putch((char)(act & 0xFF));
		    act >>= 8;
		}
	    }
	}
	
	// abort if the user pressed "cancel"
	if (src.isCancelled()) break;
	// --> this would leave a corrupted file !!!
    }

    dst.close();
    return true;
}

/***************************************************************************/
/***************************************************************************/
