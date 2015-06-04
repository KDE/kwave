/*************************************************************************
       VorbisDecoder.h  -  sub decoder for Vorbis in an Ogg container
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

#ifndef VORBIS_DECODER_H
#define VORBIS_DECODER_H

#ifdef HAVE_OGG_VORBIS
#include "config.h"

#include <vorbis/codec.h>

#include "libkwave/FileInfo.h"
#include "libkwave/Sample.h"

#include "OggSubDecoder.h"

class QIODevice;
class QWidget;

namespace Kwave
{
    class VorbisDecoder: public Kwave::OggSubDecoder
    {
    public:
	/**
	 * Constructor
	 *
	 * @param source pointer to a QIODevice to read from, must not be null
	 * @param oy sync and verify incoming physical bitstream
	 * @param os take physical pages, weld into a logical stream of packets
	 * @param og one Ogg bitstream page, Vorbis packets are inside
	 * @param op one raw packet of data for decode
	 */
	explicit VorbisDecoder(QIODevice *source,
                               ogg_sync_state &oy,
	                       ogg_stream_state &os,
	                       ogg_page &og,
	                       ogg_packet &op);

	/** destructor */
	virtual ~VorbisDecoder() {}

	/**
	 * parse the header of the stream and initialize the decoder
	 * @param widget a QWidget to be used as parent for error messages
	 * @param info reference to a FileInfo to fill
	 * @return -1 if failed or +1 if succeeded
	 */
	virtual int open(QWidget *widget, Kwave::FileInfo &info);

	/**
	 * decode received ogg data
	 * @param dst a MultiWriter to be used as sink
	 * @return -1 if failed or >= 0 if succeeded
	 */
	virtual int decode(Kwave::MultiWriter &dst);

	/** reset the stream info */
	virtual void reset();

	/**
	 * finish the decoding, last chance to fix up some file info
	 * @param info reference to a FileInfo to fill
	 */
	virtual void close(Kwave::FileInfo &info);

    protected:

	/**
	 * Searches for a vorbis comment and renders it into Kwave's FileInfo.
	 * If more than one occurrence is found, they are concatenated as a
	 * semicolon separated list.
	 * @param info the file info object to add the tag value
	 * @param tag name of the field to search for
	 * @param property specifies the FileProperty for storing the result
	 */
	void parseTag(Kwave::FileInfo &info, const char *tag,
	              Kwave::FileProperty property);

    private:
	/** IO device to read from */
	QIODevice *m_source;

	/** first stream with audio data */
	qint64 m_stream_start_pos;

	/** last known position in the output stream [samples] */
	sample_index_t m_samples_written;

	/** sync and verify incoming physical bitstream */
	ogg_sync_state &m_oy;

	/** take physical pages, weld into a logical stream of packets */
	ogg_stream_state &m_os;

	/** one Ogg bitstream page.  Vorbis packets are inside */
	ogg_page &m_og;

	/** one raw packet of data for decode */
	ogg_packet &m_op;

	/** struct that stores all the static vorbis bitstream settings */
	vorbis_info m_vi;

	/** struct that stores all the bitstream user comments */
	vorbis_comment m_vc;

	/** central working state for the packet->PCM decoder */
	vorbis_dsp_state m_vd;

	/** local working space for packet->PCM decode */
	vorbis_block m_vb;
    };
}

#endif /* HAVE_OGG_VORBIS */

#endif /* VORBIS_DECODER_H */
