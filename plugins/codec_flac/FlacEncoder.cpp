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
#include <byteswap.h>

#include <qmemarray.h>
#include <qvaluevector.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kapp.h>
#include <kglobal.h>
#include <kaboutdata.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include <vorbis/vorbisenc.h>

#include "libkwave/FileInfo.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"

#include "FlacCodecPlugin.h"
#include "FlacEncoder.h"

/***************************************************************************/
FlacEncoder::FlacEncoder()
    :Encoder(), FLAC::Encoder::Stream(), m_vorbis_comment_map(), m_info(0),
    m_dst(0)
{
    LOAD_MIME_TYPES;
}

/***************************************************************************/
FlacEncoder::~FlacEncoder()
{
}

/***************************************************************************/
Encoder *FlacEncoder::instance()
{
    return new FlacEncoder();
}

/***************************************************************************/
QValueList<FileProperty> FlacEncoder::supportedProperties()
{
    QValueList<FileProperty> list;
    QMap<QString, FileProperty>::Iterator it;
    for (it=m_vorbis_comment_map.begin();
         it !=m_vorbis_comment_map.end(); ++it)
    {
        list.append(it.data());
    }
    return list;
}

/***************************************************************************/
::FLAC__StreamEncoderWriteStatus FlacEncoder::write_callback(
        const FLAC__byte buffer[], unsigned int bytes,
        unsigned int /* samples */, unsigned int /* current_frame */)
{
//    qDebug("FlacEncoder::write_callback(%u)", samples); // ###
    Q_ASSERT(m_dst);
    if (!m_dst) return FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR;

    m_dst->writeBlock((const char *)&(buffer[0]), bytes);
    return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
}

/***************************************************************************/
void FlacEncoder::metadata_callback(const ::FLAC__StreamMetadata *)
{
    /* we are not interested in the FLAC metadata */
}

/***************************************************************************/
FlacEncoder::VorbisCommentContainer::VorbisCommentContainer()
    :m_vc(0)
{
    m_vc = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
    Q_ASSERT(m_vc);
}

/***************************************************************************/
FlacEncoder::VorbisCommentContainer::~VorbisCommentContainer()
{
    if (m_vc) {
        // clean up all vorbis comments
    }
}

/***************************************************************************/
void FlacEncoder::VorbisCommentContainer::add(const QString &tag,
                                              const QString &value)
{
    Q_ASSERT(m_vc);
    if (!m_vc) return;

    unsigned int count = m_vc->data.vorbis_comment.num_comments;
    bool ok;

    QString s;
    s = tag+"="+value;
    qDebug("FlacEncoder::VorbisCommentContainer::add(%s)", s.data());

    // create a copy of the UTF-8 string
    QCString val = s.utf8();
    unsigned int len = val.length();
    FLAC__byte *copy = (FLAC__byte *)malloc(len);
    Q_ASSERT(copy);
    if (!copy) return;
    memcpy(copy, (const char *)val.data(), len);

    // put it into a vorbis_comment_entry structure
    FLAC__StreamMetadata_VorbisComment_Entry entry;
    entry.length = len;
    entry.entry  = copy;

    // insert the comment into the list
    ok =  FLAC__metadata_object_vorbiscomment_insert_comment(
	m_vc, count, entry, true);
    Q_ASSERT(ok);

}

/***************************************************************************/
FLAC__StreamMetadata *FlacEncoder::VorbisCommentContainer::data()
{
    return m_vc;
}

/***************************************************************************/
void FlacEncoder::encodeMetaData(FileInfo &info,
    QPtrVector<FLAC__StreamMetadata> &flac_metadata)
{
    // append some missing standard properties if they are missing
    if (!info.contains(INF_SOFTWARE)) {
        // add our Kwave Software tag
	const KAboutData *about_data = KGlobal::instance()->aboutData();
	QString software = about_data->programName() + "-" +
	    about_data->version() +
	    i18n(" for KDE ") + i18n(QString::fromLatin1(KDE_VERSION_STRING));
	qDebug("FlacEncoder: adding software tag: '%s'", software.latin1());
	info.set(INF_SOFTWARE, software);
    }

    if (!info.contains(INF_CREATION_DATE)) {
	// add a date tag
	QDate now(QDate::currentDate());
	QString date;
	date = date.sprintf("%04d-%02d-%02d",
	    now.year(), now.month(), now.day());
	QVariant value = date.utf8();
	qDebug("FlacEncoder: adding date tag: '%s'", date.latin1());
	info.set(INF_CREATION_DATE, value);
    }

    // encode all Vorbis comments
    VorbisCommentMap::ConstIterator it;
    VorbisCommentContainer vc;
    for (it = m_vorbis_comment_map.begin();
         it != m_vorbis_comment_map.end();
         ++it)
    {
	if (!info.contains(it.data())) continue; // not present -> skip

	QString value = info.get(it.data()).toString();
	vc.add(it.key(), value.utf8());
    }

    flac_metadata.resize(flac_metadata.size()+1);
    flac_metadata.insert(flac_metadata.size(), vc.data());

    // convert container to a list of pointers
    unsigned int meta_count = flac_metadata.size();
    if (meta_count) {
	if (!set_metadata(flac_metadata.data(), meta_count)) {
	    qWarning("FlacEncoder: setting meta data failed !?");
	}
    }

}

/***************************************************************************/
bool FlacEncoder::encode(QWidget *widget, MultiTrackReader &src,
                         QIODevice &dst, FileInfo &info)
{
    qDebug("FlacEncoder::encode()"); // ###
    m_info = &info;
    m_dst  = &dst;

    // get info: tracks, sample rate
    unsigned int tracks = m_info->tracks();
    unsigned int bits   = m_info->bits();
    unsigned int length = m_info->length();

    set_channels(tracks);
    set_bits_per_sample(bits);
    set_sample_rate((long)m_info->rate());
    set_total_samples_estimate(length);

    // use mid-side stereo encoding if we have two channels
    set_do_mid_side_stereo(tracks == 2);
    set_loose_mid_side_stereo(tracks == 2);

    // encode meta data, most of them as vorbis comments
    QPtrVector<FLAC__StreamMetadata> flac_metadata;
    encodeMetaData(info, flac_metadata);

    // open the output device
    if (!dst.open(IO_ReadWrite | IO_Truncate)) {
	KMessageBox::error(widget,
	    i18n("Unable to open the file for saving!"));
	m_info = 0;
	m_dst  = 0;
	dst.close();
	return false;
    }

    FLAC::Encoder::Stream::State state = init();

    if (state != FLAC__STREAM_ENCODER_OK) {
	qWarning("state = %s", state.as_cstring());
	KMessageBox::error(widget,
	    i18n("Unable to open the FLAC encoder!"));
	m_info = 0;
	m_dst  = 0;
	dst.close();
	return false;
    }

    // allocate output buffers, with FLAC 32 bit format
    unsigned int len = 8192; // samples
    QPtrVector< QMemArray<FLAC__int32> > out_buffer;
    out_buffer.resize(tracks);
    out_buffer.fill(0);
    out_buffer.setAutoDelete(true);
    Q_ASSERT(out_buffer.size() == tracks);

    QMemArray<FLAC__int32 *> flac_buffer;
    flac_buffer.resize(tracks);

    unsigned int track = 0;
    for (unsigned int track=0; track < tracks; track++)
    {
	QMemArray<FLAC__int32> *buf = new QMemArray<FLAC__int32>((int)len);
	Q_ASSERT(buf);
	if (!buf || (buf->size() < len)) {
	    out_buffer.clear();
	    break;
	}
	out_buffer.insert(track, buf);
	buf->detach();
	flac_buffer[track] = buf->data();
    }

    // allocate input buffer, with Kwave's sample_t
    QMemArray<sample_t> in_buffer;
    in_buffer.resize(len);
    Q_ASSERT(in_buffer.size() == len);
    Q_ASSERT(out_buffer.size() == tracks);

    if ((in_buffer.size() < len) || (out_buffer.size() < tracks) ||
	(flac_buffer.size() < tracks))
    {
	KMessageBox::error(widget, i18n("Out of memory!"));
	m_info = 0;
	m_dst  = 0;
	dst.close();
	return false;
    }

    // calculate divisor for reaching the proper resolution
    int shift = SAMPLE_BITS - bits;
    if (shift < 0) shift = 0;
    unsigned int div = (1 << shift);

    unsigned int rest = length;
    while (rest && len && !src.isCancelled()) {
	// limit to rest of signal
	if (len > rest) len = rest;
	if (in_buffer.size() > len) in_buffer.resize(len);

	// add all samples to one single buffer
	for (track=0; track < tracks; track++) {
	    SampleReader *reader = src[track];
	    Q_ASSERT(reader);
	    if (!reader) break;

	    (*reader) >> in_buffer; // read samples into in_buffer
	    len = in_buffer.size(); // in_buffer might have been shrinked!
	    Q_ASSERT(len);

	    for (register unsigned int in_pos=0; in_pos < len; in_pos++) {
		register FLAC__int32 s = in_buffer[in_pos];
		if (div) s /= div;
		flac_buffer[track][in_pos] = s;
	    }
	}

	// process all collected samples
	FLAC__int32 **buffer = flac_buffer.data();
	bool processed = process(buffer, len);
	Q_ASSERT(processed);
	if (!processed) {
	    len = 0;
	    break;
	}

	rest -= len;
    }

    // close the output device and the FLAC stream
    finish();

    m_info = 0;
    m_dst  = 0;
    dst.close();
    out_buffer.clear();

    return true;
}

/***************************************************************************/
/***************************************************************************/
