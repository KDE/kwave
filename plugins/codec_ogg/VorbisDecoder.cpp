/*************************************************************************
     VorbisDecoder.cpp  -  sub decoder for Vorbis in an Ogg container
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

#include <ogg/ogg.h>
#include <vorbis/codec.h>

#include <QtCore/QDate>
#include <QtCore/QIODevice>
#include <QtCore/QString>

#include <klocale.h>

#include "libkwave/CompressionType.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MultiWriter.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleArray.h"
#include "libkwave/StandardBitrates.h"

#include "VorbisDecoder.h"

/** bitrate to be used when no bitrate has been decoded */
#define DEFAULT_BITRATE 128000

//***************************************************************************
Kwave::VorbisDecoder::VorbisDecoder(QIODevice *source,
                                    ogg_sync_state &oy,
                                    ogg_stream_state &os,
                                    ogg_page& og,
                                    ogg_packet& op)
    :m_source(source), m_stream_start_pos(0), m_samples_written(0),
     m_oy(oy), m_os(os), m_og(og), m_op(op)
{
}


//***************************************************************************
void Kwave::VorbisDecoder::parseTag(Kwave::FileInfo &info, const char *tag,
                                    Kwave::FileProperty property)
{
    int count = vorbis_comment_query_count(&m_vc, const_cast<char *>(tag));
    if (count < 1) return;
    QString value;
    for (int i = 0; i < count; ++i) {
	char *text = vorbis_comment_query(&m_vc, const_cast<char *>(tag), i);
	if (i) value += _("; ");
	value += QString::fromUtf8(text);
    }

    info.set(property, value);
}

//***************************************************************************
int Kwave::VorbisDecoder::open(QWidget *widget, Kwave::FileInfo &info)
{
    // extract the initial header from the first page and verify that the
    // Ogg bitstream is in fact Vorbis data

    // I handle the initial header first instead of just having the code
    // read all three Vorbis headers at once because reading the initial
    // header is an easy way to identify a Vorbis bitstream and it's
    // useful to see that functionality seperated out.
    vorbis_info_init(&m_vi);
    vorbis_comment_init(&m_vc);

    if (vorbis_synthesis_headerin(&m_vi, &m_vc, &m_op) < 0) {
	// error case; not a vorbis header
	Kwave::MessageBox::error(widget, i18n(
	    "This Ogg bitstream does not contain any Vorbis audio data."));
	return -1;
    }

    // At this point, we're sure we're Vorbis.  We've set up the logical
    // (Ogg) bitstream decoder.  Get the comment and codebook headers and
    // set up the Vorbis decoder

    // The next two packets in order are the comment and codebook headers.
    // They're likely large and may span multiple pages.  Thus we read
    // and submit data until we get our two packets, watching that no
    // pages are missing.  If a page is missing, error out; losing a
    // header page is the only place where missing data is fatal. */
    unsigned int counter = 0;
    while (counter < 2) {
	while(counter < 2) {
	    int result = ogg_sync_pageout(&m_oy, &m_og);
	    if (result == 0) break; // Need more data
	    // Don't complain about missing or corrupt data yet.  We'll
	    // catch it at the packet output phase
	    if (result == 1) {
		// we can ignore any errors here
		// as they'll also become apparent
		// at packetout
		ogg_stream_pagein(&m_os, &m_og);
		while (counter < 2) {
		    result = ogg_stream_packetout(&m_os, &m_op);
		    if (result == 0) break;
		    if (result < 0) {
			// Uh oh; data at some point was corrupted or
			// missing! We can't tolerate that in a header.
			// Die.
			Kwave::MessageBox::error(widget, i18n(
			    "Corrupt secondary header. Exiting."));
			return -1;
		    }
		    vorbis_synthesis_headerin(&m_vi, &m_vc, &m_op);
		    counter++;
		}
	    }
	}

	// no harm in not checking before adding more
	char *buffer = ogg_sync_buffer(&m_oy, 4096);
	qint64 bytes = m_source->read(buffer, 4096);
	if (!bytes && counter < 2) {
	    Kwave::MessageBox::error(widget, i18n(
	        "End of file before finding all Vorbis headers."));
	    return -1;
	}
	ogg_sync_wrote(&m_oy, static_cast<long int>(bytes));
    }

    // OK, got and parsed all three headers. Initialize the Vorbis
    // packet->PCM decoder. */
    vorbis_synthesis_init(&m_vd, &m_vi); // central decode state
    vorbis_block_init(&m_vd, &m_vb); // local state for most of the decode
                                     // so multiple block decodes can
                                     // proceed in parallel.  We could init
                                     // multiple vorbis_block structures
                                     // for m_vd here

    // get the standard properties
    info.setTracks(m_vi.channels);
    info.setRate(m_vi.rate);
    info.set(Kwave::INF_COMPRESSION, Kwave::CompressionType::OGG_VORBIS);
    info.set(Kwave::INF_SOURCE, _(m_vc.vendor));
    if (m_vi.bitrate_nominal > 0)
	info.set(Kwave::INF_BITRATE_NOMINAL, QVariant(
	static_cast<int>(m_vi.bitrate_nominal)));
    if (m_vi.bitrate_lower > 0)
	info.set(Kwave::INF_BITRATE_LOWER, QVariant(
	static_cast<int>(m_vi.bitrate_lower)));
    if (m_vi.bitrate_upper > 0)
	info.set(Kwave::INF_BITRATE_UPPER, QVariant(
	static_cast<int>(m_vi.bitrate_upper)));

    // the first comment sometimes is used for the software version
    {
	char **ptr = m_vc.user_comments;
	QString s = _(*ptr);
	if (s.length() && !s.contains(QLatin1Char('='))) {
	    info.set(Kwave::INF_SOFTWARE, s);
	    qDebug("Bitstream is %d channel, %ldHz", m_vi.channels, m_vi.rate);
	    qDebug("Encoded by: %s\n\n", m_vc.vendor);
	}
    }

    /** convert the date property to a QDate */
    parseTag(info, "DATE",         Kwave::INF_CREATION_DATE);
    if (info.contains(Kwave::INF_CREATION_DATE)) {
	QString str_date  = QVariant(info.get(
	    Kwave::INF_CREATION_DATE)).toString();
	QDate date;
	date = QDate::fromString(str_date, Qt::ISODate);
	if (!date.isValid()) {
	    int year = str_date.toInt();
	    date.setYMD(year, 1, 1);
	}
	if (date.isValid()) info.set(Kwave::INF_CREATION_DATE, date);
    }

    // parse all other (simple) properties
    parseTag(info, "TITLE",        Kwave::INF_NAME);
    parseTag(info, "VERSION",      Kwave::INF_VERSION);
    parseTag(info, "ALBUM",        Kwave::INF_ALBUM);
    parseTag(info, "TRACKNUMBER",  Kwave::INF_TRACK);
    parseTag(info, "ARTIST",       Kwave::INF_AUTHOR);
    parseTag(info, "PERFORMER",    Kwave::INF_PERFORMER);
    parseTag(info, "COPYRIGHT",    Kwave::INF_COPYRIGHT);
    parseTag(info, "LICENSE",      Kwave::INF_LICENSE);
    parseTag(info, "ORGANIZATION", Kwave::INF_ORGANIZATION);
    parseTag(info, "DESCRIPTION",  Kwave::INF_SUBJECT);
    parseTag(info, "GENRE",        Kwave::INF_GENRE);
    parseTag(info, "LOCATION",     Kwave::INF_SOURCE);
    parseTag(info, "CONTACT",      Kwave::INF_CONTACT);
    parseTag(info, "ISRC",         Kwave::INF_ISRC);
    parseTag(info, "ENCODER",      Kwave::INF_SOFTWARE);
    parseTag(info, "VBR_QUALITY",  Kwave::INF_VBR_QUALITY);

    m_stream_start_pos = m_source->pos();

    return 1;
}

//***************************************************************************
static inline int decodeFrame(float **pcm, unsigned int size,
                              Kwave::MultiWriter &dest)
{
//     bool clipped = false;
    unsigned int track;
    unsigned int tracks = dest.tracks();

    // convert floats to 16 bit signed ints
    // (host order) and interleave
    for (track = 0; track < tracks; track++) {
	float *mono = pcm[track];
	int bout = size;
	unsigned int ofs = 0;
	Kwave::SampleArray buffer(size);

	while (bout--) {
	    // scale, use some primitive noise shaping
	    sample_t s = static_cast<sample_t>(
		*(mono++) *
		static_cast<float>(SAMPLE_MAX) + drand48() - 0.5f);

	    // might as well guard against clipping
	    if (s > SAMPLE_MAX) {
		s = SAMPLE_MAX;
// 		clipped = true;
	    }
	    if (s < SAMPLE_MIN) {
		s = SAMPLE_MIN;
// 		clipped = true;
	    }

	    // write the clipped sample to the stream
	    buffer[ofs++] = s;
	}

	// write the buffer to the stream
	*(dest[track]) << buffer;
    }

//    if (clipped) qDebug("Clipping in frame %ld", (long)(m_vd.sequence));

    return size;
}

//***************************************************************************
int Kwave::VorbisDecoder::decode(Kwave::MultiWriter &dst)
{
    // we have a packet.  Decode it
    float **pcm;
    int samples;

    // test for success!
    if (vorbis_synthesis(&m_vb, &m_op) == 0)
	vorbis_synthesis_blockin(&m_vd, &m_vb);

    // **pcm is a multichannel float vector. In stereo, for example,
    // pcm[0] is left, and pcm[1] is right.  samples is the size of
    // each channel.  Convert the float values (-1.<=range<=1.) to
    // whatever PCM format and write it out
    while ((samples = vorbis_synthesis_pcmout(&m_vd, &pcm)) > 0)
    {
	register int bout = decodeFrame(pcm, samples, dst);

	// tell libvorbis how many samples we
	// actually consumed
	vorbis_synthesis_read(&m_vd, bout);
    }

    m_samples_written = dst.last();
    return 0;
}

//***************************************************************************
void Kwave::VorbisDecoder::reset()
{
    // ogg_page and ogg_packet structs always point to storage in
    // libvorbis.  They're never freed or manipulated directly

    vorbis_block_clear(&m_vb);
    vorbis_dsp_clear(&m_vd);
    vorbis_comment_clear(&m_vc);
    vorbis_info_clear(&m_vi);  // must be called last
}

//***************************************************************************
void Kwave::VorbisDecoder::close(Kwave::FileInfo &info)
{
    if (!info.contains(Kwave::INF_BITRATE_NOMINAL) &&
        !info.contains(Kwave::INF_VBR_QUALITY))
    {
	qWarning("file contains neither nominal bitrate (ABR mode) "
	         "nor quality (VBR mode)");

	int bitrate = DEFAULT_BITRATE;

	if (static_cast<int>(info.rate()) && m_samples_written) {
	    // guess bitrates from the stream
	    const qint64 stream_end_pos = m_source->pos();
	    const qint64 stream_read = stream_end_pos -
	                                     m_stream_start_pos + 1;
	    double bits = static_cast<double>(stream_read) * 8.0;
	    double seconds = static_cast<double>(m_samples_written) /
		static_cast<double>(info.rate());
	    bitrate = static_cast<unsigned int>(bits / seconds);

	    // round to nearest standard bitrate
	    bitrate = Kwave::StandardBitrates::instance().nearest(bitrate);
	    qDebug("-> using guessed bitrate %d bits/sec", bitrate);
	} else {
	    // guessing not possible -> use default
	    qDebug("-> using default %d kBits/sec", bitrate);
	}

	info.set(Kwave::INF_BITRATE_NOMINAL, QVariant(static_cast<int>(bitrate)));
    }
}


//***************************************************************************
//***************************************************************************
