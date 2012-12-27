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

#include <stdlib.h>

#include <QtCore/QDate>
#include <QtCore/qendian.h>
#include <QtCore/QIODevice>
#include <QtCore/QString>

#include <klocale.h>

#include "libkwave/CompressionType.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MultiWriter.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleArray.h"
#include "libkwave/StandardBitrates.h"

#include "OpusDecoder.h"

/** bitrate to be used when no bitrate has been decoded */
#define DEFAULT_BITRATE 128000

//***************************************************************************
Kwave::OpusDecoder::OpusDecoder(QIODevice *source,
                                    ogg_sync_state &oy,
                                    ogg_stream_state &os,
                                    ogg_page& og,
                                    ogg_packet& op)
    :m_source(source), m_stream_start_pos(0), m_samples_written(0),
     m_oy(oy), m_os(os), m_og(og), m_op(op),
     m_comments_map()
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
int Kwave::OpusDecoder::open(QWidget *widget, Kwave::FileInfo &info)
{
    // extract the initial header from the first page and verify that the
    // Ogg bitstream is in fact valid/usable Opus data
    if (parseOpusHead(widget, info) < 1)
	return -1;

    // extract the second packet, it should contain the comments...
    if (parseOpusTags(widget, info) < 1)
	return -1;

    m_stream_start_pos = m_source->pos();
    return 1;
}

//***************************************************************************
int Kwave::OpusDecoder::decode(Kwave::MultiWriter &dst)
{
    // TODO
    m_samples_written = dst.last();
    return 0;
}

//***************************************************************************
void Kwave::OpusDecoder::reset()
{
    // TODO
}

//***************************************************************************
void Kwave::OpusDecoder::close(Kwave::FileInfo &info)
{
    Q_UNUSED(info);
    // TODO
}


//***************************************************************************
//***************************************************************************
