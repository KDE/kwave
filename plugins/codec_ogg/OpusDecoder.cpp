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

#include <limits>
#include <math.h>
#include <stdlib.h>

#include <opus/opus_defines.h>

#include <QtGui/QApplication>
#include <QtCore/QDate>
#include <QtCore/qendian.h>
#include <QtCore/QIODevice>
#include <QtCore/QString>

#include <klocale.h>

#include "libkwave/BitrateMode.h"
#include "libkwave/Compression.h"
#include "libkwave/Connect.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MultiStreamWriter.h"
#include "libkwave/MultiTrackSource.h"
#include "libkwave/MultiWriter.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleArray.h"
#include "libkwave/StandardBitrates.h"
#include "libkwave/Writer.h"
#include "libkwave/modules/RateConverter.h"

#include "OpusCommon.h"
#include "OpusDecoder.h"

/** maximum frame size in samples, 120ms at 48000 */
#define MAX_FRAME_SIZE (960 * 6)

//***************************************************************************
Kwave::OpusDecoder::OpusDecoder(QIODevice *source,
                                ogg_sync_state &oy,
                                ogg_stream_state &os,
                                ogg_page &og,
                                ogg_packet &op)
    :m_source(source), m_stream_start_pos(0), m_samples_written(0),
     m_oy(oy), m_os(os), m_og(og), m_op(op),
     m_comments_map(), m_raw_buffer(0), m_buffer(0),
     m_rate_converter(0),
     m_converter_connected(false),
     m_packet_count(0), m_samples_raw(0), m_bytes_count(0),
     m_packet_len_min(0), m_packet_len_max(0),
     m_packet_size_min(0), m_packet_size_max(0),
     m_granule_first(0), m_granule_last(0), m_granule_offset(0),
     m_preskip(0)
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
    QString val = comment.mid(pos + 1).trimmed();

    // special handling for gain
    if ((tag == _("REPLAY_TRACK_GAIN")) || (tag == _("REPLAY_ALBUM_GAIN"))) {
	// gain in dB:
	// remove "dB" from the end
	val = val.left(val.indexOf(_("dB"), 0, Qt::CaseInsensitive)).trimmed();

	// convert to a integer Q8 dB value
	bool ok = false;
	int q8gain = static_cast<int>(rint(val.toDouble(&ok) * 256.0));
	if (ok && q8gain) {
	    m_opus_header.gain += q8gain;
	    qDebug("    OpusDecoder: %s %+0.1g dB", DBG(tag),
	           static_cast<double>(q8gain) / 256.0);
	    return;
	}
    } else if ((tag == _("R128_TRACK_GAIN")) || (tag == _("R128_ALBUM_GAIN"))) {
	// R128_... already is a 7.8 integer value
	bool ok = false;
	int q8gain = rint(val.toInt(&ok));
	if (ok && q8gain) {
	    m_opus_header.gain += q8gain;
	    qDebug("    OpusDecoder: %s %+0.1g dB", DBG(tag),
	           static_cast<double>(q8gain) / 256.0);
	    return;
	}
    }

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
	    QString encoder =
		QString::fromUtf8(reinterpret_cast<const char *>(c), len);
	    c      += len;
	    length -= len;
	    qDebug("    Encoded with '%s'", DBG(encoder));
	    /* info.set(Kwave::INF_SOFTWARE, software); */

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
    info.set(Kwave::INF_COMPRESSION, Kwave::Compression::OGG_OPUS);

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

    // allocate memory for the output data
    if (m_raw_buffer) free(m_raw_buffer);
    m_raw_buffer = static_cast<float *>(
	malloc(sizeof(float) * MAX_FRAME_SIZE * m_opus_header.channels));
    if (!m_raw_buffer) {
	Kwave::MessageBox::error(widget, i18n("Out of memory"));
	return -1;
    }

    int err = OPUS_BAD_ARG;
    qDebug("    sample rate = %d", m_opus_header.sample_rate);
    m_opus_decoder = opus_multistream_decoder_create(
        Kwave::opus_next_sample_rate(m_opus_header.sample_rate),
        m_opus_header.channels,
        m_opus_header.streams,
        m_opus_header.coupled,
        m_opus_header.map,
        &err
    );

    if ((err != OPUS_OK) || !m_opus_decoder) {
	info.dump();
	Kwave::MessageBox::error(widget, Kwave::opus_error(err),
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
    int rate_supp = Kwave::opus_next_sample_rate(rate_orig);

    // create a multi track sample buffer
    m_buffer = new Kwave::MultiTrackSink<Kwave::SampleBuffer, true>(tracks);
    Q_ASSERT(m_buffer);
    if (!m_buffer) return -1;

    // handle sample rate conversion
    if (rate_orig != rate_supp) {
	bool ok = false;

	qDebug("    OpusDecoder::open(): converting sample rate: %d -> %d",
	       rate_supp, rate_orig);
	ok = true;

	m_rate_converter =
	    new Kwave::MultiTrackSource<Kwave::RateConverter, true>(tracks);
	Q_ASSERT(m_rate_converter);
	if (!m_rate_converter) ok = false;

	if (ok) {
	    double rate_from = static_cast<double>(rate_supp);
	    double rate_to   = static_cast<double>(rate_orig);
	    m_rate_converter->setAttribute(
		SLOT(setRatio(const QVariant)),
		QVariant(rate_to / rate_from)
	    );

	    ok = Kwave::connect(
		*m_buffer,         SIGNAL(output(Kwave::SampleArray)),
		*m_rate_converter, SLOT(input(Kwave::SampleArray))
	    );
	}

	if (!ok) {
	    qWarning("OpusDecoder::open(): creating rate converter failed!");
	}

	if (!ok) {
	    qDebug("OpusDecoder::open(): sample rate %d is not "
		   "supported but rate conversion is not available "
		   "-> setting to %d", rate_orig, rate_supp);
	    m_opus_header.sample_rate = rate_supp;
	}
    }

    // estimate the length of the file from file size, bitrate, channels
    if (!m_source->isSequential()) {
	qint64 file_size       = m_source->size();
	qreal bitrate          = 196000; // just guessed
	qreal rate             = rate_orig;
	qreal seconds          = file_size / (bitrate / 8);
	sample_index_t samples = seconds * rate;

	qDebug("    OpusDecoder: estimated length: %llu samples", samples);
	info.set(Kwave::INF_ESTIMATED_LENGTH, samples);
    }

    m_stream_start_pos = m_source->pos();
    m_samples_written  = 0;
    m_packet_count     = 0;
    m_samples_raw      = 0;
    m_bytes_count      = 0;

    m_packet_len_min  = std::numeric_limits<int>::max();
    m_packet_len_max  = 0;
    m_packet_size_min = std::numeric_limits<int>::max();
    m_packet_size_max = 0;

    m_preskip         = m_opus_header.preskip;
    m_granule_first   = std::numeric_limits<qint64>::max();
    m_granule_last    = 0;
    m_granule_offset  = 0;

    return 1;
}

//***************************************************************************
int Kwave::OpusDecoder::decode(Kwave::MultiWriter &dst)
{
    if (!m_opus_decoder || !m_raw_buffer || !m_buffer)
	return -1;

    // get some statistical data, for bitrate mode detection
    m_packet_count++;

    int frames = opus_packet_get_nb_frames(m_op.packet, m_op.bytes);
    if(frames < 1 || frames > 48) {
	qWarning("WARNING: Invalid packet TOC in packet #%u",
	         static_cast<unsigned int>(m_op.packetno));
    }
    int spf = opus_packet_get_samples_per_frame(m_op.packet, 48000);
    int spp = frames * spf;
    if (spp < 120 || spp > 5760 || (spp % 120) != 0) {
	qWarning("WARNING: Invalid packet TOC in packet #%u",
	         static_cast<unsigned int>(m_op.packetno));
    }

    if (spp < m_packet_len_min) m_packet_len_min = spp;
    if (spp > m_packet_len_max) m_packet_len_max = spp;
    if (m_op.bytes < m_packet_size_min) m_packet_size_min = m_op.bytes;
    if (m_op.bytes > m_packet_size_max) m_packet_size_max = m_op.bytes;

    // total count of samples and bytes
    m_samples_raw += spp;
    m_bytes_count += m_op.bytes;

    // granule pos handling
    const qint64 gp = static_cast<qint64>(ogg_page_granulepos(&m_og));
    if (gp >= 0) {
	if (gp < m_granule_first) m_granule_first = gp;
	if (gp > m_granule_last)  m_granule_last  = gp;
	if (m_granule_first == m_granule_last) {
	    // calculate how many samples might be missing at the start
	    m_granule_offset = m_granule_first - m_samples_raw;
	}
    }

    // decode the audio data into a buffer with float values
    int length = opus_multistream_decode_float(
	m_opus_decoder,
	static_cast<unsigned char *>(m_op.packet),
	m_op.bytes, m_raw_buffer, MAX_FRAME_SIZE, 0
    );
    if (length <= 0) {
	qWarning("OpusDecoder::decode() failed: '%s'",
	         DBG(Kwave::opus_error(length)));
	return -1;
    }
    Q_ASSERT(length <= MAX_FRAME_SIZE);

    // manually apply the gain if necessary
    if (m_opus_header.gain) {
	const float g = pow(10.0, m_opus_header.gain / (20.0 * 256.0));
	for (int i = 0; i < (length * m_opus_header.channels); i++)
	    m_raw_buffer[i] *= g;
    }

    const unsigned int tracks = m_opus_header.channels;

    bool ok = true;
    if (m_rate_converter) {
	if (!m_converter_connected) {
	    ok = Kwave::connect(
		*m_rate_converter, SIGNAL(output(Kwave::SampleArray)),
		dst,               SLOT(input(Kwave::SampleArray)));
	    m_converter_connected = true;
	}
    } else {
	ok = Kwave::connect(
	    *m_buffer, SIGNAL(output(Kwave::SampleArray)),
	    dst,       SLOT(input(Kwave::SampleArray))
	);
    }
    if (!ok) {
	qWarning("OpusDecoder::decode() connecting converter failed");
	return -1;
    }

    // handle preskip
    float *p = m_raw_buffer;
    if (m_preskip) {
	if (m_preskip >= length) {
	    m_preskip -= length;
	    return 0; // skip the complete buffer
	}
	// shrink buffer by preskip samples
	length    -= m_preskip;
	p         += m_preskip * tracks;
	m_preskip  = 0;
    }

    // check trailing data at the end (after the granule)
    const sample_index_t last = (m_granule_last - m_granule_offset) -
	m_opus_header.preskip;

    if ((m_samples_written + length) > last) {
	int diff = (m_samples_written + length) - last;
	if (diff > length) return 0;
	length -= diff;
    }

    // convert the buffer from float to sample_t, blockwise...
    Kwave::SampleArray samples(length);
    for (unsigned int t = 0; t < tracks; t++) {
	Kwave::SampleBuffer *buffer = m_buffer->at(t);
	float    *in  = p + t;
	for (int frame = 0; frame < length; frame++) {
	    // scale, use some primitive noise shaping
	    sample_t s = static_cast<sample_t>(
		(*in) * static_cast<float>(SAMPLE_MAX) + drand48() - 0.5f);
	    if (s > SAMPLE_MAX) s = SAMPLE_MAX;
	    if (s < SAMPLE_MIN) s = SAMPLE_MIN;

	    buffer->put(s);
	    in += tracks;
	}
    }

    m_samples_written += length;

    // update the progress bar
    QApplication::processEvents();

    return 0;
}

//***************************************************************************
void Kwave::OpusDecoder::reset()
{
    if (m_opus_decoder)
	opus_multistream_decoder_destroy(m_opus_decoder);
    m_opus_decoder = 0;

    if (m_raw_buffer)
	free(m_raw_buffer);
    m_raw_buffer = 0;

}

//***************************************************************************
void Kwave::OpusDecoder::close(Kwave::FileInfo &info)
{
    // flush the buffer of the sample rate converter, to avoid missing
    // samples due to the limitations of libsamplerate
    if (m_buffer) {
	const unsigned int tracks = m_opus_header.channels;
	for (unsigned int t = 0; t < tracks; t++) {
	    Kwave::SampleBuffer *buffer = m_buffer->at(t);
	    Q_ASSERT(buffer);
	    buffer->finished();
	}
    }

    if (m_buffer) delete m_buffer;
    m_buffer = 0;

    delete m_rate_converter;
    m_rate_converter = 0;

    m_converter_connected = false;

    qDebug("    OpusDecoder: packet count=%u", m_packet_count);
    qDebug("    OpusDecoder: packet length: %d...%d samples",
	   m_packet_len_min, m_packet_len_max);
    qDebug("    OpusDecoder: packet size: %d...%d bytes",
	   m_packet_size_min, m_packet_size_max);

    if ( (m_packet_len_min == m_packet_len_max) &&
         (m_packet_size_min == m_packet_size_max) )
    {
	// detected hard CBR mode
	info.set(INF_BITRATE_MODE, Kwave::BITRATE_MODE_CBR_HARD);
	qDebug("    OpusDecoder: hard CBR mode");
    } else {
	// otherwise default to VBR mode
	info.set(INF_BITRATE_MODE, Kwave::BITRATE_MODE_VBR);
	qDebug("    OpusDecoder: VBR mode");
    }

    // determine the avarage frame length in ms
    qreal avg_ms = (static_cast<qreal>(m_samples_raw) /
                    static_cast<qreal>(m_packet_count)) / 48.0;
    qDebug("    OpusDecoder: average frame length: %0.1f ms", avg_ms);
    info.set(INF_OPUS_FRAME_LEN, QVariant(avg_ms));

    // calculate the bitrate == n_bits / n_seconds
    // n_bits = n_bytes * 8
    // n_seconds = n_samples / sample_rate
    // => bitrate = (n_bytes * 8) / (n_samples / sample_rate)
    const double sr = Kwave::opus_next_sample_rate(m_opus_header.sample_rate);
    int bitrate = ((m_bytes_count * 8) * sr) / m_samples_written;
    qDebug("    OpusDecoder: average bitrate: %d bits/sec", bitrate);
    info.set(INF_BITRATE_NOMINAL, QVariant(bitrate));

    reset();
}

//***************************************************************************
//***************************************************************************
