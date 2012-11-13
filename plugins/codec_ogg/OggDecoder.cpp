/*************************************************************************
        OggDecoder.cpp  -  decoder for Ogg/Vorbis data
                             -------------------
    begin                : Tue Sep 10 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
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
#include <math.h>

#include <QDate>

#include <klocale.h>
#include <kmimetype.h>

#include "libkwave/CompressionType.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MultiWriter.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleArray.h"
#include "libkwave/StandardBitrates.h"
#include "libkwave/Writer.h"

#include "OggCodecPlugin.h"
#include "OggDecoder.h"

/** bitrate to be used when no bitrate has been decoded */
#define DEFAULT_BITRATE 128000

//***************************************************************************
OggDecoder::OggDecoder()
    :Kwave::Decoder(), m_source(0)
{
    REGISTER_MIME_TYPES;
    REGISTER_COMPRESSION_TYPES;
}

//***************************************************************************
OggDecoder::~OggDecoder()
{
    if (m_source) close();
}

//***************************************************************************
Kwave::Decoder *OggDecoder::instance()
{
    return new OggDecoder();
}

//***************************************************************************
void OggDecoder::parseTag(Kwave::FileInfo &info, const char *tag,
                          Kwave::FileProperty property)
{
    int count = vorbis_comment_query_count(&m_vc, const_cast<char *>(tag));
    if (count < 1) return;
    QString value;
    for (int i=0; i < count; ++i) {
	char *text = vorbis_comment_query(&m_vc, const_cast<char *>(tag), i);
	if (i) value += "; ";
	value += QString::fromUtf8(text);
    }

    info.set(property, value);
}

//***************************************************************************
int OggDecoder::parseHeader(QWidget *widget)
{
    int counter = 0;

    // grab some data at the head of the stream.  We want the first page
    // (which is guaranteed to be small and only contain the Vorbis
    // stream initial header) We need the first page to get the stream
    // serialno.

    // submit a 4k block to libvorbis' Ogg layer
    m_buffer = ogg_sync_buffer(&m_oy, 4096);
    Q_ASSERT(m_buffer);
    if (!m_buffer) return -1;

    long int bytes = static_cast<long int>(m_source->read(m_buffer, 4096));
    if ((bytes <= 0) && (!m_source->pos())) {
	Kwave::MessageBox::error(widget, i18n(
	    "Ogg bitstream has zero-length."));
	return -1;
    }
    ogg_sync_wrote(&m_oy, bytes);

    // Get the first page.
    if (ogg_sync_pageout(&m_oy, &m_og) != 1) {
	// have we simply run out of data?  If so, we're done.
	if (bytes < 4096) return 0;

	// error case.  Must not be Vorbis data
	Kwave::MessageBox::error(widget, i18n(
	     "Input does not appear to be an Ogg bitstream."));
	return -1;
    }

    // Get the serial number and set up the rest of decode.
    // serialno first; use it to set up a logical stream
    ogg_stream_init(&m_os, ogg_page_serialno(&m_og));

    // extract the initial header from the first page and verify that the
    // Ogg bitstream is in fact Vorbis data

    // I handle the initial header first instead of just having the code
    // read all three Vorbis headers at once because reading the initial
    // header is an easy way to identify a Vorbis bitstream and it's
    // useful to see that functionality seperated out.
    vorbis_info_init(&m_vi);
    vorbis_comment_init(&m_vc);
    if (ogg_stream_pagein(&m_os, &m_og) < 0) {
	// error; stream version mismatch perhaps
	Kwave::MessageBox::error(widget, i18n(
	    "Error reading first page of the Ogg bitstream data."));
	return -1;
    }

    if (ogg_stream_packetout(&m_os, &m_op) != 1) {
	// no page? must not be vorbis
	Kwave::MessageBox::error(widget, i18n(
	    "Error reading initial header packet."));
	return -1;
    }

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
    // They're likely large and may span multiple pages.  Thus we reead
    // and submit data until we get our two pacakets, watching that no
    // pages are missing.  If a page is missing, error out; losing a
    // header page is the only place where missing data is fatal. */
    counter = 0;
    while (counter < 2) {
	while(counter < 2) {
	    int result=ogg_sync_pageout(&m_oy, &m_og);
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
	m_buffer = ogg_sync_buffer(&m_oy, 4096);
	bytes = m_source->read(m_buffer, 4096);
	if (!bytes && counter < 2) {
	    Kwave::MessageBox::error(widget, i18n(
	        "End of file before finding all Vorbis headers."));
	    return -1;
	}
	ogg_sync_wrote(&m_oy, bytes);
    }

    // OK, got and parsed all three headers. Initialize the Vorbis
    // packet->PCM decoder. */
    vorbis_synthesis_init(&m_vd, &m_vi); // central decode state
    vorbis_block_init(&m_vd, &m_vb); // local state for most of the decode
                                     // so multiple block decodes can
                                     // proceed in parallel.  We could init
                                     // multiple vorbis_block structures
                                     // for m_vd here
    return 1;
}

//***************************************************************************
bool OggDecoder::open(QWidget *widget, QIODevice &src)
{
    metaData().clear();
    Q_ASSERT(!m_source);
    if (m_source) qWarning("OggDecoder::open(), already open !");

    // try to open the source
    if (!src.open(QIODevice::ReadOnly)) {
	qWarning("failed to open source !");
	return false;
    }

    // take over the source
    m_source = &src;

    /********** Decode setup ************/
    qDebug("--- OggDecoder::open() ---");
    ogg_sync_init(&m_oy); // Now we can read pages

    // read the header the first time
    if (parseHeader(widget) < 0) return false;

    Kwave::FileInfo info(metaData());

    // get the standard properties
    info.setLength(0);         // use streaming
    info.setRate(m_vi.rate);
    info.setBits(SAMPLE_BITS); // use Kwave's internal resolution
    info.setTracks(m_vi.channels);
    info.set(Kwave::INF_MIMETYPE, DEFAULT_MIME_TYPE);
    info.set(Kwave::INF_COMPRESSION, Kwave::CompressionType::OGG_VORBIS);
    info.set(Kwave::INF_SOURCE, QString(m_vc.vendor));
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
	QString s = *ptr;
	if (s.length() && !s.contains('=')) {
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

    metaData().replace(info);

    return true;
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
bool OggDecoder::decode(QWidget *widget, Kwave::MultiWriter &dst)
{
    Q_ASSERT(m_source);
    if (!m_source) return false;

    int eos = 0;
    unsigned int stream_start_pos = m_source->pos();

    // we repeat if the bitstream is chained
    while (!dst.isCanceled()) {
	// The rest is just a straight decode loop until end of stream
	while (!eos) {
	    while (!eos) {
		int result = ogg_sync_pageout(&m_oy, &m_og);
		if (result == 0) break; // need more data
		if (result < 0) {
		    // missing or corrupt data at this page position
		    Kwave::MessageBox::error(widget, i18n(
		        "Corrupt or missing data in bitstream. Continuing."
		        ));
		} else {
		    // can safely ignore errors at this point
		    ogg_stream_pagein(&m_os,&m_og);
		    while (1) {
			result = ogg_stream_packetout(&m_os, &m_op);

			if (result == 0) break; // need more data
			if (result < 0) {
			    // missing or corrupt data at this page position
			    // no reason to complain; already complained above
			} else {
			    // we have a packet.  Decode it
			    float **pcm;
			    int samples;

			    // test for success!
			    if (vorbis_synthesis(&m_vb, &m_op) == 0)
			        vorbis_synthesis_blockin(&m_vd, &m_vb);

			    // **pcm is a multichannel float vector. In
			    // stereo, for example, pcm[0] is left, and
			    // pcm[1] is right.  samples is the size of
			    // each channel.  Convert the float values
			    // (-1.<=range<=1.) to whatever PCM format
			    // and write it out
			    while ((samples = vorbis_synthesis_pcmout(
			           &m_vd, &pcm)) > 0)
			    {
				register int bout =
				    decodeFrame(pcm, samples, dst);
				// tell libvorbis how many samples we
				// actually consumed
				vorbis_synthesis_read(&m_vd, bout);
			    }

			    // signal the current position
			    emit sourceProcessed(m_source->pos());
			}
		    }
		    if (ogg_page_eos(&m_og) || dst.isCanceled()) eos = 1;
		}
	    }

	    if (!eos) {
		m_buffer = ogg_sync_buffer(&m_oy, 4096);
		unsigned int bytes = m_source->read(m_buffer, 4096);
		ogg_sync_wrote(&m_oy, bytes);
		if (!bytes) eos = 1;
	    }
	}

	// clean up this logical bitstream; before exit we see if we're
	// followed by another [chained]
	ogg_stream_clear(&m_os);

	// ogg_page and ogg_packet structs always point to storage in
	// libvorbis.  They're never freed or manipulated directly
	vorbis_block_clear(&m_vb);
	vorbis_dsp_clear(&m_vd);
	vorbis_comment_clear(&m_vc);
	vorbis_info_clear(&m_vi);  // must be called last

	// parse the next header, maybe we parse a stream or chain...
	if (eos || (parseHeader(widget) < 1)) break;
    }

    // OK, clean up the framer
    ogg_sync_clear(&m_oy);

    // signal the current position
    emit sourceProcessed(m_source->pos());

    if (!Kwave::FileInfo(metaData()).contains(Kwave::INF_BITRATE_NOMINAL) &&
        !Kwave::FileInfo(metaData()).contains(Kwave::INF_VBR_QUALITY))
    {
	qWarning("file contains neither nominal bitrate (ABR mode) "\
	         "nor quality (VBR mode)");

	const unsigned int samples = dst.last();
	int bitrate = DEFAULT_BITRATE;

	if (static_cast<int>(Kwave::FileInfo(metaData()).rate()) && samples) {
	    // guess bitrates from the stream
	    const unsigned int stream_end_pos = m_source->pos();
	    const unsigned int stream_read = stream_end_pos -
	                                     stream_start_pos + 1;
	    double bits = static_cast<double>(stream_read) * 8.0;
	    double seconds = static_cast<double>(samples) /
		static_cast<double>(Kwave::FileInfo(metaData()).rate());
	    bitrate = static_cast<unsigned int>(bits / seconds);

	    // round to neares standard bitrate
	    bitrate = Kwave::StandardBitrates::instance().nearest(bitrate);
	    qDebug("-> using guessed bitrate %d bits/sec", bitrate);
	} else {
	    // guessing not possible -> use default
	    qDebug("-> using default %d kBits/sec", bitrate);
	}
	Kwave::FileInfo info(metaData());
	info.set(Kwave::INF_BITRATE_NOMINAL, QVariant(static_cast<int>(bitrate)));
	metaData().replace(info);
    }

    // return with a valid Signal, even if the user pressed cancel !
    return true;
}

//***************************************************************************
void OggDecoder::close()
{
    m_source = 0;
}

//***************************************************************************
//***************************************************************************
