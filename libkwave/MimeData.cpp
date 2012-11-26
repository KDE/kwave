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

#include <QtGui/QApplication>
#include <QtCore/QBuffer>
#include <QtCore/QMutableListIterator>
#include <QtCore/QVariant>
#include <QtGui/QWidget>

#include "libkwave/CodecManager.h"
#include "libkwave/Decoder.h"
#include "libkwave/Encoder.h"
#include "libkwave/CompressionType.h"
#include "libkwave/Connect.h"
#include "libkwave/MimeData.h"
#include "libkwave/MultiStreamWriter.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/Signal.h"
#include "libkwave/SignalManager.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/Writer.h"

#include "libkwave/modules/ChannelMixer.h"
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
	                     Kwave::MultiTrackReader &src,
	                     const Kwave::MetaDataList &meta_data)
{
    // use our default encoder
    Kwave::Encoder *encoder = Kwave::CodecManager::encoder(WAVE_FORMAT_PCM);
    Q_ASSERT(encoder);
    if (!encoder) return false;

    Q_ASSERT(src.tracks());
    if (!src.tracks()) return false;

    // set hourglass cursor
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

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
    Kwave::FileInfo info(meta_data);
    info.set(Kwave::INF_COMPRESSION, QVariant(AF_COMPRESSION_NONE));
    info.setLength(last - first + 1);
    info.setTracks(src.tracks());
    new_meta_data.replace(info);

    // encode into the buffer
    encoder->encode(widget, src, dst, new_meta_data);

    delete encoder;

    // set the mime data into this mime data container
    setData(WAVE_FORMAT_PCM, m_data);

    // remove hourglass
    QApplication::restoreOverrideCursor();

    return true;
}

//***************************************************************************
unsigned int Kwave::MimeData::decode(QWidget *widget, const QMimeData *e,
                                     Kwave::SignalManager &sig,
                                     sample_index_t pos)
{
    // decode, use the first format that matches
    unsigned int decoded_length = 0;
    unsigned int decoded_tracks = 0;

    // try to find a suitable decoder
    foreach (QString format, e->formats()) {
	// skip all non-supported formats
	if (!Kwave::CodecManager::canDecode(format)) continue;

	Kwave::Decoder *decoder = Kwave::CodecManager::decoder(format);
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

	decoded_length = Kwave::FileInfo(decoder->metaData()).length();
	decoded_tracks = Kwave::FileInfo(decoder->metaData()).tracks();
	Q_ASSERT(decoded_length);
	Q_ASSERT(decoded_tracks);
	if (!decoded_length || !decoded_tracks) {
	    delete decoder;
	    continue;
	}

	// get sample rates of source and destination
	double src_rate = Kwave::FileInfo(decoder->metaData()).rate();
	double dst_rate = sig.rate();

	// if the sample rate has to be converted, adjust the length
	// right border
	if ((src_rate != dst_rate) && (dst_rate > 1) && sig.tracks())
	    decoded_length *= (dst_rate / src_rate);

	sample_index_t left  = pos;
	sample_index_t right = left + decoded_length - 1;
	QList<unsigned int> tracks = sig.selectedTracks();
	if (tracks.isEmpty()) tracks = sig.allTracks();

	// special case: destination is currently empty
	if (!sig.tracks()) {
	    // encode into an empty window -> create tracks
	    qDebug("Kwave::MimeData::decode(...) -> new signal");
	    dst_rate = src_rate;
	    sig.newSignal(0,
		src_rate,
		Kwave::FileInfo(decoder->metaData()).bits(),
		decoded_tracks);
	    ok = (sig.tracks() == decoded_tracks);
	    if (!ok) {
		delete decoder;
		continue;
	    }
	}
	const unsigned int dst_tracks = sig.selectedTracks().count();

	// create the final sink
	Kwave::MultiTrackWriter dst(sig, sig.selectedTracks(),
	                            Kwave::Insert, left, right);

	// if the track count does not match, then we need a channel mixer
	Q_ASSERT(ok);
	Kwave::ChannelMixer *mixer = 0;
	if (ok && (decoded_tracks != dst_tracks)) {
	    qDebug("Kwave::MimeData::decode(...) -> mixing channels: %u -> %u",
	           decoded_tracks, dst_tracks);
	    mixer = new Kwave::ChannelMixer(decoded_tracks, dst_tracks);
	    Q_ASSERT(mixer);
	    ok &= (mixer) && mixer->init();
	    Q_ASSERT(ok);
	}
	Q_ASSERT(ok);

	// if the sample rates do not match, then we need a rate converter
	Kwave::StreamObject *rate_converter = 0;
	if (ok && (src_rate != dst_rate)) {
#ifdef HAVE_SAMPLERATE_SUPPORT
	    // create a sample rate converter
	    qDebug("Kwave::MimeData::decode(...) -> rate conversion: "\
	           "%0.1f -> %0.1f", src_rate, dst_rate);
	    rate_converter =
		new Kwave::MultiTrackSource<Kwave::RateConverter, true>(
		    dst_tracks, widget);
	    Q_ASSERT(rate_converter);
	    if (rate_converter)
		rate_converter->setAttribute(SLOT(setRatio(const QVariant)),
	                                     QVariant(dst_rate / src_rate));
	    else
		ok = false;
#else
	    ok = false;
	    #warning sample rate conversion is disabled
#endif
	}
	Q_ASSERT(ok);

	// set hourglass cursor
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	if (ok && ((rate_converter != 0) || (mixer != 0))) {
	    // pass all data through a filter chain
	    Kwave::MultiStreamWriter adapter(decoded_tracks);

	    // pass the data through a sample rate converter
	    // decoder -> adapter -> [mixer] -> [converter] -> dst

	    Kwave::StreamObject *last_output = &adapter;

	    if (ok && mixer) {
		// connect the channel mixer
		ok = Kwave::connect(
		    *last_output, SIGNAL(output(Kwave::SampleArray)),
		    *mixer,       SLOT(input(Kwave::SampleArray))
		);
		last_output = mixer;
	    }

	    if (ok && rate_converter) {
		// connect the rate converter
		ok = Kwave::connect(
		    *last_output,    SIGNAL(output(Kwave::SampleArray)),
		    *rate_converter, SLOT(input(Kwave::SampleArray))
		);
		last_output = rate_converter;
	    }

	    // connect the sink
	    if (ok) {
		ok = Kwave::connect(
		    *last_output, SIGNAL(output(Kwave::SampleArray)),
		    dst,          SLOT(input(Kwave::SampleArray))
		);
	    }

	    // this also starts the conversion automatically
	    if (ok)
		ok = decoder->decode(widget, adapter);

	    // flush all samples that are still in the adapter
	    adapter.flush();

	} else if (ok) {
	    // decode directly without any filter
	    ok = decoder->decode(widget, dst);
	}

	dst.flush();

	// clean up the filter chain
	if (mixer)          delete mixer;
	if (rate_converter) delete rate_converter;

	// remove hourglass
	QApplication::restoreOverrideCursor();

	// failed :-(
	Q_ASSERT(ok);
	if (!ok) {
	    delete decoder;
	    decoded_length = 0;
	    continue;
	}

	// take care of the meta data, shift all it by "left" and
	// add it to the signal
	Kwave::MetaDataList meta_data = decoder->metaData();

        // adjust meta data position in case of different sample rate
        if (src_rate != dst_rate)
	    meta_data.scalePositions(dst_rate / src_rate, tracks);

	meta_data.shiftRight(0, left, tracks);

	// remove the file info, this must not be handled here, otherwise
	// this would overwrite the file info of the destination
	meta_data.remove(meta_data.selectByType(
	    Kwave::FileInfo::metaDataType()));

	// add the remaining meta data (e.g. labels etc)
	sig.metaData().add(meta_data);

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
#include "MimeData.moc"
//***************************************************************************
//***************************************************************************
