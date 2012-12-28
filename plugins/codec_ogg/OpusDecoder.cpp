/*************************************************************************
        OpusDecoder.cpp  -  sub decoder for Opus in an Ogg container
                             -------------------
    begin                : Wed Dec 26 2012
    copyright            : (C) 2012 by Thomas Eschenbacher
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

#include <opus/opus_defines.h>

#include <QtCore/QDate>
#include <QtCore/qendian.h>
#include <QtCore/QIODevice>
#include <QtCore/QString>

#include <klocale.h>

#include "libkwave/CompressionType.h"
#include "libkwave/Connect.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MultiStreamWriter.h"
#include "libkwave/MultiTrackSource.h"
#include "libkwave/MultiWriter.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleArray.h"
#include "libkwave/StandardBitrates.h"
#include "libkwave/modules/RateConverter.h"

#include "OpusDecoder.h"

/** maximum frame size in samples, 120ms at 48000 */
#define MAX_FRAME_SIZE (960 * 6)

//***************************************************************************
/**
 * round up to the next supported sample rate
 * @param rate arbitrary sample rate
 * @return next supported rate
 */
static int _opus_next_sample_rate(int rate)
{
    if (rate < 8000)
	return 8000;
    else if (rate <= 12000)
	return 12000;
    else if (rate <= 16000)
	return 16000;
    else if (rate <= 24000)
	return 24000;
    else
	return 48000;
}

//***************************************************************************
Kwave::OpusDecoder::OpusDecoder(QIODevice *source,
                                ogg_sync_state &oy,
                                ogg_stream_state &os,
                                ogg_page &og,
                                ogg_packet &op)
    :m_source(source), m_stream_start_pos(0), m_samples_written(0),
     m_oy(oy), m_os(os), m_og(og), m_op(op),
     m_comments_map(), m_buffer(0)
#ifdef HAVE_SAMPLERATE_SUPPORT
    ,m_adapter(0), m_rate_converter(0), m_converter_connected(false)
#endif /* HAVE_SAMPLERATE_SUPPORT */
{
}

//***************************************************************************
void Kwave::OpusDecoder::parseComment(Kwave::FileInfo &info,
                                      const QString &comment)
{
    // assume that there is a '=' between tag and value
    int pos = comment.indexOf(_("="));
    if (pos < 1) {
	qWarning("OpusDecoder: malformed comment: '%s'", DBG(comment));
	return;
    }

    // split into tag and value
    QString tag = comment.left(pos).toUpper();
    QString val = comment.mid(pos + 1);

    // check for unknown properties
    if (!m_comments_map.contains(tag)) {
	qDebug("unsupported tag '%s', value='%s'", DBG(tag), DBG(val));
	return;
    }

    // set the new value
    Kwave::FileProperty property = m_comments_map[tag];
    if (info.contains(property)) {
	// property already exists, append it
	val = info.get(property).toString() + _("; ") + val;
    }

    info.set(property, val);
}

//***************************************************************************
int Kwave::OpusDecoder::parseOpusTags(QWidget *widget, Kwave::FileInfo &info)
{
    bool comments_ok = false;
    unsigned int counter = 0;
    while (counter < 1) {
	while(counter < 1) {
	    int result = ogg_sync_pageout(&m_oy, &m_og);
	    if (result == 0) break; // Need more data
	    if (result == 1) {
		ogg_stream_pagein(&m_os, &m_og);
		counter++;
	    }
	}

	// no harm in not checking before adding more
	char *buffer = ogg_sync_buffer(&m_oy, 4096);
	qint64 bytes = m_source->read(buffer, 4096);
	if (!bytes && counter < 1) {
	    Kwave::MessageBox::error(widget, i18n(
	        "End of file before finding Opus Comment headers."));
	    return -1;
	}
	ogg_sync_wrote(&m_oy, static_cast<long int>(bytes));
    }

    unsigned int packet = 0;
    unsigned int fields = 0;
    while (ogg_stream_packetout(&m_os, &m_op) == 1) {
	const unsigned char *c   = m_op.packet;
	unsigned long int length = m_op.bytes;

	// check lenght of comments and magic value
	if (length < 16) {
	    qWarning("OpusDecoder::parseHeader(): comment lenght < 16 (%lu)",
	              length);
	    break;
	}
	if (memcmp(c, "OpusTags", 8) != 0) {
	    qWarning("OpusDecoder::parseHeader(): OpusTags magic not found");
	    break;
	}
	c      += 8;
	length -= 8;

	if (packet == 0) {
	    // the start of the first packet contains the software

	    // read length of the comment
	    quint32 len = qFromLittleEndian<quint32>(c);
	    c      += 4;
	    length -= 4;
	    if (len > length) {
		// comment extends beyond end of packet
		qWarning("OpusDecoder::parseHeader(): encoder name truncated "
		         "(len=%u, max=%lu)", len, length);
		len = length;
	    }
	    QString software =
		QString::fromUtf8(reinterpret_cast<const char *>(c), len);
	    c      += len;
	    length -= len;
	    qDebug("    Encoded with '%s'", DBG(software));
	    info.set(Kwave::INF_SOFTWARE, software);

	    if (length < 4) {
		qWarning("OpusDecoder::parseHeader(): tag is too short (%lu)",
		         length);
		break;
	    }
	    fields = qFromLittleEndian<quint32>(c);
	    c      += 4;
	    length -= 4;
	}

	while (fields && (length > 4)) {
	    if (length < 4) {
		qWarning("OpusDecoder::parseHeader(): broken comment (%lu)",
			    length);
		break;
	    }
	    quint32 len = qFromLittleEndian<quint32>(c);
	    c      += 4;
	    length -= 4;
	    if (len > length) {
		// comment extends beyond end of packet
		qWarning("OpusDecoder::parseHeader(): comment truncated "
		         "(len=%u, max=%lu)", len, length);
		len = length;
	    }
	    QString comment =
		QString::fromUtf8(reinterpret_cast<const char *>(c), len);
	    c      += len;
	    length -= len;

	    parseComment(info, comment);

	    fields--;
	}

	comments_ok = (fields == 0);
	break;
    }

    if (!comments_ok) {
	qDebug("OpusDecoder: WARNING: no comment block found!?");
    }

    return 1;
}

//***************************************************************************
int Kwave::OpusDecoder::parseOpusHead(QWidget *widget, Kwave::FileInfo &info)
{
    memset(&m_opus_header, 0x00, sizeof(m_opus_header));
    memset(&m_opus_header.map, 0xFF, sizeof(m_opus_header.map));

    bool header_ok = false;
    do {
	if (!m_op.b_o_s || (m_op.bytes < 19)) {
	    qWarning("OpusDecoder::parseHeader(): header too short");
	    break;
	}

	Kwave::opus_header_t *h =
	    reinterpret_cast<Kwave::opus_header_t *>(m_op.packet);

	// magic value
	memcpy(&(m_opus_header.magic[0]), &(h->magic[0]),
	       sizeof(m_opus_header.magic));
	if (memcmp(&(m_opus_header.magic[0]), "OpusHead", 8) != 0) {
	    qWarning("OpusDecoder::parseHeader(): OpusHead magic not found");
	    break; // this is no Opus stream ?
	}

	// version number, only major version 0 is supported
	m_opus_header.version = h->version;
	if ((m_opus_header.version >> 6) != 0) {
	    qWarning("OpusDecoder::parseHeader(): unsupported version %d.%d",
		(m_opus_header.version >> 6), (m_opus_header.version & 0x3F));
	    break; // unsupported version
	}

	// number of channels
	m_opus_header.channels = h->channels;
	if (m_opus_header.channels < 1) {
	    qWarning("OpusDecoder::parseHeader(): channels==0");
	    break; // no channels?
	}

	// preskip
	m_opus_header.preskip = qFromLittleEndian<quint16>(h->preskip);

	// sample rate
	m_opus_header.sample_rate = qFromLittleEndian<quint32>(h->sample_rate);

	// for debugging rate conversion issues:
// 	m_opus_header.sample_rate =
// 	    _opus_next_sample_rate(m_opus_header.sample_rate);

#ifndef HAVE_SAMPLERATE_SUPPORT
	int rate_orig = m_opus_header.sample_rate;
	int rate_supp = _opus_next_sample_rate(rate_orig);
	if (rate_orig != rate_supp) {
	    qWarning("    OpusDecoder::parseHeader(): sample rate %d is not "
	             "supported but rate conversion is disabled "
	             "-> setting to %d", rate_orig, rate_supp);
	    m_opus_header.sample_rate = rate_supp;
	}
#endif /* HAVE_SAMPLERATE_SUPPORT */

	// gain
	m_opus_header.gain = qFromLittleEndian<quint16>(h->gain);

	// channel mapping
	m_opus_header.channel_mapping = h->channel_mapping;

	// multi stream support
	if (m_opus_header.channel_mapping) {
	    // number of streams, must be >= 1
	    m_opus_header.streams = h->streams;
	    if (m_opus_header.streams < 1) {
		qWarning("OpusDecoder::parseHeader(): streams==0");
		break;
	    }

	    // number of coupled
	    m_opus_header.coupled = h->coupled;
	    if (m_opus_header.coupled > m_opus_header.streams) {
		qWarning("OpusDecoder::parseHeader(): coupled=%d > %d",
		    m_opus_header.coupled, m_opus_header.streams);
		break; // must be <= number of streams
	    }
	    if ((m_opus_header.coupled + m_opus_header.streams) >= 256) {
		qWarning("OpusDecoder::parseHeader(): "
		          "coupled + streams = %d (> 256)",
		    m_opus_header.coupled + m_opus_header.streams);
		break; // must be less that 256
	    }

	    // coupling map
	    unsigned int i;
	    for (i = 0; i < m_opus_header.channels; i++) {
		quint8 c = h->map[i];
		if (c > (m_opus_header.coupled + m_opus_header.streams)) {
		    qWarning("OpusDecoder::parseHeader(): mapping[%d]"
		             "out of range: %d (> %d)", i, c,
			     m_opus_header.coupled + m_opus_header.streams);
		    break; // mapping out of range
		}
		if (m_opus_header.map[i] != 0xFF) {
		    qWarning("OpusDecoder::parseHeader(): mapping[%d]"
		             "already occupied: %d", i,
			     m_opus_header.map[i]);
		    break; // mapping already occupied
		}

		m_opus_header.map[i] = c;
	    }
	    if (i < m_opus_header.channels) break; // something went wrong
	} else {
	    if (m_opus_header.channels > 2) {
		qWarning("OpusDecoder::parseHeader(): channels > 2"
		         "(%d) but no mapping", m_opus_header.channels);
		break;
	    }
	    m_opus_header.streams = 1;
	    m_opus_header.coupled = (m_opus_header.channels > 1) ? 1 : 0;
	    m_opus_header.map[0] = 0;
	    m_opus_header.map[1] = 1;
	}

	header_ok = true;
    } while (false);

    if (!header_ok) {
	// error case; not an Opus header
	Kwave::MessageBox::error(widget, i18n(
	    "This Ogg bitstream does not contain valid Opus audio data."));
	return -1;
    }

    // get the standard properties
    info.setTracks(m_opus_header.channels);
    info.setRate(m_opus_header.sample_rate);
    info.set(Kwave::INF_COMPRESSION, Kwave::CompressionType::OGG_OPUS);

    return 1;
}

//***************************************************************************
static QString _opus_error(int err)
{
    QString msg;

    switch (err)
    {
	/** No error @hideinitializer*/
	case OPUS_OK:
	    msg =QString();
	    break;
	case OPUS_BAD_ARG:
	    msg = i18n("One or more invalid/out of range arguments.");
	    break;
	case OPUS_BUFFER_TOO_SMALL:
	    msg = i18n("The mode struct passed is invalid.");
	    break;
	case OPUS_INTERNAL_ERROR:
	    msg = i18n("An internal error was detected.");
	    break;
	case OPUS_INVALID_PACKET:
	    msg = i18n("The compressed data passed is corrupted.");
	    break;
	case OPUS_UNIMPLEMENTED:
	    msg = i18n("Invalid/unsupported request number.");
	    break;
	case OPUS_INVALID_STATE:
	    msg = i18n("A decoder structure is invalid or already freed.");
	    break;
	case OPUS_ALLOC_FAIL:
	    msg = i18n("Memory allocation has failed.");
	    break;
	default:
	    msg = i18n("Decoder error: %1", _(opus_strerror(err)));
	    break;
    }
    return msg;
}

//***************************************************************************
int Kwave::OpusDecoder::open(QWidget *widget, Kwave::FileInfo &info)
{
    // extract the initial header from the first page and verify that the
    // Ogg bitstream is in fact valid/usable Opus data
    if (parseOpusHead(widget, info) < 1)
	return -1;

    // extract the second packet, it should contain the comments...
    if (parseOpusTags(widget, info) < 1)
	return -1;

    // allocate memory for the output data
    if (m_buffer) free(m_buffer);
    m_buffer = static_cast<float *>(
	malloc(sizeof(float) * MAX_FRAME_SIZE * m_opus_header.channels));
    if (!m_buffer) {
	Kwave::MessageBox::error(widget, i18n("Out of memory"));
	return -1;
    }

    int err = OPUS_BAD_ARG;
    qDebug("    sample rate = %d", m_opus_header.sample_rate);
    m_opus_decoder = opus_multistream_decoder_create(
        _opus_next_sample_rate(m_opus_header.sample_rate),
        m_opus_header.channels,
        m_opus_header.streams,
        m_opus_header.coupled,
        m_opus_header.map,
        &err
    );

    if ((err != OPUS_OK) || !m_opus_decoder) {
	info.dump();
	Kwave::MessageBox::error(widget, _opus_error(err),
	     i18n("Opus decoder failed"));
	return -1;
    }

#ifdef OPUS_SET_GAIN
    if (m_opus_header.gain) {
	err = opus_multistream_decoder_ctl(
	    m_opus_decoder, OPUS_SET_GAIN(m_opus_header.gain)
	);
	if (err == OPUS_OK) {
	    qDebug("    OpusDecoder: gain adjusted to %d Q8dB",
	           m_opus_header.gain);
	    m_opus_header.gain = 0;
	}
    }
#endif /* OPUS_SET_GAIN */

    const unsigned int tracks = m_opus_header.channels;
    int rate_orig = m_opus_header.sample_rate;
    int rate_supp = _opus_next_sample_rate(rate_orig);

    // handle sample rate conversion
    if (rate_orig != rate_supp) {
	bool ok = false;

#ifdef HAVE_SAMPLERATE_SUPPORT
	qDebug("    OpusDecoder::open(): converting sample rate: %d -> %d",
	       rate_supp, rate_orig);

	ok = true;
	m_adapter = new Kwave::MultiStreamWriter(tracks);
	Q_ASSERT(m_adapter);
	if (!m_adapter) ok = false;

	if (ok) {
	    m_rate_converter =
		new Kwave::MultiTrackSource<Kwave::RateConverter, true>(tracks);
	    Q_ASSERT(m_rate_converter);
	    if (!m_rate_converter) ok = false;
	}

	if (ok) {
	    double rate_from = static_cast<double>(rate_supp);
	    double rate_to   = static_cast<double>(rate_orig);
	    m_rate_converter->setAttribute(
		SLOT(setRatio(const QVariant)),
		QVariant(rate_to / rate_from)
	    );
	}

	if (ok) {
	    ok = Kwave::connect(
		*m_adapter,        SIGNAL(output(Kwave::SampleArray)),
		*m_rate_converter, SLOT(input(Kwave::SampleArray))
	    );
	}

	if (!ok) {
	    qWarning("OpusDecoder::open(): creating rate converter failed!");
	}
#endif /* HAVE_SAMPLERATE_SUPPORT */

	if (!ok) {
	    qDebug("OpusDecoder::open(): sample rate %d is not "
		   "supported but rate conversion is not available "
		   "-> setting to %d", rate_orig, rate_supp);
	    m_opus_header.sample_rate = rate_supp;
	}
    }

    m_stream_start_pos = m_source->pos();
    m_samples_written  = 0;
    return 1;
}

//***************************************************************************
int Kwave::OpusDecoder::decode(Kwave::MultiWriter &dst)
{
    if (!m_opus_decoder || !m_buffer)
	return -1;

    // decode the audio data into a buffer with float values
    int length = opus_multistream_decode_float(
	m_opus_decoder,
	static_cast<unsigned char *>(m_op.packet),
	m_op.bytes, m_buffer, MAX_FRAME_SIZE, 0
    );
    if (length <= 0) {
	qWarning("OpusDecoder::decode() failed: '%s'",
	         DBG(_opus_error(length)));
	return -1;
    }

    // manually apply the gain if necessary
    if (m_opus_header.gain) {
	const float g = pow(10.0, m_opus_header.gain / (20.0 * 256.0));
	for (int i = 0; i < (length * m_opus_header.channels); i++)
	    m_buffer[i] *= g;
    }

    Kwave::MultiWriter *sink = &dst;
    const unsigned int tracks = m_opus_header.channels;

#ifdef HAVE_SAMPLERATE_SUPPORT
    if (m_adapter && m_rate_converter) {
	if (!m_converter_connected) {
	    bool ok = Kwave::connect(
		*m_rate_converter, SIGNAL(output(Kwave::SampleArray)),
		dst,               SLOT(input(Kwave::SampleArray)));
	    if (!ok) {
		qWarning("OpusDecoder::decode() connecting converter failed");
		return -1;
	    }

	    qDebug("    opusDecoder::decode(): rate converter connected");
	    m_converter_connected = true;
	}
	sink = m_adapter;
    }
#endif /* HAVE_SAMPLERATE_SUPPORT */

    // convert the buffer from float to sample_t, blockwise...
    float *p = m_buffer;
    Kwave::SampleArray samples(length);
    for (unsigned int t = 0; t < tracks; t++) {
	float    *in  = p + t;
	sample_t *out = samples.data();
	for (int frame = 0; frame < length; frame++) {
	    // scale, use some primitive noise shaping
	    sample_t s = static_cast<sample_t>(
		(*in) * static_cast<float>(SAMPLE_MAX) + drand48() - 0.5f);
	    if (s > SAMPLE_MAX) s = SAMPLE_MAX;
	    if (s < SAMPLE_MIN) s = SAMPLE_MIN;

	    *(out++) = s;
	    in += tracks;
	}
	*((*sink)[t]) << samples;
    }

    m_samples_written = dst.last();
    return 0;
}

//***************************************************************************
void Kwave::OpusDecoder::reset()
{
    if (m_opus_decoder)
	opus_multistream_decoder_destroy(m_opus_decoder);
    m_opus_decoder = 0;

    if (m_buffer)
	free(m_buffer);
    m_buffer = 0;

#ifdef HAVE_SAMPLERATE_SUPPORT
    if (m_adapter) {
	m_adapter->flush();
	delete m_adapter;
	m_adapter = 0;
    }

    delete m_rate_converter;
    m_rate_converter = 0;

    m_converter_connected = false;
#endif /* HAVE_SAMPLERATE_SUPPORT */
}

//***************************************************************************
void Kwave::OpusDecoder::close(Kwave::FileInfo &info)
{
    Q_UNUSED(info);
    reset();
}


//***************************************************************************
//***************************************************************************
