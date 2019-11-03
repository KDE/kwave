/*************************************************************************
    OpusEncoder.cpp  -  sub encoder base class for Opus in an Ogg container
                             -------------------
    begin                : Thu Jan 03 2013
    copyright            : (C) 2013 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************

    parts based on source snippets taken from "opusenc.c", opus-tools-0.1.5
   -------------------------------------------------------------------------
   Copyright (C) 2002-2011 Jean-Marc Valin <jmvalin@jmvalin.ca>
   Copyright (C) 2007-2012 Xiph.Org Foundation <monty@xiph.org/>
   Copyright (C) 2008-2012 Gregory Maxwell <greg@xiph.org>
   File: opusenc.c

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ***************************************************************************/

#include "config.h"

#include <math.h>
#include <string.h>
#include <new>

#include <opus/opus_defines.h>

#include <QApplication>
#include <QBuffer>
#include <QByteArray>
#include <QList>
#include <QString>
#include <QTime>
#include <QtGlobal>
#include <QtEndian>

#include <KLocalizedString>

#include "libkwave/BitrateMode.h"
#include "libkwave/Connect.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleArray.h"
#include "libkwave/Utils.h"
#include "libkwave/modules/ChannelMixer.h"
#include "libkwave/modules/RateConverter.h"

#include "OpusCommon.h"
#include "OpusEncoder.h"

/** lowest supported bitrate */
#define BITRATE_MIN 500

/** highest supported bitrate */
#define BITRATE_MAX 256000

/** lowest (reasonable) supported sample rate */
#define SAMPLE_RATE_MIN 1000

/** highest (reasonable) supported sample rate */
#define SAMPLE_RATE_MAX 512000

/** default encoder complexity */
#define DEFAULT_COMPLEXITY 10

/***************************************************************************/
Kwave::OpusEncoder::OpusEncoder()
    :m_comments_map(),
     m_info(),
     m_downmix(DOWNMIX_AUTO),
     m_bitrate(0),
     m_coding_rate(0),
     m_encoder_channels(0),
     m_channel_mixer(Q_NULLPTR),
     m_rate_converter(Q_NULLPTR),
     m_frame_size(0),
     m_extra_out(0),
     m_opus_header(),
     m_max_frame_bytes(0),
     m_packet_buffer(Q_NULLPTR),
     m_encoder(Q_NULLPTR),
     m_encoder_input(Q_NULLPTR),
     m_last_queue_element(Q_NULLPTR),
     m_buffer(Q_NULLPTR)
{

    memset(&m_opus_header, 0x00, sizeof(m_opus_header));
    memset(&m_opus_header.map, 0xFF, sizeof(m_opus_header.map));
}

/***************************************************************************/
Kwave::OpusEncoder::~OpusEncoder()
{
}

/***************************************************************************/
bool Kwave::OpusEncoder::setupDownMix(QWidget *widget, unsigned int tracks,
                                      int bitrate)
{
    // get "downmix" setting, default is "auto"
    m_downmix = DOWNMIX_AUTO; // currently not user configurable

    if ((m_downmix == DOWNMIX_AUTO) &&
	(bitrate > 0) && (bitrate < (32000 * Kwave::toInt(tracks))))
    {
	if (tracks > 8) {
	    // downmix from more than 8 channels to mono
	    if (Kwave::MessageBox::warningContinueCancel(
		widget,
		i18n("Surround bitrate would be less than 32kBit/sec per "
		      "channel, this file should be mixed down to mono."),
		QString(), QString(), QString(),
		_("opus_accept_down_mix_on_export")) != KMessageBox::Continue)
	    {
		return false;
	    }
	    m_downmix = DOWNMIX_MONO;
	} else if (tracks > 2) {
	    // downmix from more than stereo to stereo
	    if (Kwave::MessageBox::warningContinueCancel(
		widget,
		i18n("Surround bitrate would be less than 32kBit/sec per "
		      "channel, this file should be mixed down to stereo."),
		QString(), QString(), QString(),
		_("opus_accept_down_mix_on_export")) != KMessageBox::Continue)
	    {
		return false;
	    }
	    m_downmix = DOWNMIX_STEREO;
	}
    }
    if (m_downmix == DOWNMIX_AUTO) // if still "auto"
	m_downmix = DOWNMIX_OFF;   // then switch it off

    switch (m_downmix) {
	case DOWNMIX_MONO:   m_encoder_channels = 1;      break;
	case DOWNMIX_STEREO: m_encoder_channels = 2;      break;
	default:             m_encoder_channels = tracks; break;
    }

    if (m_encoder_channels != tracks) {
	// create a channel mixer
	m_channel_mixer = new(std::nothrow)
	    Kwave::ChannelMixer(tracks, m_encoder_channels);
	Q_ASSERT(m_channel_mixer);
	if (!m_channel_mixer || !m_channel_mixer->init()) {
	    qWarning("creating channel mixer failed");
	    return false;
	}

	// connect it to the end of the current preprocessing queue
	// (normally this is the original sample source)
	if (!Kwave::connect(
	    *m_last_queue_element, SIGNAL(output(Kwave::SampleArray)),
	    *m_channel_mixer,      SLOT(input(Kwave::SampleArray))))
	{
	    qWarning("connecting the channel mixer failed");
	    return false;
	}
	m_last_queue_element = m_channel_mixer;
    }

    return true;
}

/***************************************************************************/
bool Kwave::OpusEncoder::setupBitrate(QWidget *widget, unsigned int tracks)
{
    int bitrate_nominal = m_info.contains(Kwave::INF_BITRATE_NOMINAL) ?
        QVariant(m_info.get(Kwave::INF_BITRATE_NOMINAL)).toInt() : -1;
    int bitrate_lower = m_info.contains(Kwave::INF_BITRATE_LOWER) ?
        QVariant(m_info.get(Kwave::INF_BITRATE_LOWER)).toInt() : -1;
    int bitrate_upper = m_info.contains(Kwave::INF_BITRATE_UPPER) ?
        QVariant(m_info.get(Kwave::INF_BITRATE_UPPER)).toInt() : -1;

    // prefer bitrates in this order:
    // nominal -> upper -> lower -> "auto" (-1)
    int bitrate = -1;
    if (bitrate_nominal > 0) bitrate = bitrate_nominal;
    else if (bitrate_upper   > 0) bitrate = bitrate_upper;
    else if (bitrate_lower   > 0) bitrate = bitrate_lower;

    if ((bitrate > 0) && ((bitrate > (BITRATE_MAX * Kwave::toInt(tracks)))
        || (bitrate < BITRATE_MIN)))
    {
	int bitrate_new =
	    qBound<int>(BITRATE_MIN, bitrate, BITRATE_MAX * tracks);

	if (Kwave::MessageBox::warningContinueCancel(
	    widget,
	    i18nc("%1=original bitrate, %2=new/limited bitrate",
	          "Bitrate %1 kBit/sec is out of range, "
	          "limited to %2 kBit/sec",
	           bitrate / 1000,
	           bitrate_new / 1000),
	    QString(), QString(), QString(),
	    _("opus_bitrate_limit"))  != KMessageBox::Continue)
	{
	    return false;
	}
    }

    if (bitrate > 0)
	qDebug("    OpusEncoder: bitrate %d bits/sec (configured)", bitrate);
    m_bitrate = bitrate;
    return true;
}

/***************************************************************************/
bool Kwave::OpusEncoder::setupCodingRate(QWidget *widget,
                                         unsigned int tracks, double rate)
{
    Q_UNUSED(widget)

    Q_ASSERT(!m_rate_converter);

    int rate_orig = Kwave::toInt(rate);
    int rate_supp = Kwave::opus_next_sample_rate(rate_orig);

    m_coding_rate = rate_supp;

    if (rate_orig == rate_supp) {
   	qDebug("    OpusEncoder: using sample rate %d", rate_orig);
	return true; // no conversion needed :-)
    }

    double rate_from = static_cast<double>(rate_orig);
    double rate_to   = static_cast<double>(rate_supp);
    double ratio     = rate_to / rate_from;

    qDebug("    OpusEncoder: converting sample rate: %d -> %d",
           rate_orig, rate_supp);

    // range check: conversion ration must be between 1/256 and 256
    if ((ratio < (1.0 / 256.0)) || (ratio > 256.0)) {
	int lowest  = qMin<int>(SAMPLE_RATE_MIN,
	                        Kwave::toInt(ceil( rate_to / 256.0)));
	int highest = qMax<int>(Kwave::toInt(floor(rate_to * 256.0)),
	                        SAMPLE_RATE_MAX);
	Kwave::MessageBox::sorry(
	    widget,
	    i18nc("%1=requested sample rate, "
	          "%2=lowest supported, %3=highest supported",
	          "Sample rate %1 samples/sec is out of range,\n"
	          "supported are %2 ... %3 samples/sec.",
	           rate_supp, lowest, highest),
	    QString()
	);
	return false;
    }

    // create a new rate converter
    m_rate_converter = new(std::nothrow)
	Kwave::MultiTrackSource<Kwave::RateConverter, true>(tracks);
    Q_ASSERT(m_rate_converter);
    if (!m_rate_converter)
	return false;

    m_rate_converter->setAttribute(
	SLOT(setRatio(QVariant)),
	QVariant(ratio)
    );

    // connect the rate converter to the end of the current preprocessing
    // queue/ (normally this is either the original sample source or
    // a channel mixer)
    if (!Kwave::connect(
	*m_last_queue_element, SIGNAL(output(Kwave::SampleArray)),
	*m_rate_converter,     SLOT(input(Kwave::SampleArray))))
    {
	qWarning("connecting the rate converter failed");
	return false;
    }
    m_last_queue_element = m_rate_converter;

    return true;
}

/***************************************************************************/
bool Kwave::OpusEncoder::setupEncoder(QWidget *widget, unsigned int tracks,
                                      double rate)
{
    // get the frame size in ms if present,
    // otherwise fall back to the default of 20ms
    // (supported values are 2.5, 5, 10, 20, 40, or 60 ms)
    qreal ms_per_frame = 20.0; // default is 20ms
    if (m_info.contains(INF_OPUS_FRAME_LEN)) {
	double len = m_info.get(INF_OPUS_FRAME_LEN).toDouble();
	if (len >= 60)
	    ms_per_frame = 60.0;
	else if (len >= 40)
	    ms_per_frame = 40.0;
	else if (len >= 20)
	    ms_per_frame = 20.0;
	else if (len >= 5)
	    ms_per_frame = 5;
	else
	    ms_per_frame = 2.5;
   	qDebug("    OpusEncoder: %0.1f ms/frame", ms_per_frame);
    } else {
	ms_per_frame = 20.0; // <- default
   	qDebug("    OpusEncoder: %0.1f ms/frame (default)", ms_per_frame);
    }

    // calculate the frame size in samples from the frame duration in ms
    // = frame_length [ms] * bitrate [bits/sec] / 1000 [ms/sec]
    m_frame_size = Kwave::toUint(
	(ms_per_frame * m_coding_rate) / 1000);

    if (tracks > 255) {
	qWarning("too many tracks: %u, supported: 255", tracks);
	return false; // more than 255 tracks are not supported
    }

    // fill out all header fields
    m_opus_header.channels        = static_cast<quint8>(tracks);
    m_opus_header.preskip         = 0;
    m_opus_header.sample_rate     = static_cast<quint32>(rate);
    m_opus_header.gain            = 0;
    m_opus_header.channel_mapping = 255;
    m_opus_header.streams         = static_cast<quint8>(tracks);
    m_opus_header.coupled         = 0;

    // determine channel mapping and coupling
    quint8 force_narrow = 0x00;
    if (tracks <= 8) {
	/* apply a mapping as done in opusenc.c from opus-tools-0.1.5 */
	static const quint8 opusenc_streams[8][10]= {
	    /*       Coupled,   NB_bitmap, mapping...*/
	    /* 1 */ {0,         0,         0                      },
	    /* 2 */ {1,         0,         0, 1                   },
	    /* 3 */ {1,         0,         0, 2, 1                },
	    /* 4 */ {2,         0,         0, 1, 2, 3             },
	    /* 5 */ {2,         0,         0, 4, 1, 2, 3          },
	    /* 6 */ {2,    1 << 3,         0, 4, 1, 2, 3, 5       },
	    /* 7 */ {2,    1 << 4,         0, 4, 1, 2, 3, 5, 6    },
	    /* 8 */ {3,    1 << 4,         0, 6, 1, 2, 3, 4, 5, 7 }
	};
	for (unsigned int i = 0; i < tracks; i++)
	    m_opus_header.map[i] = opusenc_streams[tracks - 1][i + 2];
	force_narrow = opusenc_streams[tracks - 1][1];
	m_opus_header.coupled         = opusenc_streams[tracks - 1][0];
	m_opus_header.streams         = static_cast<quint8>(
	    tracks - m_opus_header.coupled);
	m_opus_header.channel_mapping = (m_opus_header.streams > 1);

   	qDebug("    OpusEncoder: %d stream(s) / %d coupled (mapping=%d)",
	       m_opus_header.streams, m_opus_header.coupled,
	       m_opus_header.channel_mapping);
    } else {
	/* map all channels 1:1 */
	for (quint8 i = 0; i < m_opus_header.channels; ++i)
	    m_opus_header.map[i] = i;
   	qDebug("    OpusEncoder: mapping channels 1:1");
    }

    // allocate a packet buffer
    m_max_frame_bytes = ((1275 * 3) + 7) * m_opus_header.streams;
    qDebug("    OpusEncoder: max frame size %u bytes", m_max_frame_bytes);
    Q_ASSERT(!m_packet_buffer);
    m_packet_buffer = static_cast<unsigned char *>(malloc(m_max_frame_bytes));
    if (!m_packet_buffer) {
	Kwave::MessageBox::error(widget, i18n("Out of memory"));
	return false;
    }

    // initialize the Opus encoder:
    // frame sizes < 10ms can only use the MDCT modes,
    // so we switch on RESTRICTED_LOWDELAY to save the extra 2.5ms of codec
    // lookahead when we'll be using only small frames

    int err = OPUS_ALLOC_FAIL;
    Q_ASSERT(!m_encoder);
    m_encoder = opus_multistream_encoder_create(
	m_coding_rate,
	tracks,
	m_opus_header.streams,
	m_opus_header.coupled,
	m_opus_header.map,
	(ms_per_frame < 10.0) ? OPUS_APPLICATION_RESTRICTED_LOWDELAY :
	                        OPUS_APPLICATION_AUDIO,
	&err
    );
    if (err != OPUS_OK) {
	Kwave::MessageBox::error(widget, Kwave::opus_error(err),
	     i18n("Opus encoder failed"));
	return false;
    }

    if (force_narrow) {
	for (unsigned int i = 0; i < m_opus_header.streams; i++ ) {
	    if (force_narrow & (1 << i)) {
                ::OpusEncoder *oe = Q_NULLPTR;

		opus_multistream_encoder_ctl(
		    m_encoder,
		    OPUS_MULTISTREAM_GET_ENCODER_STATE_REQUEST, i, &oe
		);

		int err_ctl = opus_encoder_ctl(oe,
		    OPUS_SET_MAX_BANDWIDTH_REQUEST, OPUS_BANDWIDTH_NARROWBAND);

		if (err_ctl != OPUS_OK) {
		    Kwave::MessageBox::error(widget, Kwave::opus_error(err_ctl),
			i18n("Opus encoder failed"));
		    return false;
		}
	    }
	}
    }

    // allocate a buffer to be used as input of the encoder
    m_encoder_input = static_cast<float *>(
	malloc(sizeof(float) * m_frame_size * tracks));
    if (!m_encoder_input) {
	Kwave::MessageBox::error(widget, i18n("Out of memory"));
	return false;
    }

    return true;
}

/***************************************************************************/
bool Kwave::OpusEncoder::setupBitrateMode(QWidget *widget)
{
    const bool with_cvbr = false;
    int err;

    // determine a reasonable bitrate in case we still use "autodetect"
    if (m_bitrate < 0) {
	m_bitrate = (64000 * m_opus_header.streams) +
	            (32000 * m_opus_header.coupled);
	m_bitrate = qBound<int>(500, m_bitrate, 256000);
	qDebug("    OpusEncoder: bitrate %d bits/sec (auto)", m_bitrate);
    }

    err = opus_multistream_encoder_ctl(m_encoder, OPUS_SET_BITRATE(
	static_cast<opus_int32>(m_bitrate)));
    if (err != OPUS_OK) {
	Kwave::MessageBox::error(widget,
	    i18n("Opus encoder failed setting bitrate: '%1'",
	         Kwave::opus_error(err)));
	return false;
    }

    int bitrate_mode = m_info.get(INF_BITRATE_MODE).toInt();
    bool with_hard_cbr = (bitrate_mode == BITRATE_MODE_CBR_HARD);

    err = opus_multistream_encoder_ctl(m_encoder, OPUS_SET_VBR(
	static_cast<opus_int32>(with_hard_cbr ? 0 : 1)));
    if (err != OPUS_OK) {
	Kwave::MessageBox::error(widget,
	    i18n("Opus encoder failed configuring VBR mode: '%1'",
	         Kwave::opus_error(err)));
	return false;
    }

    if (!with_hard_cbr ) {
        err = opus_multistream_encoder_ctl(m_encoder, OPUS_SET_VBR_CONSTRAINT(
	    static_cast<opus_int32>(with_cvbr ? 1 : 0)));
	if (err != OPUS_OK) {
	    Kwave::MessageBox::error(widget,
		i18n("Opus encoder failed configuring VBR constraint: '%1'",
		    Kwave::opus_error(err)));
	    return false;
	}
    }

    return true;
}

/***************************************************************************/
bool Kwave::OpusEncoder::open(QWidget *widget, const Kwave::FileInfo &info,
                              Kwave::MultiTrackReader &src)
{
    // get info: tracks, sample rate, bitrate(s)
    m_info = info;
    const unsigned int src_tracks  = m_info.tracks();
    const double       sample_rate = m_info.rate();
    int err;

    // reset everything to defaults
    m_downmix            = DOWNMIX_AUTO;
    m_bitrate            = -1;
    m_coding_rate        = 0;
    m_extra_out          = 0;
    m_frame_size         = 0;
    memset(&m_opus_header, 0x00, sizeof(m_opus_header));
    memset(&m_opus_header.map, 0xFF, sizeof(m_opus_header.map));
    m_max_frame_bytes    = 0;
    m_last_queue_element = &src;

    // get the desired bitrate
    if (!setupBitrate(widget, src_tracks))
	return false;

    // determine the down mixing mode
    // and set up the mixer if necessary
    if (!setupDownMix(widget, src_tracks, m_bitrate))
	return false;

    // determine the decoding sample rate
    // and set up the rate converter if necessary
    if (!setupCodingRate(widget, m_encoder_channels, sample_rate))
	return false;

    // set up mapping, packet size and encoder
    if (!setupEncoder(widget, m_encoder_channels, sample_rate))
	return false;

    // set up bitrate mode (e.g. VBR, ABR, ...)
    if (!setupBitrateMode(widget))
	return false;

    // create a sample buffer at the end of the filter chain
    m_buffer = new(std::nothrow)
	Kwave::MultiTrackSink<Kwave::SampleBuffer, true>(m_encoder_channels);
    Q_ASSERT(m_buffer);
    if (!m_buffer) {
	qWarning("cannot create sample buffer");
	return false;
    }
    if (!Kwave::connect(
	*m_last_queue_element, SIGNAL(output(Kwave::SampleArray)),
	*m_buffer,             SLOT(input(Kwave::SampleArray))) )
    {
	qWarning("failed to connect sample buffer");
	return false;
    }

    // set up the encoder complexity (ignore errors)
    opus_int32 complexity = DEFAULT_COMPLEXITY;
    err = opus_multistream_encoder_ctl(m_encoder,
	OPUS_SET_COMPLEXITY(complexity));
    if (err != OPUS_OK) {
	qWarning("OpusEncoder: failed setting encoder complexity: '%s'",
	         DBG(Kwave::opus_error(err)));
    }

    // set up the expected loss [percent] (ignore errors)
    opus_int32 expect_loss = 0;
    err = opus_multistream_encoder_ctl(m_encoder,
	OPUS_SET_PACKET_LOSS_PERC(expect_loss));
    if (err != OPUS_OK) {
	qWarning("OpusEncoder: failed setting expected loss: '%s'",
	         DBG(Kwave::opus_error(err)));
    }

#ifdef OPUS_SET_LSB_DEPTH
    // set up the LSB depth
    opus_int32 bits = qBound<unsigned int>(8, m_info.bits(), 24);
    err = opus_multistream_encoder_ctl(m_encoder, OPUS_SET_LSB_DEPTH(bits));
    if (err != OPUS_OK) {
	qWarning("OpusEncoder: failed setting LSB depth loss: '%s'",
	         DBG(Kwave::opus_error(err)));
    }
#endif /* OPUS_SET_LSB_DEPTH */

    // get the lookahead value
    opus_int32 lookahead;
    err = opus_multistream_encoder_ctl(m_encoder,
	OPUS_GET_LOOKAHEAD(&lookahead));
    if (err != OPUS_OK) {
	Kwave::MessageBox::error(widget,
	    i18n("Opus encoder failed getting lookahead value: '%1'",
	         Kwave::opus_error(err)));
	return false;
    }

    // regardless of the rate we're coding at the ogg timestamping/skip is
    //  always timed at 48000 kBit/s
    m_opus_header.preskip = static_cast<quint16>(
	lookahead * (48000.0 / m_coding_rate));
    qDebug("    OpusEncoder: preskip=%d", m_opus_header.preskip);

    /* Extra samples that need to be read to compensate for the pre-skip */
    m_extra_out = lookahead;


    // set up our packet->stream encoder
    // pick a random serial number; that way we can more likely build
    // chained streams just by concatenation
    qsrand(QTime::currentTime().msec() ^ qrand());
    ogg_stream_init(&m_os, qrand());

    return true;
}

/***************************************************************************/
static inline void _writeInt(QBuffer &buffer, quint32 value)
{
    quint32 x = qToLittleEndian<quint32>(value);
    buffer.write(reinterpret_cast<const char *>(&x), sizeof(x));
}

/***************************************************************************/
bool Kwave::OpusEncoder::writeOpusHeader(QIODevice &dst)
{
    Kwave::opus_header_t header;
    unsigned int len;

    // create an Opus header, using correct byte order
    memset(&header, 0x00, sizeof(header));
    memset(&header.map, 0xFF, sizeof(header.map));

    memcpy(&(header.magic[0]), "OpusHead", 8);
    header.version     = 1;
    header.channels    = m_opus_header.channels;
    header.preskip     = qToLittleEndian<quint16>(m_opus_header.preskip);
    header.sample_rate = qToLittleEndian<quint32>(m_opus_header.sample_rate);
    header.gain        = qToLittleEndian<quint16>(m_opus_header.gain);
    header.channel_mapping = m_opus_header.channel_mapping;
    len = 19; // bytes so far
    if (m_opus_header.channel_mapping) {
	header.streams = m_opus_header.streams;
	header.coupled = m_opus_header.coupled;
	len += 2;

	for (quint8 i = 0; i < m_opus_header.channels; ++i)
	    header.map[i] = m_opus_header.map[i];
	len += m_opus_header.channels;
    }

    // write the header into the ogg stream
    m_op.packet     = reinterpret_cast<unsigned char *>(&header);
    m_op.bytes      = len;
    m_op.b_o_s      = 1;
    m_op.e_o_s      = 0;
    m_op.granulepos = 0;
    m_op.packetno   = 0;
    ogg_stream_packetin(&m_os, &m_op);

    while (ogg_stream_flush(&m_os, &m_og)) {
	dst.write(reinterpret_cast<char *>(m_og.header),
	          m_og.header_len);
	dst.write(reinterpret_cast<char *>(m_og.body),
	          m_og.body_len);
    }

    return true;
}

/***************************************************************************/
bool Kwave::OpusEncoder::writeOpusTags(QIODevice &dst)
{
    QBuffer buffer;

    buffer.open(QBuffer::ReadWrite);

    // let the header start with the magic string "OpusTags"
    buffer.write("OpusTags", 8);

    // write the vendor string == name + version of the encoder library
    const char *opus_version = opus_get_version_string();
    quint32 len = quint32(strlen(opus_version));
    _writeInt(buffer, len);
    buffer.write(opus_version, len);

    // iterate over all known properties and collect them in a list
    QList<QByteArray> tags;
    len = 0;

    for (VorbisCommentMap::const_iterator it(m_comments_map.constBegin());
         it != m_comments_map.constEnd(); ++it)
    {
	const QString       &key     = it.key();
	Kwave::FileProperty property = it.value();
	if (!m_info.contains(property)) continue; // skip if not present

	// convert the value into a byte array,  UTF-8 encoded
	QString str = key + _("=") + m_info.get(property).toString();
	QByteArray v = str.toUtf8();

	// make sure that the "ENCODER" tag is at the start of the list
	if ((property == INF_SOFTWARE) && (!tags.isEmpty()))
	    tags.prepend(v);
	else
	    tags.append(v);

	len += (4 + v.size()); // sum up the data length
    }

    // write the number of user tags
    _writeInt(buffer, tags.count());

    // serialize the tags into the buffer
    foreach (const QByteArray &tag, tags) {
	_writeInt(buffer, tag.size());
	buffer.write(tag);
    }

    m_op.packet     = reinterpret_cast<unsigned char *>(buffer.buffer().data());
    m_op.bytes      = static_cast<long int>(buffer.size());
    m_op.b_o_s      = 0;
    m_op.e_o_s      = 0;
    m_op.granulepos = 0;
    m_op.packetno   = 1;
    ogg_stream_packetin(&m_os, &m_op);

    while (ogg_stream_flush(&m_os, &m_og)) {
	if (!writeOggPage(dst)) return false;
    }

    return true;
}

/***************************************************************************/
bool Kwave::OpusEncoder::writeHeader(QIODevice &dst)
{
    if (!writeOpusHeader(dst))
	return false;

    if (!writeOpusTags(dst))
	return false;

    return true;
}

/***************************************************************************/
bool Kwave::OpusEncoder::writeOggPage(QIODevice &dst)
{
    qint64 n;

    n = dst.write(reinterpret_cast<char *>(m_og.header), m_og.header_len);
    if (n != m_og.header_len) {
	qWarning("OpusEncoder: I/O error writing header, len=%u, written=%u",
	          static_cast<unsigned int>(n),
	          static_cast<unsigned int>(m_og.header_len));
	return false; // write error ?
    }

    n = dst.write(reinterpret_cast<char *>(m_og.body),   m_og.body_len);
    if (n != m_og.body_len) {
	qWarning("OpusEncoder: I/O error writing body, len=%u, written=%u",
	          static_cast<unsigned int>(n),
	          static_cast<unsigned int>(m_og.body_len));
	return false; // write error ?
    }

    // update the progress bar
    QApplication::processEvents();

    return true;
}

/***************************************************************************/
unsigned int Kwave::OpusEncoder::fillInBuffer(Kwave::MultiTrackReader &src)
{
    unsigned int min_count = m_frame_size + 1; // will be used as "invalid"

    for (unsigned int t = 0; t < m_encoder_channels; ++t) {
	Kwave::SampleBuffer *buf = m_buffer->at(t);
	Q_ASSERT(buf);
	if (!buf) return 0;

	unsigned int count = 0;
	unsigned int rest  = m_frame_size;
	while (rest) {
	    float *p = m_encoder_input + t;

	    // while buffer is empty and source is not at eof:
	    // trigger the start of the chain to produce some data
	    while (!buf->available() && !src.eof())
		src.goOn();
	    const unsigned int avail = buf->available();
	    if (!avail) break; // reached EOF

	    // while there is something in the current buffer
	    // and some rest is still to do
	    unsigned int len = qMin<unsigned int>(rest, avail);
	    const sample_t *s = buf->get(len);
	    Q_ASSERT(s);
	    if (!s) break;

	    // fill the frame data
	    rest  -= len;
	    count += len;
	    while (len--) {
		*p = sample2float(*(s++));
		p += m_encoder_channels;
	    }
	}
	if (count < min_count) min_count = count;
    }

    // take the minimum number of samples if valid, otherwise zero (eof?)
    unsigned int n = (min_count <= m_frame_size) ? min_count : 0;

    // if we were not able to fill a complete frame, we probably are at eof
    // and have some space to pad with extra samples to compensate preskip
    while ((n < m_frame_size) && m_extra_out) {
	Q_ASSERT(src.eof());
	for (unsigned int t = 0; t < m_encoder_channels; ++t) {
	    m_encoder_input[(n + t) * m_encoder_channels] = 0.0;
	}
	m_extra_out--;
	n++;
    }

    return n;
}

/***************************************************************************/
bool Kwave::OpusEncoder::encode(Kwave::MultiTrackReader &src,
                                QIODevice &dst)
{
    long int     eos             =  0;
    opus_int64   nb_encoded      =  0;
    opus_int64   nb_samples      = -1;
    opus_int64   total_bytes     =  0;
    opus_int64   total_samples   =  0;
    ogg_int64_t  enc_granulepos  =  0;
    ogg_int64_t  last_granulepos =  0;
    ogg_int32_t  packet_id       =  1;
    int          last_segments   =  0;
    const int    max_ogg_delay   =  48000; /* 48kHz samples */

    Q_ASSERT(m_encoder);
    Q_ASSERT(m_encoder_input);

    Q_UNUSED(dst)

    /* Main encoding loop (one frame per iteration) */
    while (!m_op.e_o_s && !src.isCanceled()) {
	int size_segments = 0;

	packet_id++;

	if (nb_samples < 0) {
	    nb_samples = fillInBuffer(src);
	    total_samples += nb_samples;
	    m_op.e_o_s = (nb_samples < m_frame_size) ? 1 : 0;
	}
	m_op.e_o_s |= eos; // eof from last pass

	// pad the rest of the frame with zeroes if necessary
	if (nb_samples < m_frame_size ) {
	    const unsigned int pad_from =
		Kwave::toUint(nb_samples * m_encoder_channels);
	    const unsigned int pad_to   =
		Kwave::toUint(m_frame_size * m_encoder_channels);
	    for (unsigned int pos = pad_from; pos < pad_to; pos++ )
		m_encoder_input[pos] = 0;
	}

	/* encode current frame */
	int nbBytes = opus_multistream_encode_float(
	    m_encoder,
	    m_encoder_input,
	    m_frame_size,
	    m_packet_buffer,
	    m_max_frame_bytes
	);
	if (nbBytes < 0 ) {
	    qWarning("Opus encoder failed: '%s'",
		    DBG(Kwave::opus_error(nbBytes)));
	    return false;
	}

	nb_encoded     += m_frame_size;
	enc_granulepos += m_frame_size * 48000 / m_coding_rate;
	total_bytes    += nbBytes;
	size_segments   = (nbBytes + 255) / 255;

	// flush early if adding this packet would make us end up with a
	// continued page which we wouldn't have otherwise
	while (( ((size_segments <= 255) &&
		(last_segments + size_segments > 255)) ||
		(enc_granulepos - last_granulepos > max_ogg_delay)) &&
#ifdef HAVE_OGG_STREAM_FLUSH_FILL
		ogg_stream_flush_fill(&m_os, &m_og, 255 * 255))
#else /* HAVE_OGG_STREAM_FLUSH_FILL */
		ogg_stream_flush(&m_os, &m_og))
#endif /* HAVE_OGG_STREAM_FLUSH_FILL */
	{
	    if (ogg_page_packets(&m_og) != 0)
		last_granulepos = ogg_page_granulepos(&m_og);

	    last_segments -= m_og.header[26];

	    if (!writeOggPage(dst)) {
		qWarning("Opus encoder: I/O error");
		return false;
	    }
	}

        // the downside of early reading is if the input is an exact
        // multiple of the frame_size you'll get an extra frame that needs
        // to get cropped off. The downside of late reading is added delay.
        // If your ogg_delay is 120ms or less we'll assume you want the
        // low delay behavior.
        if ((!m_op.e_o_s ) && (max_ogg_delay > 5760)) {
            nb_samples = fillInBuffer(src);
            total_samples += nb_samples;
            if (nb_samples < m_frame_size) eos = 1;
            if (nb_samples == 0) m_op.e_o_s = 1;
        } else {
            nb_samples = -1;
        }

        m_op.packet     = m_packet_buffer;
        m_op.packetno   = packet_id;
        m_op.bytes      = nbBytes;
        m_op.b_o_s      = 0;
        m_op.granulepos = enc_granulepos;
        if (m_op.e_o_s) {
            // compute the final GP as ceil(len*48k/input_rate). When a
            // resampling decoder does the matching floor(len*input/48k)
            // conversion the length will be exactly the same as the input.
            sample_index_t length = m_info.length();
	    double         rate   = m_info.rate();
	    m_op.granulepos = static_cast<ogg_int64_t>(
		ceil((length * 48000.0) / rate) + m_opus_header.preskip);
        }
        ogg_stream_packetin(&m_os, &m_op);
        last_segments += size_segments;

        // If the stream is over or we're sure that the delayed flush will
        // fire, go ahead and flush now to avoid adding delay.
        while ((m_op.e_o_s || (enc_granulepos +
	        ((m_frame_size * 48000) / m_coding_rate ) -
	         last_granulepos > max_ogg_delay) || (last_segments >= 255)) ?
#ifdef HAVE_OGG_STREAM_FLUSH_FILL
                ogg_stream_flush_fill(&m_os, &m_og, 255 * 255) :
                ogg_stream_pageout_fill(&m_os, &m_og, 255 * 255))
#else /* HAVE_OGG_STREAM_FLUSH_FILL */
                /*Libogg > 1.2.2 allows us to achieve lower overhead by
                  producing larger pages. For 20ms frames this is only relevant
                  above ~32kbit/sec.*/
                ogg_stream_flush(&m_os, &m_og) :
                ogg_stream_pageout(&m_os, &m_og))
#endif /* HAVE_OGG_STREAM_FLUSH_FILL */
	{
            if (ogg_page_packets(&m_og) != 0)
                last_granulepos = ogg_page_granulepos(&m_og);

            last_segments -= m_og.header[26];
	    if (!writeOggPage(dst)) {
		qWarning("Opus encoder: I/O error");
		return false;
	    }
        }
    }

    return true;
}

/***************************************************************************/
void Kwave::OpusEncoder::close()
{
    if (m_channel_mixer) delete m_channel_mixer;
    m_channel_mixer = Q_NULLPTR;

    if (m_rate_converter) delete m_rate_converter;
    m_rate_converter = Q_NULLPTR;

    if (m_buffer) delete m_buffer;
    m_buffer = Q_NULLPTR;

    if (m_encoder) opus_multistream_encoder_destroy(m_encoder);
    m_encoder = Q_NULLPTR;

    ogg_stream_clear(&m_os);

    if (m_packet_buffer) free(m_packet_buffer);
    m_packet_buffer = Q_NULLPTR;

    if (m_encoder_input) free(m_encoder_input);
    m_encoder_input = Q_NULLPTR;

    m_last_queue_element = Q_NULLPTR;
}

/***************************************************************************/
/***************************************************************************/
