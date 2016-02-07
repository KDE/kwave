/*************************************************************************
        FlacDecoder.cpp  -  decoder for FLAC data
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

#include <QDateTime>

#include <KLocalizedString>
#include <QMimeType>

#include "libkwave/Compression.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MultiWriter.h"
#include "libkwave/Sample.h"
#include "libkwave/String.h"
#include "libkwave/Writer.h"

#include "FlacCodecPlugin.h"
#include "FlacDecoder.h"

//***************************************************************************
Kwave::FlacDecoder::FlacDecoder()
    :Kwave::Decoder(), FLAC::Decoder::Stream(), m_source(0), m_dest(0),
     m_vorbis_comment_map()
{
    REGISTER_MIME_TYPES;
    REGISTER_COMPRESSION_TYPES;
}

//***************************************************************************
Kwave::FlacDecoder::~FlacDecoder()
{
    if (m_source) close();
}

//***************************************************************************
Kwave::Decoder *Kwave::FlacDecoder::instance()
{
    return new Kwave::FlacDecoder();
}

//***************************************************************************
::FLAC__StreamDecoderReadStatus Kwave::FlacDecoder::read_callback(
        FLAC__byte buffer[], size_t *bytes)
{
    Q_ASSERT(bytes);
    Q_ASSERT(m_source);
    if (!bytes || !m_source) return FLAC__STREAM_DECODER_READ_STATUS_ABORT;

    // check for EOF
    if (m_source->atEnd()) {
	*bytes = 0;
	return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
    }

    // read into application buffer
    *bytes = static_cast<unsigned>(m_source->read(
        reinterpret_cast<char *>(&(buffer[0])),
        static_cast<qint64>(*bytes)
    ));

    if (!*bytes) return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;

    return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

//***************************************************************************
::FLAC__StreamDecoderWriteStatus Kwave::FlacDecoder::write_callback(
        const ::FLAC__Frame *frame,
        const FLAC__int32 * const buffer[])
{
    Q_ASSERT(buffer);
    Q_ASSERT(frame);
    Q_ASSERT(m_dest);
    if (!buffer || !frame || !m_dest)
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;

    const unsigned int samples = frame->header.blocksize;

    const unsigned int tracks  = Kwave::FileInfo(metaData()).tracks();
    Q_ASSERT(samples);
    Q_ASSERT(tracks);
    if (!samples || !tracks)
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;

    Kwave::SampleArray dst(samples);

    // expand the samples up to the correct number of bits
    int shift = SAMPLE_BITS - Kwave::FileInfo(metaData()).bits();
    if (shift < 0) shift = 0;
    unsigned int mul = (1 << shift);

    // decode the samples into a temporary buffer and
    // flush it to the Writer(s), track by track
    for (unsigned int track=0; track < tracks; track++) {
	Kwave::Writer *writer = (*m_dest)[track];
	Q_ASSERT(writer);
	if (!writer) continue;
	const FLAC__int32 *src = buffer[track];
	sample_t *d = dst.data();

	for (unsigned int sample = 0; sample < samples; sample++) {
	    // the following cast is only necessary if
	    // sample_t is not equal to a quint32
	    sample_t s  = static_cast<sample_t>(*src++);

	    // correct precision
	    if (shift) s *= mul;

	    // write to destination buffer
	    *d++ = s;
	}

	// flush the temporary buffer
	(*writer) << dst;
    }

    // at this point we check for a user-cancel
    return (m_dest->isCanceled()) ?
	FLAC__STREAM_DECODER_WRITE_STATUS_ABORT :
	FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

//***************************************************************************
void Kwave::FlacDecoder::parseStreamInfo(
    const FLAC::Metadata::StreamInfo &stream_info)
{
    qDebug("FLAC stream info");
    qDebug("\tmin_blocksize   = %d", stream_info.get_min_blocksize());
    qDebug("\tmax_blocksize   = %d", stream_info.get_max_blocksize());
    qDebug("\tmin_framesize   = %d", stream_info.get_min_framesize());
    qDebug("\tmax_framesize   = %d", stream_info.get_max_framesize());

    Kwave::FileInfo info(metaData());
    info.setRate(stream_info.get_sample_rate());
    info.setTracks(stream_info.get_channels());
    info.setBits(stream_info.get_bits_per_sample());
    info.setLength(stream_info.get_total_samples());
    metaData().replace(Kwave::MetaDataList(info));

    qDebug("Bitstream is %u channel, %uHz",
           stream_info.get_channels(),
	   stream_info.get_sample_rate());
}

//***************************************************************************
void Kwave::FlacDecoder::parseVorbisComments(
        const FLAC::Metadata::VorbisComment &vorbis_comments)
{
    Kwave::FileInfo info(metaData());

    // first of all: the vendor string, specifying the software
    QString vendor = QString::fromUtf8(reinterpret_cast<const char *>(
	vorbis_comments.get_vendor_string()));
    if (vendor.length()) {
	info.set(Kwave::INF_SOFTWARE, vendor);
	qDebug("Encoded by: '%s'\n\n", DBG(vendor));
    }

    // parse all vorbis comments into Kwave file properties
    for (unsigned int i = 0; i < vorbis_comments.get_num_comments(); i++) {
	FLAC::Metadata::VorbisComment::Entry comment =
	    vorbis_comments.get_comment(i);
	Q_ASSERT(comment.is_valid());
	if (!comment.is_valid()) continue;

	QString name = QString::fromUtf8(
	    comment.get_field_name(), comment.get_field_name_length());
	QString value = QString::fromUtf8(
	    comment.get_field_value(), comment.get_field_value_length());

	if (!m_vorbis_comment_map.contains(name)) continue;

	// we have a known vorbis tag
	Kwave::FileProperty prop = m_vorbis_comment_map[name];
	info.set(prop, value);
    }

    // convert the date property to a QDate
    if (info.contains(Kwave::INF_CREATION_DATE)) {
	QString str_date =
	    QVariant(info.get(Kwave::INF_CREATION_DATE)).toString();
	QDate date;
	date = QDate::fromString(str_date, Qt::ISODate);
	if (!date.isValid()) {
	    int year = str_date.toInt();
	    date.setDate(year, 1, 1);
	}
	if (date.isValid()) info.set(Kwave::INF_CREATION_DATE, date);
     }

     metaData().replace(Kwave::MetaDataList(info));
}

//***************************************************************************
void Kwave::FlacDecoder::metadata_callback(
    const ::FLAC__StreamMetadata *metadata)
{
    Q_ASSERT(metadata);
    if (!metadata) return;

    switch (metadata->type) {
	case FLAC__METADATA_TYPE_STREAMINFO: {
	    FLAC::Metadata::StreamInfo stream_info(
	        const_cast< ::FLAC__StreamMetadata * >(metadata), true);
	    parseStreamInfo(stream_info);
	    break;
	}
	case FLAC__METADATA_TYPE_PADDING:
	    // -> ignored
	    break;
	case FLAC__METADATA_TYPE_APPLICATION:
	    qDebug("FLAC metadata: application data");
	    break;
	case FLAC__METADATA_TYPE_SEEKTABLE:
	    qDebug("FLAC metadata: seektable - not supported yet");
	    break;
	case FLAC__METADATA_TYPE_VORBIS_COMMENT: {
	    FLAC::Metadata::VorbisComment vorbis_comments(
	        const_cast< ::FLAC__StreamMetadata * >(metadata), true);
	    parseVorbisComments(vorbis_comments);
	    break;
	}
	case FLAC__METADATA_TYPE_CUESHEET:
	    qDebug("FLAC metadata: cuesheet - not supported yet");
	    break;
	case FLAC__METADATA_TYPE_UNDEFINED:
	default:
	    qDebug("FLAC metadata: unknown/undefined type");
    }
}

//***************************************************************************
void Kwave::FlacDecoder::error_callback(::FLAC__StreamDecoderErrorStatus status)
{
    qDebug("FlacDecoder::error_callback: status=%d", status);
}

//***************************************************************************
bool Kwave::FlacDecoder::open(QWidget *widget, QIODevice &src)
{
    metaData().clear();
    Q_ASSERT(!m_source);
    if (m_source) qWarning("FlacDecoder::open(), already open !");

    // try to open the source
    if (!src.open(QIODevice::ReadOnly)) {
	qWarning("failed to open source !");
	return false;
    }

    // take over the source
    m_source = &src;

    /********** Decoder setup ************/
    qDebug("--- FlacDecoder::open() ---");
    set_metadata_respond_all();

    // initialize the stream
    FLAC__StreamDecoderInitStatus init_state = init();
    if (init_state > FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        Kwave::MessageBox::error(widget, i18n(
           "Opening the FLAC bitstream failed."));
        return false;
    }

    // read in all metadata
    process_until_end_of_metadata();

    FLAC::Decoder::Stream::State state = get_state();
    if (state >= FLAC__STREAM_DECODER_END_OF_STREAM) {
	Kwave::MessageBox::error(widget, i18n(
	   "Error while parsing the FLAC metadata. (%s)"),
	   _(state.as_cstring()));
	return false;
    }

    // set some more standard properties
    Kwave::FileInfo info(metaData());
    info.set(Kwave::INF_MIMETYPE, _(DEFAULT_MIME_TYPE));
    info.set(Kwave::INF_COMPRESSION, Kwave::Compression::FLAC);
    metaData().replace(Kwave::MetaDataList(info));

    return true;
}

//***************************************************************************
bool Kwave::FlacDecoder::decode(QWidget * /* widget */,
                                Kwave::MultiWriter &dst)
{
    Q_ASSERT(m_source);
    if (!m_source) return false;

    m_dest = &dst;

    // read in all remaining data
    qDebug("FlacDecoder::decode(...)");
    process_until_end_of_stream();

    m_dest = 0;
    Kwave::FileInfo info(metaData());
    info.setLength(dst.last() ? (dst.last() + 1) : 0);
    metaData().replace(Kwave::MetaDataList(info));

    // return with a valid Signal, even if the user pressed cancel !
    return true;
}

//***************************************************************************
void Kwave::FlacDecoder::close()
{
    finish();
    m_source = 0;
}

//***************************************************************************
//***************************************************************************
