/***************************************************************************
      KwaveMimeData.cpp  -  mime data container for Kwave's audio data
			     -------------------
    begin                : Oct 04 2008
    copyright            : (C) 2008 by Thomas Eschenbacher
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

#include <QBuffer>
#include <QMimeSource>

#include "libkwave/CodecManager.h"
#include "libkwave/Decoder.h"
#include "libkwave/Encoder.h"
#include "libkwave/KwaveMimeData.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SampleWriter.h"
#include "libkwave/Signal.h"
#include "libkwave/SignalManager.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/MultiTrackWriter.h"

// RFC 2361:
#define WAVE_FORMAT_PCM "audio/vnd.wave" // ; codec=001"

//***************************************************************************
Kwave::MimeData::MimeData()
    :QMimeData(), m_data()
{
}

//***************************************************************************
Kwave::MimeData::~MimeData()
{
}

//***************************************************************************
bool Kwave::MimeData::encode(QWidget *widget,
	                     MultiTrackReader &src,
	                     FileInfo &info)
{
    // use our default encoder
    Encoder *encoder = CodecManager::encoder(WAVE_FORMAT_PCM);
    Q_ASSERT(encoder);
    if (!encoder) return false;

    // create a buffer for the wav data
    m_data.resize(0);
    QBuffer dst(&m_data);

    // encode into the buffer
    encoder->encode(widget, src, dst, info);

    delete encoder;

    // set the mime data into this drag&drop container
    setData("audio/vnd.wave", m_data);
    return true;
}

//***************************************************************************
unsigned int Kwave::MimeData::decode(QWidget *widget, const QMimeSource *e,
                                     SignalManager &sig, unsigned int pos)
{
    // try to find a suitable decoder
    Decoder *decoder = CodecManager::decoder(e);
    Q_ASSERT(decoder);
    if (!decoder) return 0;

    // decode, use the first format that matches
    int i;
    const char *format;
    unsigned int decoded_length = 0;
    unsigned int decoded_tracks = 0;

    for (i=0; (format = e->format(i)); ++i) {
	if (CodecManager::canDecode(format)) {
	    QByteArray raw_data = e->encodedData(format);
	    QBuffer src(&raw_data);

	    // open the mime source and get header information
	    bool ok = decoder->open(widget, src);
	    if (!ok) continue;
	    decoded_length = decoder->info().length();
	    decoded_tracks = decoder->info().tracks();
	    Q_ASSERT(decoded_length);
	    Q_ASSERT(decoded_tracks);
	    if (!decoded_length || !decoded_tracks) continue;

	    if (!sig.tracks()) {
		// drop into an empty window -> create tracks
		qDebug("KwaveDrag::decode(...) -> new signal");
		sig.newSignal(0,
		    decoder->info().rate(),
		    decoder->info().bits(),
		    decoded_tracks);
		ok = (sig.tracks() == decoded_tracks);
		if (!ok) continue;
	    }

	    // prepare the signal
	    unsigned int left  = pos;
	    unsigned int right = left + decoded_length - 1;
	    MultiTrackWriter dst(sig, sig.selectedTracks(), Insert,
	                         left, right);

	    ok = decoder->decode(widget, dst);
	    break;
	}
    }
    delete decoder;
    return decoded_length;
}

//***************************************************************************
#include "KwaveMimeData.moc"
//***************************************************************************
//***************************************************************************
