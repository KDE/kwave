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
#include <QMutableListIterator>

#include "libkwave/CodecManager.h"
#include "libkwave/Decoder.h"
#include "libkwave/Encoder.h"
#include "libkwave/KwaveMimeData.h"
#include "libkwave/LabelList.h"
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

    Q_ASSERT(src.tracks());
    if (!src.tracks()) return false;

    // create a buffer for the wav data
    m_data.resize(0);
    QBuffer dst(&m_data);

    // make a copy of the file info and move all labels
    // to start at the beginning of the selection
    unsigned int first = src.first();
    unsigned int last  = src.last();
    FileInfo new_info = info;
    LabelList &labels = new_info.labels();
    QMutableListIterator<Label> it(labels);
    while (it.hasNext()) {
	Label &label = it.next();
	unsigned int pos = label.pos();
	if ((pos < first) || (pos > last)) {
	    // out of the selected area -> remove
	    it.remove();
	} else {
	    // move label left
	    label.moveTo(pos - first);
	    qDebug("KwaveDrag::encode(...) -> new label @ %9d '%s'",
		label.pos(), label.name().toLocal8Bit().data());
	}
    }

    // fix the length information in the new file info
    new_info.setLength(last - first + 1);

    // encode into the buffer
    encoder->encode(widget, src, dst, new_info);

    delete encoder;

    // set the mime data into this drag&drop container
    setData("audio/vnd.wave", m_data);
    return true;
}

//***************************************************************************
unsigned int Kwave::MimeData::decode(QWidget *widget, const QMimeData *e,
                                     SignalManager &sig, unsigned int pos)
{
    // decode, use the first format that matches
    unsigned int decoded_length = 0;
    unsigned int decoded_tracks = 0;

    // try to find a suitable decoder
    foreach (QString format, e->formats()) {
	// skip all non-supported formats
	if (!CodecManager::canDecode(format)) continue;

	Decoder *decoder = CodecManager::decoder(format);
	Q_ASSERT(decoder);
	if (!decoder) return 0;

	QByteArray raw_data = e->data(format);
	QBuffer src(&raw_data);

	// open the mime source and get header information
	bool ok = decoder->open(widget, src);
	if (!ok) {
	    delete decoder;
	    continue;
	}

	decoded_length = decoder->info().length();
	decoded_tracks = decoder->info().tracks();
	Q_ASSERT(decoded_length);
	Q_ASSERT(decoded_tracks);
	if (!decoded_length || !decoded_tracks) {
	    delete decoder;
	    continue;
	}

	unsigned int left  = pos;
	unsigned int right = left + decoded_length - 1;

	if (!sig.tracks()) {
	    // encode into an empty window -> create tracks
	    qDebug("Kwave::MimeData::decode(...) -> new signal");
	    sig.newSignal(0,
		decoder->info().rate(),
		decoder->info().bits(),
		decoded_tracks);
	    ok = (sig.tracks() == decoded_tracks);
	    if (!ok) {
		delete decoder;
		continue;
	    }
	}

	// decode from clipboard
	MultiTrackWriter dst(sig, sig.selectedTracks(), Insert,
				left, right);
	ok = decoder->decode(widget, dst);
	dst.flush();

	// take care of the labels, shift all of them by "left" and
	// add them to the signal
	LabelList labels = decoder->info().labels();
	foreach(const Label &label, labels) {
	    sig.addLabel(label.pos() + left, label.name());
	    qDebug("Kwave::MimeData::decode(...) -> new label @ %9d '%s'",
		label.pos(), label.name().toLocal8Bit().data());
	}

	delete decoder;
	break;
    }

    qDebug("Kwave::MimeData::decode -> decoded_length=%u", decoded_length);
    return decoded_length;
}

//***************************************************************************
void Kwave::MimeData::clear()
{
    m_data.clear();
}

//***************************************************************************
#include "KwaveMimeData.moc"
//***************************************************************************
//***************************************************************************
