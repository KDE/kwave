/*************************************************************************
          OpusDecoder.h  -  sub decoder for Opus in an Ogg container
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

#ifndef _OPUS_DECODER_H_
#define _OPUS_DECODER_H_

#include "config.h"
#ifdef HAVE_OGG_OPUS

#include <ogg/ogg.h>
#include <opus/opus.h>
#include <opus/opus_multistream.h>

#include "libkwave/FileInfo.h"
#include "libkwave/Sample.h"
#include "libkwave/VorbisCommentMap.h"

#include "OggSubDecoder.h"
#include "OpusHeader.h"

class QIODevice;
class QWidget;

namespace Kwave
{
    class MultiWriter;
    class StreamObject;

    class OpusDecoder: public Kwave::OggSubDecoder
    {
    public:
	/**
	 * Constructor
	 *
	 * @param source pointer to a QIODevice to read from, must not be null
	 * @param oy sync and verify incoming physical bitstream
	 * @param os take physical pages, weld into a logical stream of packets
	 * @param og one Ogg bitstream page, Opus packets are inside
	 * @param op one raw packet of data for decode
	 */
	OpusDecoder(QIODevice *source,
                    ogg_sync_state &oy,
	            ogg_stream_state &os,
	            ogg_page &og,
	            ogg_packet &op);

	/** destructor */
	virtual ~OpusDecoder() {}

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
	 * Parses an Ogg comment into a into Kwave FileInfo.
	 * If more than one occurance is found, they are concatenated as a
	 * semicolon separated list.
	 * @param info the file info object to add the value
	 * @param comment string with the full comment, assumed "tag=value"
	 */
	void parseComment(Kwave::FileInfo &info, const QString &comment);

	/**
	 * parse the "OpusHeader" header of the stream
	 * @param widget a QWidget to be used as parent for error messages
	 * @param info reference to a FileInfo to fill
	 * @return -1 if failed or +1 if succeeded
	 */
	virtual int parseOpusHead(QWidget *widget, Kwave::FileInfo &info);

	/**
	 * parse the "OpusTags" header of the stream
	 * @param widget a QWidget to be used as parent for error messages
	 * @param info reference to a FileInfo to fill
	 * @return -1 if failed or +1 if succeeded
	 */
	virtual int parseOpusTags(QWidget *widget, Kwave::FileInfo &info);

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

	/** one Ogg bitstream page.  Opus packets are inside */
	ogg_page &m_og;

	/** one raw packet of data for decode */
	ogg_packet &m_op;

	/** the Opus stream header */
	Kwave::opus_header_t m_opus_header;

	/** Opus multistream decoder object */
	OpusMSDecoder *m_opus_decoder;

	/** map for translating Opus comments to Kwave FileInfo */
	Kwave::VorbisCommentMap m_comments_map;

	/** buffer for decoded raw audio data */
	float *m_buffer;

#ifdef HAVE_SAMPLERATE_SUPPORT
	/** adapter for connecting the sample rate converter (when needed) */
	Kwave::MultiWriter *m_adapter;

	/** sample rate converter (when needed) */
	Kwave::StreamObject *m_rate_converter;

	/**
	 * if true, the output of the rate converter has been connected
	 * to the decoder's sink
	 */
	bool m_converter_connected;
#endif /* HAVE_SAMPLERATE_SUPPORT */
    };
}

#endif /* HAVE_OGG_OPUS */

#endif /* _OPUS_DECODER_H_ */
