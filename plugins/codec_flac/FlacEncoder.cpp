/*************************************************************************
         FlacEncoder.cpp  -  encoder for FLAC data
                             -------------------
    begin                : Tue Feb 28 2004
    copyright            : (C) 2004 by Thomas Eschenbacher
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

#include <math.h>
#include <stdlib.h>

#include <new>

#include <QApplication>
#include <QByteArray>
#include <QList>
#include <QVarLengthArray>

#include <KLocalizedString>
#include <QtGlobal>

#include <vorbis/vorbisenc.h>

#include "libkwave/FileInfo.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MetaDataList.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"

#include "FlacCodecPlugin.h"
#include "FlacEncoder.h"

/***************************************************************************/
Kwave::FlacEncoder::FlacEncoder()
    :Kwave::Encoder(), FLAC::Encoder::Stream(),
     m_vorbis_comment_map(), m_dst(Q_NULLPTR)
{
    REGISTER_MIME_TYPES
    REGISTER_COMPRESSION_TYPES
}

/***************************************************************************/
Kwave::FlacEncoder::~FlacEncoder()
{
}

/***************************************************************************/
Kwave::Encoder *Kwave::FlacEncoder::instance()
{
    return new(std::nothrow) Kwave::FlacEncoder();
}

/***************************************************************************/
QList<Kwave::FileProperty> Kwave::FlacEncoder::supportedProperties()
{
    return m_vorbis_comment_map.values();
}

/***************************************************************************/
::FLAC__StreamEncoderWriteStatus Kwave::FlacEncoder::write_callback(
        const FLAC__byte buffer[], size_t bytes,
        unsigned /* samples */, unsigned /* current_frame */)
{
    Q_ASSERT(m_dst);
    if (!m_dst) return FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR;

    qint64 written = m_dst->write(
	reinterpret_cast<const char *>(&(buffer[0])),
	static_cast<qint64>(bytes)
    );

    return (written == static_cast<qint64>(bytes)) ?
	FLAC__STREAM_ENCODER_WRITE_STATUS_OK :
	FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR;
}

/***************************************************************************/
void Kwave::FlacEncoder::metadata_callback(const ::FLAC__StreamMetadata *)
{
    /* we are not interested in the FLAC metadata */
}

/***************************************************************************/
Kwave::FlacEncoder::VorbisCommentContainer::VorbisCommentContainer()
    :m_vc(Q_NULLPTR)
{
    m_vc = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
    Q_ASSERT(m_vc);
}

/***************************************************************************/
Kwave::FlacEncoder::VorbisCommentContainer::~VorbisCommentContainer()
{
    if (m_vc) {
        // clean up all vorbis comments
    }
}

/***************************************************************************/
void Kwave::FlacEncoder::VorbisCommentContainer::add(const QString &tag,
                                                     const QString &value)
{
    Q_ASSERT(m_vc);
    if (!m_vc) return;

    QString s;
    s = tag + _("=") + value;

    // make a plain C string out of it, containing UTF-8
    QByteArray val = s.toUtf8();

    // put it into a vorbis_comment_entry structure
    FLAC__StreamMetadata_VorbisComment_Entry entry;
    entry.length = val.length(); // in bytes, not characters !
    entry.entry  = reinterpret_cast<FLAC__byte *>(val.data());

    // insert the comment into the list
    unsigned int count = m_vc->data.vorbis_comment.num_comments;
    bool ok =  FLAC__metadata_object_vorbiscomment_insert_comment(
	m_vc, count, entry, true);

    Q_ASSERT(ok);
    Q_UNUSED(ok)
}

/***************************************************************************/
FLAC__StreamMetadata *Kwave::FlacEncoder::VorbisCommentContainer::data()
{
    return m_vc;
}

/***************************************************************************/
void Kwave::FlacEncoder::encodeMetaData(const Kwave::FileInfo &info,
    QVector<FLAC__StreamMetadata *> &flac_metadata)
{
    // encode all Vorbis comments
    VorbisCommentMap::ConstIterator it;
    VorbisCommentContainer vc;
    for (it = m_vorbis_comment_map.constBegin();
         it != m_vorbis_comment_map.constEnd();
         ++it)
    {
	if (!info.contains(it.value())) continue; // not present -> skip

	QString value = info.get(it.value()).toString();
	vc.add(it.key(), value);
    }
    flac_metadata.append(vc.data());

    // todo: add cue sheet etc here...

}

/***************************************************************************/
bool Kwave::FlacEncoder::encode(QWidget *widget,
                                Kwave::MultiTrackReader &src,
                                QIODevice &dst,
                                const Kwave::MetaDataList &meta_data)
{
    bool result = true;

    qDebug("FlacEncoder::encode()");
    const Kwave::FileInfo info(meta_data);
    m_dst  = &dst;

    // get info: tracks, sample rate
    int            tracks = info.tracks();
    unsigned int   bits   = info.bits();
    sample_index_t length = info.length();

    set_compression_level(5); // @todo make the FLAC compression configurable
    set_channels(static_cast<unsigned>(tracks));
    set_bits_per_sample(static_cast<unsigned>(bits));
    set_sample_rate(static_cast<unsigned>(info.rate()));
    set_total_samples_estimate(static_cast<FLAC__uint64>(length));
    set_verify(false); // <- set to "true" for debugging

    // use mid-side stereo encoding if we have two channels
    set_do_mid_side_stereo(tracks == 2);
    set_loose_mid_side_stereo(tracks == 2);

    // encode meta data, most of them as vorbis comments
    QVector<FLAC__StreamMetadata *> flac_metadata;
    encodeMetaData(info, flac_metadata);

    // convert container to a list of pointers
    unsigned int meta_count = flac_metadata.size();
    if (meta_count) {
	// WARNING: this only stores the pointer, it does not copy!
	if (!set_metadata(flac_metadata.data(), meta_count)) {
	    qWarning("FlacEncoder: setting meta data failed !?");
	}
    }

    QVector<FLAC__int32 *> flac_buffer;
    do {
	// open the output device
	if (!dst.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
	    Kwave::MessageBox::error(widget,
		i18n("Unable to open the file for saving."));
	    result = false;
	    break;
	}

	// initialize the FLAC stream, this already writes some meta info
        FLAC__StreamEncoderInitStatus init_state = init();
        if (init_state != FLAC__STREAM_ENCODER_INIT_STATUS_OK) {
            qWarning("state = %d", static_cast<int>(init_state));
            Kwave::MessageBox::error(widget,
                i18n("Unable to open the FLAC encoder."));
            result = false;
            break;
        }

	// allocate output buffers, with FLAC 32 bit format
	unsigned int len = src.blockSize(); // samples
	for (int track=0; track < tracks; track++)
	{
	    FLAC__int32 *buffer =
		static_cast<FLAC__int32 *>(malloc(sizeof(FLAC__int32) * len));
	    Q_ASSERT(buffer);
	    if (!buffer) break;
	    flac_buffer.append(buffer);
	}

	// allocate input buffer, with Kwave's sample_t
	Kwave::SampleArray in_buffer(len);
	Q_ASSERT(in_buffer.size() == len);
	Q_ASSERT(flac_buffer.size() == tracks);

	if ((in_buffer.size() < len) || (flac_buffer.size() < tracks))
	{
	    Kwave::MessageBox::error(widget, i18n("Out of memory"));
	    result = false;
	    break;
	}

	// calculate divisor for reaching the proper resolution
	int shift = SAMPLE_BITS - bits;
	if (shift < 0) shift = 0;
	FLAC__int32 div = (1 << shift);
	if (div == 1) div = 0;
	const FLAC__int32 clip_min = -(1 << bits);
	const FLAC__int32 clip_max =  (1 << bits) - 1;

	sample_index_t rest = length;
	while (rest && len && !src.isCanceled() && result) {
	    // limit to rest of signal
	    if (len > rest) len = Kwave::toUint(rest);
	    if (!in_buffer.resize(len)) {
		Kwave::MessageBox::error(widget, i18n("Out of memory"));
		result = false;
		break;
	    }

	    // add all samples to one single buffer
	    for (int track = 0; track < tracks; track++) {
		Kwave::SampleReader *reader = src[track];
		Q_ASSERT(reader);
		if (!reader) break;

		(*reader) >> in_buffer;           // read samples into in_buffer
		unsigned int l = in_buffer.size();// in_buffer might be empty!
		if (l < len) {
		    if (!in_buffer.resize(len)) {
			Kwave::MessageBox::error(widget, i18n("Out of memory"));
			result = false;
			break;
		    }
		    while (l < len) in_buffer[l++] = 0;
		}

		FLAC__int32 *buf = flac_buffer.at(track);
		Q_ASSERT(buf);
		if (!buf) break;

		const Kwave::SampleArray &in = in_buffer;
		for (unsigned int in_pos = 0; in_pos < len; in_pos++) {
		    FLAC__int32 s = in[in_pos];
		    if (div) s /= div;
		    if (s > clip_max) s = clip_max;
		    if (s < clip_min) s = clip_min;
		    *buf = s;
		    buf++;
		}
	    }
	    if (!result) break; // error occurred?

	    // process all collected samples
	    FLAC__int32 **buffer = flac_buffer.data();
	    bool processed = process(buffer, static_cast<unsigned>(len));
	    if (!processed) {
		result = false;
		break;
	    }

	    rest -= len;
	}

    } while (false);

    // close the output device and the FLAC stream
    finish();

    // clean up all FLAC metadata
    while (!flac_metadata.isEmpty()) {
	FLAC__StreamMetadata *m = flac_metadata[0];
	if (m) FLAC__metadata_object_delete(m);
	flac_metadata.remove(0);
    }

    m_dst = Q_NULLPTR;
    dst.close();

    while (!flac_buffer.isEmpty()) {
	FLAC__int32 *buf = flac_buffer.first();
	if (buf) free(buf);
	flac_buffer.remove(0);
    }

    return result;
}

/***************************************************************************/
/***************************************************************************/
