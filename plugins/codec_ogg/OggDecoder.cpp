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
#include <byteswap.h>
#include <stdlib.h>
#include <math.h>
#include <vorbis/codec.h>

#include <qlist.h>
#include <qprogressdialog.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>

#include "libkwave/MultiTrackWriter.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleWriter.h"
#include "libkwave/Signal.h"

#include "OggCodecPlugin.h"
#include "OggDecoder.h"


//***************************************************************************
OggDecoder::OggDecoder()
    :Decoder(), m_source(0)
{
    LOAD_MIME_TYPES;
}

//***************************************************************************
OggDecoder::~OggDecoder()
{
    if (m_source) close();
}

//***************************************************************************
Decoder *OggDecoder::instance()
{
    return new OggDecoder();
}

ogg_int16_t convbuffer[4096]; /* take 8k out of the data segment, not the stack */
int convsize=4096;

  ogg_sync_state   oy; /* sync and verify incoming physical bitstream */
  ogg_stream_state os; /* take physical pages, weld into a logical
			  stream of packets */
  ogg_page         og; /* one Ogg bitstream page.  Vorbis packets are inside */
  ogg_packet       op; /* one raw packet of data for decode */

  vorbis_info      vi; /* struct that stores all the static vorbis bitstream
			  settings */
  vorbis_comment   vc; /* struct that stores all the bitstream user comments */
  vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
  vorbis_block     vb; /* local working space for packet->PCM decode */

  char *buffer = 0;
  int  bytes = 0;
  int i = 0;

//***************************************************************************
int OggDecoder::parseHeader(QWidget *widget)
{

    // grab some data at the head of the stream.  We want the first page
    // (which is guaranteed to be small and only contain the Vorbis
    // stream initial header) We need the first page to get the stream
    // serialno.

    // submit a 4k block to libvorbis' Ogg layer
    buffer = ogg_sync_buffer(&oy, 4096);
    ASSERT(buffer);
    if (!buffer) return -1;
    
    bytes = m_source->readBlock(buffer,4096);
    ogg_sync_wrote(&oy, bytes);

    // Get the first page.
    if (ogg_sync_pageout(&oy, &og) != 1) {
	// have we simply run out of data?  If so, we're done.
	if (bytes < 4096) return 0;
	
	// error case.  Must not be Vorbis data
	KMessageBox::error(widget, i18n(
	     "Input does not appear to be an Ogg bitstream."));
	return -1;
    }

    // Get the serial number and set up the rest of decode.
    // serialno first; use it to set up a logical stream
    ogg_stream_init(&os, ogg_page_serialno(&og));

    // extract the initial header from the first page and verify that the
    // Ogg bitstream is in fact Vorbis data

    // I handle the initial header first instead of just having the code
    // read all three Vorbis headers at once because reading the initial
    // header is an easy way to identify a Vorbis bitstream and it's
    // useful to see that functionality seperated out.
    vorbis_info_init(&vi);
    vorbis_comment_init(&vc);
    if (ogg_stream_pagein(&os, &og) < 0) {
	// error; stream version mismatch perhaps
	KMessageBox::error(widget, i18n(
	    "Error reading first page of Ogg bitstream data."));
	return -1;
    }

    if (ogg_stream_packetout(&os, &op) != 1) {
	// no page? must not be vorbis
	KMessageBox::error(widget, i18n(
	    "Error reading initial header packet."));
	return -1;
    }

    if (vorbis_synthesis_headerin(&vi, &vc, &op) < 0) {
	// error case; not a vorbis header
	KMessageBox::error(widget, i18n(
	    "This Ogg bitstream does not contain Vorbis audio data."));
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
    i = 0;
    while (i < 2) {
	while(i < 2) {
	    int result=ogg_sync_pageout(&oy, &og);
	    if (result == 0) return 0; // Need more data
	    // Don't complain about missing or corrupt data yet.  We'll
	    // catch it at the packet output phase
	    if (result == 1) {
		// we can ignore any errors here
		// as they'll also become apparent
		// at packetout
		ogg_stream_pagein(&os, &og);
		while (i < 2) {
		    result = ogg_stream_packetout(&os, &op);
		    if (result == 0) return 0;
		    if (result < 0) {
			// Uh oh; data at some point was corrupted or
			// missing! We can't tolerate that in a header.
			// Die.
			KMessageBox::error(widget, i18n(
			    "Corrupt secondary header. Exiting."));
			return -1;
		    }
		    vorbis_synthesis_headerin(&vi, &vc, &op);
		    i++;
		}
	    }
	}

	// no harm in not checking before adding more
	buffer = ogg_sync_buffer(&oy, 4096);
	bytes = m_source->readBlock(buffer, 4096);
	if (bytes == 0 && i < 2) {
	    KMessageBox::error(widget, i18n(
	        "End of file before finding all Vorbis headers!"));
	    return -1;
	}
	ogg_sync_wrote(&oy, bytes);
    }

    // Throw the comments plus a few lines about the bitstream we're decoding
    {
	char **ptr = vc.user_comments;
	while (*ptr) {
	    debug("%s\n", *ptr);
	    ++ptr;
	}
	debug("Bitstream is %d channel, %ldHz", vi.channels, vi.rate);
	debug("Encoded by: %s\n\n", vc.vendor);
    }

    convsize = 4096 / vi.channels;

    // OK, got and parsed all three headers. Initialize the Vorbis
    // packet->PCM decoder. */
    vorbis_synthesis_init(&vd, &vi); // central decode state
    vorbis_block_init(&vd, &vb);     // local state for most of the decode
                                     // so multiple block decodes can
                                     // proceed in parallel.  We could init
                                     // multiple vorbis_block structures
                                     // for vd here
    return 1;
}

//***************************************************************************
bool OggDecoder::open(QWidget *widget, QIODevice &src)
{
    info().clear();
    ASSERT(!m_source);
    if (m_source) warning("OggDecoder::open(), already open !");

    // try to open the source
    if (!src.open(IO_ReadOnly)) {
	warning("failed to open source !");
	return false;
    }

    // take over the source
    m_source = &src;
    
    /********** Decode setup ************/
    debug("--- OggDecoder::open() ---");
    ogg_sync_init(&oy); // Now we can read pages

    // read the header the first time
    if (parseHeader(widget) < 0) return false;

    // get the standard properties
    m_info.setRate(vi.rate);
    m_info.setBits(SAMPLE_BITS);
    m_info.setTracks(vi.channels);
    
    return true;
}

//***************************************************************************
bool OggDecoder::decode(QWidget *widget, MultiTrackWriter &dst)
{
    ASSERT(m_source);
    if (!m_source) return false;

    int eos = 0;

    // we repeat if the bitstream is chained
    while (true) {
	// The rest is just a straight decode loop until end of stream
	while (!eos) {
	    while (!eos) {
		int result = ogg_sync_pageout(&oy, &og);
		if (result == 0) break; // need more data
		if (result < 0) {
		    // missing or corrupt data at this page position
		    KMessageBox::error(widget, i18n(
		        "Corrupt or missing data in bitstream; continuing..."
		        ));
		} else {
		    // can safely ignore errors at this point
		    ogg_stream_pagein(&os,&og); 
		    while (1) {
			result = ogg_stream_packetout(&os, &op);
			
			if (result == 0) break; // need more data
			if (result < 0) {
			    // missing or corrupt data at this page position
			    // no reason to complain; already complained above
			} else {
			    // we have a packet.  Decode it
			    float **pcm;
			    int samples;
			
			    // test for success!
			    if (vorbis_synthesis(&vb, &op) == 0)
			        vorbis_synthesis_blockin(&vd, &vb);

			    // **pcm is a multichannel float vector. In
			    // stereo, for example, pcm[0] is left, and
			    // pcm[1] is right.  samples is the size of
			    // each channel.  Convert the float values
			    // (-1.<=range<=1.) to whatever PCM format
			    // and write it out
			    while ((samples = vorbis_synthesis_pcmout(
			           &vd, &pcm)) > 0)
			    {
				int j;
				int clipflag = 0;
				int bout = (samples <
				           convsize ? samples : convsize);
				
				// convert floats to 16 bit signed ints
				// (host order) and interleave
				for (i=0; i < vi.channels; i++) {
				    ogg_int16_t *ptr=convbuffer+i;
				    float *mono=pcm[i];
				    for (j=0; j < bout; j++) {
#if 1
					int val = mono[j] * 32767.f;
#else /* optional dither */
					int val = mono[j] * 32767.f+drand48()-0.5f;
#endif
					// might as well guard against clipping
					if (val > 32767) {
					    val = 32767;
					    clipflag = 1;
					}
					if (val < -32768) {
					    val=-32768;
					    clipflag=1;
					}
					*ptr = val;
					ptr += vi.channels;
				    }
				}
				
				if (clipflag)
				    debug("Clipping in frame %ld",
				        (long)(vd.sequence));
				
				// fwrite(convbuffer, 2*vi.channels, bout, stdout);
				
				vorbis_synthesis_read(&vd, bout);
				// tell libvorbis how many samples we
				// actually consumed
			    }
			}
		    }
		    if (ogg_page_eos(&og)) eos = 1;
		}
	    }
	
	    if (!eos) {
		buffer = ogg_sync_buffer(&oy, 4096);
		bytes = m_source->readBlock(buffer, 4096);
		ogg_sync_wrote(&oy,bytes);
		if (bytes == 0) eos = 1;
	    }
	}

	// clean up this logical bitstream; before exit we see if we're
	// followed by another [chained]
	ogg_stream_clear(&os);

	// ogg_page and ogg_packet structs always point to storage in
	// libvorbis.  They're never freed or manipulated directly
	vorbis_block_clear(&vb);
	vorbis_dsp_clear(&vd);
	vorbis_comment_clear(&vc);
	vorbis_info_clear(&vi);  // must be called last

	// parse the next header, maybe we parse a stream or chain...
	if (parseHeader(widget) < 1) break;
    }

    // OK, clean up the framer
    ogg_sync_clear(&oy);

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
