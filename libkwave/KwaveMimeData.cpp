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
#include <QVariant>
#include <QWidget>

#include "libkwave/CodecManager.h"
#include "libkwave/CompressionType.h"
#include "libkwave/Decoder.h"
#include "libkwave/Encoder.h"
#include "libkwave/KwaveConnect.h"
#include "libkwave/KwaveMimeData.h"
#include "libkwave/LabelList.h"
#include "libkwave/MultiStreamWriter.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/Writer.h"
#include "libkwave/Signal.h"
#include "libkwave/SignalManager.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/modules/RateConverter.h"

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
	                     const Kwave::MetaDataList &meta_data)
{
    // use our default encoder
    Encoder *encoder = CodecManager::encoder(WAVE_FORMAT_PCM);
    Q_ASSERT(encoder);
    if (!encoder) return false;

    Q_ASSERT(src.tracks());
    if (!src.tracks()) return false;

    sample_index_t first = src.first();
    sample_index_t last  = src.last();
    Kwave::MetaDataList new_meta_data = meta_data.selectByRange(first, last);

    // create a buffer for the wav data
    m_data.resize(0);
    QBuffer dst(&m_data);

    // move all meta data left, to start at the beginning of the selection
    new_meta_data.shiftLeft(first, first, QList<unsigned int>());

    // fix the length information in the new file info
    // and change to uncompressed mode
    FileInfo info = meta_data.fileInfo();
    info.set(INF_COMPRESSION, QVariant(AF_COMPRESSION_NONE));
    info.setLength(last - first + 1);
    info.setTracks(src.tracks());
    new_meta_data.setFileInfo(info);

    // encode into the buffer
    encoder->encode(widget, src, dst, new_meta_data);

    delete encoder;

    // set the mime data into this mime data container
    setData(WAVE_FORMAT_PCM, m_data);
    return true;
}

//***************************************************************************
unsigned int Kwave::MimeData::decode(QWidget *widget, const QMimeData *e,
                                     SignalManager &sig, sample_index_t pos)
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

	decoded_length = decoder->metaData().fileInfo().length();
	decoded_tracks = decoder->metaData().fileInfo().tracks();
	Q_ASSERT(decoded_length);
	Q_ASSERT(decoded_tracks);
	if (!decoded_length || !decoded_tracks) {
	    delete decoder;
	    continue;
	}

	// get sample rates of source and destination
	double src_rate = decoder->metaData().fileInfo().rate();
	double dst_rate = sig.rate();

	// if the sample rate has to be converted, adjust the length
	// right border
	if (src_rate != dst_rate) decoded_length *= (dst_rate / src_rate);

	sample_index_t left  = pos;
	sample_index_t right = left + decoded_length - 1;
	QList<unsigned int> tracks = sig.selectedTracks();
	if (tracks.isEmpty()) tracks = sig.allTracks();

	if (!sig.tracks()) {
	    // encode into an empty window -> create tracks
	    qDebug("Kwave::MimeData::decode(...) -> new signal");
	    src_rate = dst_rate;
	    sig.newSignal(0,
		src_rate,
		decoder->metaData().fileInfo().bits(),
		decoded_tracks);
	    ok = (sig.tracks() == decoded_tracks);
	    if (!ok) {
		delete decoder;
		continue;
	    }
	}

	// decode from the mime data
	MultiTrackWriter dst(sig, sig.selectedTracks(), Insert, left, right);

	if (src_rate != dst_rate) {
#ifdef HAVE_SAMPLERATE_SUPPORT
	    // pass the data through a sample rate converter
	    // decoder -> adapter -> converter -> dst

	    Kwave::MultiStreamWriter adapter(tracks.count());

	    // create a sample rate converter
	    Kwave::MultiTrackSource<Kwave::RateConverter, true>
		converter(tracks.count(), widget);
	    converter.setAttribute(SLOT(setRatio(const QVariant)),
	                           QVariant(dst_rate / src_rate));

	    Kwave::connect(adapter,   SIGNAL(output(Kwave::SampleArray)),
	                   converter, SLOT(input(Kwave::SampleArray)));
	    Kwave::connect(converter, SIGNAL(output(Kwave::SampleArray)),
	                   dst,       SLOT(input(Kwave::SampleArray)));

	    // this also starts the conversion automatically
	    ok = decoder->decode(widget, adapter);

	    // flush all samples that are still in the adapter
	    adapter.flush();
#else
	    ok = false;
	    #warning sample rate conversion is disabled
#endif
	} else {
	    // decode without sample rate conversion
	    ok = decoder->decode(widget, dst);
	}
	dst.flush();

	// failed :-(
	Q_ASSERT(ok);
	if (!ok) {
	    delete decoder;
	    decoded_length = 0;
	    continue;
	}

// 	// take care of the labels, shift all of them by "left" and
// 	// add them to the signal
// 	LabelList labels = decoder->info().labels();
// 	foreach (const Label &label, labels) {
// 	    sample_index_t pos = label.pos();
//
// 	    // adjust label position in case of different sample rate
// 	    if (src_rate != dst_rate) pos *= (dst_rate / src_rate);
//
// 	    sig.addLabel(pos + left, label.name());
// // 	    qDebug("Kwave::MimeData::decode(...) -> new label @ %9d '%s'",
// // 		label.pos(), label.name().toLocal8Bit().data());
// 	}

	delete decoder;
	break;
    }

//     qDebug("Kwave::MimeData::decode -> decoded_length=%u", decoded_length);
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
