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
#include <qfile.h> // ###

#include "libkwave/FileFormat.h"
#include "libkwave/KwaveDrag.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/MultiTrackReader.h"

// RFC 2361:
#define WAVE_FORMAT_PCM "audio/vnd.wave; codec=001"

//***************************************************************************
KwaveDrag::KwaveDrag(QWidget *dragSource, const char *name)
    :QDragObject(dragSource, name), data(0)
{
}

//***************************************************************************
KwaveDrag::~KwaveDrag()
{
    data.resize(0);
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
    if (QCString(WAVE_FORMAT_PCM) == QCString(format)) return data;
    return 0;
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
    debug("KwaveDrag::encode(...)");

    /* first get and check some header information */
    const unsigned int tracks = src.count();
    if (!tracks) return false;

    const unsigned int length = src[0]->last() - src[0]->first() + 1;
    wav_header_t header;
    const int bytes = bits >> 3;
    const unsigned int datalen = bytes * length * tracks;
    const unsigned int array_size = datalen + sizeof(wav_header_t) + 4 + 4;

    debug("KwaveDrag::encode(): array size=%d",array_size);
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
    debug("KwaveDrag::encode(): writing %d samples, %d bit, %d tracks...",
	length, bits, tracks);
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

//    /* this produces a correct file !? */
//    /* but saving clipboard content to file fails !!! */
//    QFile f("/tmp/test.wav");
//    f.open(IO_WriteOnly);
//    f.writeBlock(data);
//    f.close();

    debug("KwaveDrag::encode(): done, %d bytes", dst_pos);
    return true;
}

#define CHECK(cond) ASSERT(cond); if (!(cond)) return false;

//***************************************************************************
bool KwaveDrag::decode(const QMimeSource *e, unsigned int &rate,
                              unsigned int &bits, MultiTrackReader &src)
{
    QByteArray data;

    if (!KwaveDrag::canDecode(e)) return false;
    debug("KwaveDrag::decode(...)");

    // get the encoded block of data from the mime source
    data = e->encodedData(e->format());
    if (data.isEmpty()) return false;
    if (data.size() <= sizeof(wav_header_t)) return false;

    wav_header_t header;
    unsigned int datalen = data.size() - sizeof(wav_header_t);
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
    CHECK(header.filelength == datalen + sizeof(wav_header_t));
    CHECK(header.fmtlength == 16);
    CHECK(header.mode == 1);

    src_pos += sizeof(wav_header_t);
    CHECK(data[src_pos++] == 'd');
    CHECK(data[src_pos++] == 'a');
    CHECK(data[src_pos++] == 't');
    CHECK(data[src_pos++] == 'a');
    CHECK(data[src_pos++] == static_cast<char>( datalen        & 0xFF));
    CHECK(data[src_pos++] == static_cast<char>((datalen >>  8) & 0xFF));
    CHECK(data[src_pos++] == static_cast<char>((datalen >> 16) & 0xFF));
    CHECK(data[src_pos++] == static_cast<char>((datalen >> 24) & 0xFF));

    // create decoder objects
    // @todo create decoders compatible with the SampleReader class

    // return with a valid MultiTrackReader
    return true;
}

//***************************************************************************
//***************************************************************************
