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

#include <qbuffer.h>

#include "libkwave/Decoder.h"
#include "libkwave/Encoder.h"
#include "libkwave/KwaveDrag.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SampleWriter.h"
#include "libkwave/Signal.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/MultiTrackWriter.h"
#include "kwave/CodecManager.h"
#include "kwave/SignalManager.h"

// RFC 2361:
#define WAVE_FORMAT_PCM "audio/vnd.wave" // ; codec=001"

//***************************************************************************
KwaveDrag::KwaveDrag(QWidget *dragSource, const char *name)
    :QDragObject(dragSource, name), m_data()
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
    qDebug("KwaveDrag::encodedData(%s)", format);
    if (QCString(WAVE_FORMAT_PCM) == QCString(format)) return m_data;
    return QByteArray();
}

//***************************************************************************
bool KwaveDrag::canDecode(const QMimeSource* e)
{
    if (!e) return false;
    for (int i=0; e->format(i); ++i) {
	if (CodecManager::canDecode(e->format(i))) return true;
    }
    return false;
}

//***************************************************************************
bool KwaveDrag::encode(QWidget *widget, MultiTrackReader &src, FileInfo &info)
{
    Q_ASSERT(src.tracks());
    if (!src.tracks()) return false;
    Q_ASSERT(src[0]);
    if (!src[0]) return false;

    // use our default encoder
    Encoder *encoder = CodecManager::encoder(WAVE_FORMAT_PCM);
    Q_ASSERT(encoder);
    if (!encoder) return false;

    // create a buffer for the wav data
    m_data.resize(0);
    QBuffer dst(m_data);

    // encode into the buffer
    encoder->encode(widget, src, dst, info);

    delete encoder;
    return true;
}

//***************************************************************************
bool KwaveDrag::decode(QWidget *widget, const QMimeSource *e,
                       SignalManager &sig)
{
    // try to find a suitable decoder
    Decoder *decoder = CodecManager::decoder(e);
    Q_ASSERT(decoder);
    if (!decoder) return false;

    // decode, use the first format that matches
    int i;
    const char *format;
    bool ok = false;

    for (i=0; (format = e->format(i)); ++i) {
	if (CodecManager::canDecode(format)) {
	    QBuffer src(e->encodedData(format));

	    // open the mime source and get header information
	    ok = decoder->open(widget, src);
	    if (!ok) break;

	    // prepare the signal
	    MultiTrackWriter dst(sig, sig.allTracks(), Overwrite,
	                         0, sig.length()-1);

	    ok = decoder->decode(widget, dst);
	    break;
	}
    }
    delete decoder;
    return ok;
}

//***************************************************************************
#include "KwaveDrag.moc"
//***************************************************************************
//***************************************************************************
