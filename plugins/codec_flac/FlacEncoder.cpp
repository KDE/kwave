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
void FlacEncoder::encodeMetaData(FileInfo &info)
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

    // ### TODO ###

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
    QValueVector< QMemArray<FLAC__int32> > out_buffer;
    out_buffer.resize(tracks);
    Q_ASSERT(out_buffer.size() == tracks);

    QMemArray<FLAC__int32 *> flac_buffer;
    flac_buffer.resize(tracks);

    QValueVector< QMemArray<FLAC__int32> >::Iterator it;
    unsigned int track = 0;
    for (it=out_buffer.begin(); it != out_buffer.end(); ++it) {
	(*it).resize(len);
	Q_ASSERT(it->size() == len);
	if (it->size() != len) {
	    out_buffer.clear();
	    break;
	}
	flac_buffer[track++] = it->data();
    }

    // allocate input buffer, with Kwave's sample_t
    QMemArray<sample_t> in_buffer;
    in_buffer.resize(len);
    Q_ASSERT(in_buffer.size() == len);

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

	    for (unsigned int in_pos=0; in_pos < len; in_pos++) {
		register FLAC__int32 s = in_buffer[in_pos];
		if (div) s /= div;
		out_buffer[track][in_pos] = s;
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

    return true;
}

/***************************************************************************/
/***************************************************************************/
