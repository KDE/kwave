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
#include <byteswap.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kapp.h>
#include <kglobal.h>
#include <kaboutdata.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include <vorbis/codec.h>

#include "libkwave/FileInfo.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"

#include "OggCodecPlugin.h"
#include "OggEncoder.h"

/***************************************************************************/
static const struct {
    FileProperty property;
    const char *name;
} supported_properties[] = {
	{ INF_NAME,         "TITLE" },
	{ INF_VERSION,      "VERSION" },
	{ INF_ALBUM,        "ALBUM" },
	{ INF_TRACK,        "TRACKNUMBER" },
	{ INF_AUTHOR,       "ARTIST" },
	{ INF_PERFORMER,    "PERFORMER" },
	{ INF_COPYRIGHT,    "COPYRIGHT" },
	{ INF_LICENSE,      "LICENSE" },
	{ INF_ORGANIZATION, "ORGANIZATION" },
	{ INF_SUBJECT,      "DESCRIPTION" },
	{ INF_GENRE,        "GENRE" },
	{ INF_SOURCE,       "LOCATION" },
	{ INF_CONTACT,      "CONTACT" },
	{ INF_ISRC,         "ISRC" },
	{ INF_SOFTWARE,     "ENCODE" },
	{ INF_MIMETYPE,     0 }
};

/***************************************************************************/
OggEncoder::OggEncoder()
    :Encoder()
{
    LOAD_MIME_TYPES;
}

/***************************************************************************/
OggEncoder::~OggEncoder()
{
}

/***************************************************************************/
Encoder *OggEncoder::instance()
{
    return new OggEncoder();
}

/***************************************************************************/
QValueList<FileProperty> OggEncoder::supportedProperties()
{
    QValueList<FileProperty> list;
    for (unsigned int i=0; i < sizeof(supported_properties) /
                               sizeof(supported_properties[0]); ++i)
    {
	list.append(supported_properties[i].property);
    }
    
    return list;
}

/***************************************************************************/
void OggEncoder::encodeProperties(FileInfo &info, vorbis_comment *vc)
{
    for (unsigned int i=0; i < sizeof(supported_properties) /
                               sizeof(supported_properties[0]); ++i)
    {
	FileProperty property = supported_properties[i].property;
	
	if (!info.contains(property)) continue; // skip if not present
	if (!info.canLoadSave(property)) continue;

	// encode the property as string
	const char *tag = supported_properties[i].name;
	QString value = info.get(property).toString();
	vorbis_comment_add_tag(vc, (char*)tag, (char*)value.data());
    }
}

/***************************************************************************/
bool OggEncoder::encode(QWidget *widget, MultiTrackReader &src,
                        QIODevice &dst, FileInfo &info)
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
    int ret;

    // get info
    const unsigned int tracks = info.tracks();
    const long rate = (long)info.rate();
    
    // some checks first
    ASSERT(tracks < 255);
    if (tracks > 255) return false;

    /********** Encode setup ************/
    vorbis_info_init(&vi);

    /* choose an encoding mode.  A few possibilities commented out, one
       actually used: */

    /*********************************************************************
     Encoding using a VBR quality mode.  The usable range is -.1
     (lowest quality, smallest file) to 1. (highest quality, largest file).
     Example quality mode .4: 44kHz stereo coupled, roughly 128kbps VBR

     ret = vorbis_encode_init_vbr(&vi,2,44100,.4);

     ---------------------------------------------------------------------

     Encoding using an average bitrate mode (ABR).
     example: 44kHz stereo coupled, average 128kbps VBR

     ret = vorbis_encode_init(&vi,2,44100,-1,128000,-1);

     ---------------------------------------------------------------------

     Encode using a qulity mode, but select that quality mode by asking for
     an approximate bitrate.  This is not ABR, it is true VBR, but selected
     using the bitrate interface, and then turning bitrate management off:

     ret = ( vorbis_encode_setup_managed(&vi,2,44100,-1,128000,-1) ||
             vorbis_encode_ctl(&vi,OV_ECTL_RATEMANAGE_AVG,NULL) ||
             vorbis_encode_setup_init(&vi));

     *********************************************************************/

    ret = vorbis_encode_init_vbr(&vi, tracks, rate, .5);

    /* do not continue if setup failed; this can happen if we ask for a
       mode that libVorbis does not support (eg, too low a bitrate, etc,
       will return 'OV_EIMPL') */
    if (ret) {
	KMessageBox::sorry(widget, i18n("One or more encoding "
	    "parameters are not supported. Please change the "
	    "settings and try again..."));
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
    srand(time(NULL));
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
	
	vorbis_analysis_headerout(&vd,&vc,&header,&header_comm,&header_code);
	ogg_stream_packetin(&os,&header); // automatically placed in its own
					  // page
	ogg_stream_packetin(&os,&header_comm);
	ogg_stream_packetin(&os,&header_code);

	// This ensures the actual audio data will start on a
	// new page, as per spec
	while (!eos) {
	    if (!ogg_stream_flush(&os, &og)) break;
	    dst.writeBlock((char*)og.header, og.header_len);
	    dst.writeBlock((char*)og.body, og.body_len);
	}
    }

    while (!eos) {
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
	    unsigned int pos;
	    sample_t s;
	    for (pos=0; (pos < BUFFER_SIZE)  && !src.eof(); ++pos) {
		for (unsigned int track = 0; (track < tracks); ++track) {
		    *(src[track]) >> s;
		    buffer[track][pos] = s;
		}
	    }
	    
	    // tell the library how much we actually submitted
	    vorbis_analysis_wrote(&vd, pos);
	}
	
	// vorbis does some data preanalysis, then divvies up blocks for
	// more involved (potentially parallel) processing.  Get a single
	// block for encoding now
	while(vorbis_analysis_blockout(&vd, &vb) == 1) {
	    // analysis, assume we want to use bitrate management
	    vorbis_analysis(&vb, NULL);
	    vorbis_bitrate_addblock(&vb);
	
	    while(vorbis_bitrate_flushpacket(&vd, &op)) {
		// weld the packet into the bitstream
		ogg_stream_packetin(&os, &op);
		
		// write out pages (if any)
		while (!eos) {
		    int result = ogg_stream_pageout(&os,&og);
		    if (!result) break;
		    dst.writeBlock((char*)og.header, og.header_len);
		    dst.writeBlock((char*)og.body, og.body_len);
		
		    // this could be set above, but for illustrative
		    // purposes, I do it here (to show that vorbis
		    // does know where the stream ends)
		    if (ogg_page_eos(&og)) eos=true;
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

/***************************************************************************/
/***************************************************************************/
