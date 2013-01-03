/*************************************************************************
         OggEncoder.cpp  -  encoder for Ogg/Vorbis data
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

#include <math.h>
#include <stdlib.h>

#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QTime>

#include <klocale.h>
#include <kmimetype.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kaboutdata.h>

#include <vorbis/vorbisenc.h>

#include "libkwave/FileInfo.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MetaDataList.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/Sample.h"

#include "OggCodecPlugin.h"
#include "OggEncoder.h"

/** bitrate to be used when no bitrate has been selected */
#define DEFAULT_BITRATE 64000

/***************************************************************************/
static const struct {
    Kwave::FileProperty property;
    const char *name;
} supported_properties[] = {
	{ Kwave::INF_NAME,         "TITLE" },
	{ Kwave::INF_VERSION,      "VERSION" },
	{ Kwave::INF_ALBUM,        "ALBUM" },
	{ Kwave::INF_TRACK,        "TRACKNUMBER" },
	{ Kwave::INF_AUTHOR,       "ARTIST" },
	{ Kwave::INF_PERFORMER,    "PERFORMER" },
	{ Kwave::INF_COPYRIGHT,    "COPYRIGHT" },
	{ Kwave::INF_LICENSE,      "LICENSE" },
	{ Kwave::INF_ORGANIZATION, "ORGANIZATION" },
	{ Kwave::INF_SUBJECT,      "DESCRIPTION" },
	{ Kwave::INF_GENRE,        "GENRE" },
	{ Kwave::INF_SOURCE,       "LOCATION" },
	{ Kwave::INF_CONTACT,      "CONTACT" },
	{ Kwave::INF_ISRC,         "ISRC" },
	{ Kwave::INF_SOFTWARE,     "ENCODER" },
	{ Kwave::INF_CREATION_DATE,"DATE" },
	{ Kwave::INF_VBR_QUALITY,  "VBR_QUALITY" },
	{ Kwave::INF_MIMETYPE,     0 }
};

/***************************************************************************/
Kwave::OggEncoder::OggEncoder()
    :Kwave::Encoder()
{
    REGISTER_COMMON_MIME_TYPES;

#ifdef HAVE_OGG_OPUS
    REGISTER_OGG_OPUS_MIME_TYPES;
    REGISTER_COMPRESSION_TYPE_OGG_OPUS;
#endif /* HAVE_OGG_OPUS */

#ifdef HAVE_OGG_VORBIS
    REGISTER_OGG_VORBIS_MIME_TYPES;
    REGISTER_COMPRESSION_TYPE_OGG_VORBIS;
#endif /* HAVE_OGG_VORBIS */
}

/***************************************************************************/
Kwave::OggEncoder::~OggEncoder()
{
}

/***************************************************************************/
Kwave::Encoder *Kwave::OggEncoder::instance()
{
    return new Kwave::OggEncoder();
}

/***************************************************************************/
QList<Kwave::FileProperty> Kwave::OggEncoder::supportedProperties()
{
    QList<Kwave::FileProperty> list;

    for (unsigned int i=0; i < sizeof(supported_properties) /
                               sizeof(supported_properties[0]); ++i)
    {
	list.append(supported_properties[i].property);
    }

    return list;
}

/***************************************************************************/
void Kwave::OggEncoder::encodeProperties(const Kwave::FileInfo &info,
                                         vorbis_comment *vc)
{
    for (unsigned int i=0; i < sizeof(supported_properties) /
                               sizeof(supported_properties[0]); ++i)
    {
	Kwave::FileProperty property = supported_properties[i].property;

	if (!info.contains(property)) continue; // skip if not present

	// encode the property as string
	const char *tag = supported_properties[i].name;
	if (!tag) continue;

	QByteArray value = info.get(property).toString().toUtf8();
	vorbis_comment_add_tag(vc, const_cast<char *>(tag), value.data());
    }
}

/***************************************************************************/
bool Kwave::OggEncoder::encode(QWidget *widget, Kwave::MultiTrackReader &src,
                               QIODevice &dst,
                               const Kwave::MetaDataList &meta_data)
{
    #define BUFFER_SIZE 1024

    ogg_stream_state os; // take physical pages, weld into a logical
                         // stream of packets
    ogg_page         og; // one Ogg bitstream page.  Vorbis packets are inside
    ogg_packet       op; // one raw packet of data for decode
    vorbis_info      vi; // struct that stores all the static vorbis bitstream
			 // settings
    vorbis_comment   vc; // struct that stores all the user comments
    vorbis_dsp_state vd; // central working state for the packet->PCM decoder
    vorbis_block     vb; // local working space for packet->PCM decode

    bool eos = false;
    int ret = -1;

    // get info: tracks, sample rate, bitrate(s)
    const Kwave::FileInfo info(meta_data);
    const unsigned int tracks = info.tracks();
    const unsigned int length = info.length();
    const long sample_rate = static_cast<const long>(info.rate());

    // ABR bitrates
    int bitrate_nominal = info.contains(Kwave::INF_BITRATE_NOMINAL) ?
        QVariant(info.get(Kwave::INF_BITRATE_NOMINAL)).toInt() : -1;
    int bitrate_lower = info.contains(Kwave::INF_BITRATE_LOWER) ?
        QVariant(info.get(Kwave::INF_BITRATE_LOWER)).toInt() : -1;
    int bitrate_upper = info.contains(Kwave::INF_BITRATE_UPPER) ?
        QVariant(info.get(Kwave::INF_BITRATE_UPPER)).toInt() : -1;

    // VBR quality
    int vbr_quality = info.contains(Kwave::INF_VBR_QUALITY) ?
        QVariant(info.get(Kwave::INF_VBR_QUALITY)).toInt() : -1;

    qDebug("OggEncoder: ABR=%d...%d...%d Bits/s, VBR=%d%%",
           bitrate_lower,bitrate_nominal,bitrate_upper,vbr_quality);

    if ((vbr_quality < 0) && (bitrate_nominal <= 0)) {
	// no quality and no bitrate given -> complain !
	if (Kwave::MessageBox::warningContinueCancel(widget,
	    i18n("You have not selected any bitrate for the encoding. "
	         "Do you want to continue and encode with %1 kBit/s "
	         "or cancel and choose a different bitrate?",
	         DEFAULT_BITRATE/1000)) !=
	    KMessageBox::Continue)
	    return false; // <- canceled

	bitrate_nominal = DEFAULT_BITRATE;
	bitrate_lower = -1;
	bitrate_upper = -1;
    }

    // some checks first
    Q_ASSERT(tracks < 255);
    if (tracks > 255) return false;

    /********** Encode setup ************/
    vorbis_info_init(&vi);

    if ((bitrate_lower > 0) || (bitrate_upper > 0)) {
	// Encoding using ABR mode.
	bitrate_nominal = (bitrate_upper + bitrate_lower) / 2;
	ret = vorbis_encode_init(&vi, tracks, sample_rate,
	                         bitrate_upper,
	                         bitrate_nominal,
	                         bitrate_lower);
	qDebug("OggEncoder: ABR with %d...%d...%d Bits/s",
	       bitrate_lower, bitrate_nominal, bitrate_upper);
    } else if ((vbr_quality < 0) && (bitrate_nominal > 0)) {
	// Encoding using constant bitrate in ABR mode
	ret = vorbis_encode_setup_managed(&vi, tracks, sample_rate,
	      -1, bitrate_nominal, -1);

	// ### this might not work with Debian / old Ogg/Vorbis ###
#ifdef OV_ECTL_RATEMANAGE_AVG
	if (!ret) ret =
	      vorbis_encode_ctl(&vi,OV_ECTL_RATEMANAGE_AVG,NULL) ||
              vorbis_encode_setup_init(&vi);
#endif

	qDebug("OggEncoder: CBR with %d Bits/s", bitrate_nominal);
    } else if (vbr_quality >= 0) {
	// Encoding using VBR mode.
	ret = vorbis_encode_init_vbr(&vi, tracks, sample_rate,
	    static_cast<float>(vbr_quality) / 100.0);
	qDebug("OggEncoder: VBR with %d%%", vbr_quality);
    } else {
	// unknown setup !?
	qWarning("unknown Ogg/Vorbis setup: VBR quality=%d%%, "\
	         "ABR lower=%d, ABR highest=%d, ABR nominal=%d",
	         vbr_quality, bitrate_lower, bitrate_upper,
	         bitrate_nominal);
	return false;
    }

    /*********************************************************************
     Encoding using a VBR quality mode.  The usable range is -.1
     (lowest quality, smallest file) to 1. (highest quality, largest file).
     Example quality mode .4: 44kHz stereo coupled, roughly 128kbps VBR

     ret = vorbis_encode_init_vbr(&vi,2,44100,.4);

     ---------------------------------------------------------------------

     Encode using a quality mode, but select that quality mode by asking for
     an approximate bitrate.  This is not ABR, it is true VBR, but selected
     using the bitrate interface, and then turning bitrate management off:

     ret = ( vorbis_encode_setup_managed(&vi,2,44100,-1,128000,-1) ||
             vorbis_encode_ctl(&vi,OV_ECTL_RATEMANAGE_AVG,NULL) ||
             vorbis_encode_setup_init(&vi));

     *********************************************************************/

    /* do not continue if setup failed; this can happen if we ask for a
       mode that libVorbis does not support (eg, too low a bitrate, etc,
       will return 'OV_EIMPL') */
    if (ret) {
	Kwave::MessageBox::sorry(widget, i18n("One or more encoding "
	    "parameters are not supported. Please change the "
	    "settings and try again."));
	return false;
    }

    // open the output device
    if (!dst.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
	Kwave::MessageBox::error(widget,
	    i18n("Unable to open the file for saving."));
	return false;
    }

    // add all supported properties as file comments
    vorbis_comment_init(&vc);
    encodeProperties(info, &vc);

    // set up the analysis state and auxiliary encoding storage
    vorbis_analysis_init(&vd, &vi);
    vorbis_block_init(&vd, &vb);

    // set up our packet->stream encoder

    // pick a random serial number; that way we can more likely build
    // chained streams just by concatenation
    srand(QTime::currentTime().msec());
    ogg_stream_init(&os, rand());

    // Vorbis streams begin with three headers; the initial header (with
    // most of the codec setup parameters) which is mandated by the Ogg
    // bitstream spec.  The second header holds any comment fields.  The
    // third header holds the bitstream codebook.  We merely need to
    // make the headers, then pass them to libvorbis one at a time;
    // libvorbis handles the additional Ogg bitstream constraints
    {
	ogg_packet header;
	ogg_packet header_comm;
	ogg_packet header_code;

	vorbis_analysis_headerout(&vd, &vc, &header, &header_comm,
	                          &header_code);
	// automatically placed in its own page
	ogg_stream_packetin(&os, &header);
	ogg_stream_packetin(&os, &header_comm);
	ogg_stream_packetin(&os, &header_code);

	// This ensures the actual audio data will start on a
	// new page, as per spec
	while (!eos) {
	    if (!ogg_stream_flush(&os, &og)) break;
	    dst.write(reinterpret_cast<char *>(og.header), og.header_len);
	    dst.write(reinterpret_cast<char *>(og.body),   og.body_len);
	}
    }

    unsigned int rest = length;
    while (!eos && !src.isCanceled()) {
	if (src.eof()) {
	    // end of file.  this can be done implicitly in the mainline,
	    // but it's easier to see here in non-clever fashion.
	    // Tell the library we're at end of stream so that it can handle
	    // the last frame and mark end of stream in the output properly
	    vorbis_analysis_wrote(&vd, 0);
	} else {
	    // data to encode

	    // expose the buffer to submit data
	    float **buffer = vorbis_analysis_buffer(&vd, BUFFER_SIZE);
	    unsigned int pos = 0;
	    unsigned int len = (rest > BUFFER_SIZE) ? BUFFER_SIZE : rest;
	    Kwave::SampleArray samples(BUFFER_SIZE);
	    for (unsigned int track = 0; (track < tracks); ++track) {
		float *p = buffer[track];
		unsigned int l = src[track]->read(samples, 0, len);
		const sample_t *s = samples.data();

		const unsigned int block = 8;
		pos = 0;
		while (pos + block < l) {
		    for (unsigned int i = 0; i < block; i++, pos++)
			p[pos] = sample2float(s[pos]);
		}
		while (pos < l) {
		    p[pos] = sample2float(s[pos]);
		    pos++;
		}
		while (pos < len)
		    p[pos++] = 0;
	    }

	    // tell the library how much we actually submitted
	    vorbis_analysis_wrote(&vd, pos);
	}

	// vorbis does some data preanalysis, then divvies up blocks for
	// more involved (potentially parallel) processing.  Get a single
	// block for encoding now
	while (vorbis_analysis_blockout(&vd, &vb) == 1) {
	    // analysis, assume we want to use bitrate management
	    vorbis_analysis(&vb, 0);
	    vorbis_bitrate_addblock(&vb);

	    while (vorbis_bitrate_flushpacket(&vd, &op)) {
		// weld the packet into the bitstream
		ogg_stream_packetin(&os, &op);

		// write out pages (if any)
		while (!eos) {
		    int result = ogg_stream_pageout(&os, &og);
		    if (!result) break;
		    dst.write(reinterpret_cast<char*>(og.header), og.header_len);
		    dst.write(reinterpret_cast<char *>(og.body), og.body_len);

		    // this could be set above, but for illustrative
		    // purposes, I do it here (to show that vorbis
		    // does know where the stream ends)
		    if (ogg_page_eos(&og)) eos = true;
		}
	    }
	}
    }

    // clean up and exit.  vorbis_info_clear() must be called last
    ogg_stream_clear(&os);
    vorbis_block_clear(&vb);
    vorbis_dsp_clear(&vd);
    vorbis_comment_clear(&vc);
    vorbis_info_clear(&vi);
    // ogg_page and ogg_packet structs always point to storage in
    // libvorbis.  They're never freed or manipulated directly

    return true;
}

//***************************************************************************
//***************************************************************************
