/***************************************************************************
          KwaveDrag.cpp  -  Drag&Drop container for Kwave's audio data
			     -------------------
    begin                : Jan 24 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
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

#include <qdatastream.h>

#include "libkwave/KwaveDrag.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SampleWriter.h"
#include "libkwave/Signal.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/WavFileFormat.h"

// RFC 2361:
#define WAVE_FORMAT_PCM "audio/vnd.wave; codec=001"

#define CHECK(cond) ASSERT(cond); if (!(cond)) return false;

//***************************************************************************
KwaveDrag::KwaveDrag(QWidget *dragSource, const char *name)
    :QDragObject(dragSource, name), data()
{
}

//***************************************************************************
KwaveDrag::~KwaveDrag()
{
}

//***************************************************************************
const char *KwaveDrag::format(int i) const
{
    // see RFC 2361 for other codecs
    switch (i) {
	case 0: return WAVE_FORMAT_PCM;
    }
    return 0;
}

//***************************************************************************
QByteArray KwaveDrag::encodedData(const char *format) const
{
    debug("KwaveDrag::encodedData(%s)", format);
    if (QCString(WAVE_FORMAT_PCM) == QCString(format)) return data;
    return QByteArray();
}

//***************************************************************************
bool KwaveDrag::canDecode(const QMimeSource* e)
{
    return (e && e->provides(WAVE_FORMAT_PCM));
}

//***************************************************************************
bool KwaveDrag::encode(unsigned int rate, unsigned int bits,
                       MultiTrackReader &src)
{
    data = QByteArray();

    /* first get and check some header information */
    const unsigned int tracks = src.count();
    CHECK(tracks);

    const unsigned int length = src[0]->last() - src[0]->first() + 1;
    wav_header_t header;
    const int bytes = bits >> 3;
    const unsigned int datalen = bytes * length * tracks;
    const unsigned int array_size = datalen + sizeof(wav_header_t) + 4 + 4;

    data.resize(array_size);
    if (data.size() != array_size) {
	debug("KwaveDrag::encode(): out of memory!");
	data.resize(0);
	return false;
    }

    /* create the wav header */

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

    memcpy(data.data(), &header, sizeof(wav_header_t));
    unsigned int dst_pos = sizeof(wav_header_t);

    data[dst_pos++] = 'd';
    data[dst_pos++] = 'a';
    data[dst_pos++] = 't';
    data[dst_pos++] = 'a';
    data[dst_pos++] =  datalen        & 0xFF;
    data[dst_pos++] = (datalen >>  8) & 0xFF;
    data[dst_pos++] = (datalen >> 16) & 0xFF;
    data[dst_pos++] = (datalen >> 24) & 0xFF;

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
		data[dst_pos++] = (char)((act - 128) & 0xFF);
	    } else {
		// >= 16 bits -> signed
		for (register int byte = bytes; byte; byte--) {
		    data[dst_pos++] = (char)(act & 0xFF);
		    act >>= 8;
		}
	    }
	}
    }
    return true;
}

//***************************************************************************
bool KwaveDrag::decode(const QMimeSource *e, Signal &sig,
                       unsigned int &rate, unsigned int &bits)
{
    if (!KwaveDrag::canDecode(e)) return false;

    // get the encoded block of data from the mime source
    QByteArray data(e->encodedData(e->format()));
    CHECK(!data.isEmpty());
    CHECK(data.size() > sizeof(wav_header_t)+8);

    wav_header_t header;
    unsigned int datalen = data.size() - (sizeof(wav_header_t) + 8);
    unsigned int src_pos = 0;

    // get the header
    memcpy(&header, data.data(), sizeof(wav_header_t));
#if defined(IS_BIG_ENDIAN)
    header.filelength = bswap_32(header.filelength);
    header.fmtlength = bswap_32(header.fmtlength);
    header.mode = bswap_16(header.mode);
    header.channels = bswap_16(header.channels);
    header.rate = bswap_32(header.rate);
    header.AvgBytesPerSec = bswap_32(header.AvgBytesPerSec);
    header.BlockAlign = bswap_16(header.BlockAlign);
    header.bitspersample = bswap_16(header.bitspersample);
#endif
    const unsigned int tracks = header.channels;
    rate = header.rate;
    bits = header.bitspersample;
    const unsigned int bytes = (bits >> 3);

    // some sanity checks
    CHECK(header.AvgBytesPerSec == rate * bytes * tracks);
    CHECK(static_cast<unsigned int>(header.BlockAlign) == bytes*tracks);
    CHECK(!strncmp((char*)&(header.riffid), "RIFF", 4));
    CHECK(!strncmp((char*)&(header.wavid), "WAVE", 4));
    CHECK(!strncmp((char*)&(header.fmtid), "fmt ", 4));
    CHECK(header.filelength == (datalen + sizeof(wav_header_t)));
    CHECK(header.fmtlength == 16);
    CHECK(header.mode == 1);

    src_pos += sizeof(wav_header_t);
    CHECK(data[src_pos+0] == 'd');
    CHECK(data[src_pos+1] == 'a');
    CHECK(data[src_pos+2] == 't');
    CHECK(data[src_pos+3] == 'a');
    CHECK(data[src_pos+4] == static_cast<char>( datalen        & 0xFF));
    CHECK(data[src_pos+5] == static_cast<char>((datalen >>  8) & 0xFF));
    CHECK(data[src_pos+6] == static_cast<char>((datalen >> 16) & 0xFF));
    CHECK(data[src_pos+7] == static_cast<char>((datalen >> 24) & 0xFF));
    src_pos += 8;

    // create a signal
    unsigned int track;
    unsigned int length = datalen / bytes / tracks;
    for (track=0; track < tracks; track++) {
	if (!sig.appendTrack(length)) {
	    // out of memory
	    debug("KwaveDrag::decode: creating signal failed");
	    sig.close();
	    return false;
	}
    }

    // open writers
    MultiTrackWriter dst;
    sig.openMultiTrackWriter(dst, sig.allTracks(), Overwrite, 0, length-1);
    sig.setBits(bits);
    sig.setRate(rate);
    if (sig.tracks() != tracks) {
	debug("KwaveDrag::decode: creating writers failed");
	sig.close();
	return false;
    }

    // fill the signal with data
    const __uint32_t sign = 1 << (24-1);
    const unsigned int negative = ~(sign - 1);
    const unsigned int shift = 24-bits;

    while (src_pos < data.size()) {
	__uint32_t s = 0; // raw 32bit value
	for (track = 0; track < tracks; track++) {
	    SampleWriter *stream = dst.at(track);
	
	    if (bytes == 1) {
		// 8-bit files are always unsigned !
		s = (static_cast<__uint8_t>(data[src_pos++]) - 128)
		    << shift;
	    } else {
		// >= 16 bits is signed
		s = 0;
		for (register unsigned int byte = 0; byte < bytes; byte++) {
		    s |= (static_cast<__uint8_t>(data[src_pos++])
		        << ((byte << 3) + shift));
		}
		// sign correcture for negative values
		if (s & sign) s |= negative;
	    }
	
	    // the following cast is only necessary if
	    // sample_t is not equal to a 32bit int
	    sample_t sample = static_cast<sample_t>(s);
	    *stream << sample;
	}
    }

    // return with a valid Signal
    return true;
}

//***************************************************************************
//***************************************************************************
