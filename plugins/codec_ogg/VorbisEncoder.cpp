/*************************************************************************
    VorbisEncoder.cpp  -  sub encoder base class for Vorbis in an Ogg container
                             -------------------
    begin                : Thu Jan 03 2013
    copyright            : (C) 2013 by Thomas Eschenbacher
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

#include <string.h>

#include <QtCore/QtGlobal>

#include <klocale.h>

#include "libkwave/MessageBox.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleArray.h"

#include "VorbisEncoder.h"

/** size of a buffer used for Vorbis encoding */
#define BUFFER_SIZE 1024

/** bitrate to be used when no bitrate has been selected */
#define DEFAULT_BITRATE 64000

/***************************************************************************/
Kwave::VorbisEncoder::VorbisEncoder(ogg_stream_state &os,
                                    ogg_page         &og,
                                    ogg_packet       &op)
    :m_comments_map(), m_info(), m_os(os), m_og(og), m_op(op)
{
    memset(&m_vb, 0x00, sizeof(m_vb));
    memset(&m_vc, 0x00, sizeof(m_vc));
    memset(&m_vd, 0x00, sizeof(m_vd));
    memset(&m_vi, 0x00, sizeof(m_vi));
}

/***************************************************************************/
Kwave::VorbisEncoder::~VorbisEncoder()
{
    close();
}

/***************************************************************************/
void Kwave::VorbisEncoder::encodeProperties(const Kwave::FileInfo &info)
{
    foreach (QString key, m_comments_map.keys()) {
	Kwave::FileProperty property = m_comments_map[key];
	if (!info.contains(property)) continue; // skip if not present

	// encode the property as string
	vorbis_comment_add_tag(&m_vc,
	    __(key),
	    __(info.get(property).toString())
	);
    }
}

/***************************************************************************/
bool Kwave::VorbisEncoder::open(QWidget *widget, const Kwave::FileInfo &info,
                                Kwave::MultiTrackReader &src)
{
    int ret = -1;

    Q_UNUSED(src);

    // get info: tracks, sample rate, bitrate(s)
    m_info = info;
    const unsigned int tracks = info.tracks();
    const long int sample_rate = static_cast<const long int>(info.rate());

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
	         DEFAULT_BITRATE/1000)) != KMessageBox::Continue)
	    return false; // <- canceled

	bitrate_nominal = DEFAULT_BITRATE;
	bitrate_lower = -1;
	bitrate_upper = -1;
    }

    // some checks first
    Q_ASSERT(tracks < 255);
    if (tracks > 255) return false;

    /********** Encode setup ************/
    vorbis_info_init(&m_vi);

    if ((bitrate_lower > 0) || (bitrate_upper > 0)) {
	// Encoding using ABR mode.
	bitrate_nominal = (bitrate_upper + bitrate_lower) / 2;
	ret = vorbis_encode_init(&m_vi, tracks, sample_rate,
	                         bitrate_upper,
	                         bitrate_nominal,
	                         bitrate_lower);
	qDebug("VorbisEncoder: ABR with %d...%d...%d Bits/s",
	       bitrate_lower, bitrate_nominal, bitrate_upper);
    } else if ((vbr_quality < 0) && (bitrate_nominal > 0)) {
	// Encoding using constant bitrate in ABR mode
	ret = vorbis_encode_setup_managed(&m_vi, tracks, sample_rate,
	      -1, bitrate_nominal, -1);

#ifdef OV_ECTL_RATEMANAGE2_SET
	if (!ret) ret =
	      vorbis_encode_ctl(&m_vi, OV_ECTL_RATEMANAGE2_SET, 0) ||
	      vorbis_encode_setup_init(&m_vi);
#else /* OV_ECTL_RATEMANAGE2_SET */
    #warning "OV_ECTL_RATEMANAGE2_SET not supported, maybe libvorbis is too old?"
#endif /* OV_ECTL_RATEMANAGE2_SET */

	qDebug("VorbisEncoder: CBR with %d Bits/s", bitrate_nominal);
    } else if (vbr_quality >= 0) {
	// Encoding using VBR mode.
	ret = vorbis_encode_init_vbr(&m_vi, tracks, sample_rate,
	    static_cast<float>(vbr_quality) / 100.0);
	qDebug("OggEncoder: VBR with %d%%", vbr_quality);
    } else {
	// unknown setup !?
	qWarning("unknown Ogg/Vorbis setup: VBR quality=%d%%, "
	         "ABR lower=%d, ABR highest=%d, ABR nominal=%d",
	         vbr_quality, bitrate_lower, bitrate_upper,
	         bitrate_nominal);
	return false;
    }

    /* do not continue if setup failed; this can happen if we ask for a
       mode that libVorbis does not support (eg, too low a bitrate, etc,
       will return 'OV_EIMPL') */
    if (ret) {
	Kwave::MessageBox::sorry(widget, i18n("One or more encoding "
	    "parameters are not supported. Please change the "
	    "settings and try again."));
	return false;
    }

    // add all supported properties as file comments
    vorbis_comment_init(&m_vc);
    encodeProperties(info);

    // set up the analysis state and auxiliary encoding storage
    vorbis_analysis_init(&m_vd, &m_vi);
    vorbis_block_init(&m_vd, &m_vb);

    return true;
}

/***************************************************************************/
bool Kwave::VorbisEncoder::writeHeader(QIODevice &dst)
{
    // Vorbis streams begin with three headers; the initial header (with
    // most of the codec setup parameters) which is mandated by the Ogg
    // bitstream spec.  The second header holds any comment fields.  The
    // third header holds the bitstream codebook.  We merely need to
    // make the headers, then pass them to libvorbis one at a time;
    // libvorbis handles the additional Ogg bitstream constraints
    ogg_packet header;
    ogg_packet header_comm;
    ogg_packet header_code;

    vorbis_analysis_headerout(&m_vd, &m_vc, &header, &header_comm,
                              &header_code);
    // automatically placed in its own page
    ogg_stream_packetin(&m_os, &header);
    ogg_stream_packetin(&m_os, &header_comm);
    ogg_stream_packetin(&m_os, &header_code);

    // This ensures the actual audio data will start on a
    // new page, as per spec
    while (ogg_stream_flush(&m_os, &m_og)) {
	dst.write(reinterpret_cast<char *>(m_og.header),
	          m_og.header_len);
	dst.write(reinterpret_cast<char *>(m_og.body),
	          m_og.body_len);
    }

    return true;
}

/***************************************************************************/
bool Kwave::VorbisEncoder::encode(Kwave::MultiTrackReader &src,
                                  QIODevice &dst)
{
    bool eos                  = false;
    const unsigned int tracks = m_info.tracks();
    const unsigned int length = m_info.length();

    unsigned int rest = length;
    while (!eos && !src.isCanceled()) {
	if (src.eof()) {
	    // end of file.  this can be done implicitly in the mainline,
	    // but it's easier to see here in non-clever fashion.
	    // Tell the library we're at end of stream so that it can handle
	    // the last frame and mark end of stream in the output properly
	    vorbis_analysis_wrote(&m_vd, 0);
	} else {
	    // data to encode

	    // expose the buffer to submit data
	    float **buffer = vorbis_analysis_buffer(&m_vd, BUFFER_SIZE);
	    unsigned int pos = 0;
	    unsigned int len = (rest > BUFFER_SIZE) ? BUFFER_SIZE : rest;
	    Kwave::SampleArray samples(BUFFER_SIZE);
	    for (unsigned int track = 0; track < tracks; ++track) {
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
	    vorbis_analysis_wrote(&m_vd, pos);
	}

	// vorbis does some data preanalysis, then divvies up blocks for
	// more involved (potentially parallel) processing.  Get a single
	// block for encoding now
	while (vorbis_analysis_blockout(&m_vd, &m_vb) == 1) {
	    // analysis, assume we want to use bitrate management
	    vorbis_analysis(&m_vb, 0);
	    vorbis_bitrate_addblock(&m_vb);

	    while (vorbis_bitrate_flushpacket(&m_vd, &m_op)) {
		// weld the packet into the bitstream
		ogg_stream_packetin(&m_os, &m_op);

		// write out pages (if any)
		while (!eos) {
		    int result = ogg_stream_pageout(&m_os, &m_og);
		    if (!result) break;
		    dst.write(reinterpret_cast<char*>(m_og.header),
		              m_og.header_len);
		    dst.write(reinterpret_cast<char *>(m_og.body),
		              m_og.body_len);

		    // this could be set above, but for illustrative
		    // purposes, I do it here (to show that vorbis
		    // does know where the stream ends)
		    if (ogg_page_eos(&m_og)) eos = true;
		}
	    }
	}
    }

    return true;
}

/***************************************************************************/
void Kwave::VorbisEncoder::close()
{
    ogg_stream_clear(&m_os);

    vorbis_block_clear(&m_vb);
    vorbis_dsp_clear(&m_vd);
    vorbis_comment_clear(&m_vc);
    vorbis_info_clear(&m_vi);   // <- must be called last
}

/***************************************************************************/
/***************************************************************************/
