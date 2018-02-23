/*************************************************************************
    VorbisEncoder.h  -  sub encoder base class for Vorbis in an Ogg container
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

#ifndef VORBIS_ENCODER_H
#define VORBIS_ENCODER_H

#include "config.h"

#include <vorbis/vorbisenc.h>

#include "libkwave/FileInfo.h"
#include "libkwave/VorbisCommentMap.h"

#include "OggSubEncoder.h"

class QIODevice;
class QWidget;

namespace Kwave
{

    class FileInfo;
    class MultiTrackReader;

    class VorbisEncoder: public Kwave::OggSubEncoder
    {
    public:

	/**
	 * Constructor
	 */
	VorbisEncoder();

	/** Destructor */
        virtual ~VorbisEncoder() Q_DECL_OVERRIDE;

	/**
	 * parse the header of the stream and initialize the decoder
	 * @param widget a QWidget to be used as parent for error messages
	 * @param info reference to a FileInfo to fill
	 * @param src MultiTrackReader used as source of the audio data
	 * @return true if succeeded, false if failed
	 */
        virtual bool open(QWidget *widget,
	                  const Kwave::FileInfo &info,
	                  Kwave::MultiTrackReader &src) Q_DECL_OVERRIDE;

	/**
	 * write the header information
	 * @param dst a QIODevice that receives the raw data
	 * @return true if succeeded, false if failed
	 */
        virtual bool writeHeader(QIODevice &dst) Q_DECL_OVERRIDE;

	/**
	 * encode received ogg data
	 * @param src MultiTrackReader used as source of the audio data
	 * @param dst a QIODevice that receives the raw data
	 * @return true if succeeded, false if failed
	 */
        virtual bool encode(Kwave::MultiTrackReader &src,
	                    QIODevice &dst) Q_DECL_OVERRIDE;

	/**
	 * finished the encoding, clean up
	 */
        virtual void close() Q_DECL_OVERRIDE;

    private:

	/** Encodes all file properties into a vorbis comment */
	void encodeProperties(const Kwave::FileInfo &info);

    private:

	/** map for translating Vorbis comments to Kwave FileInfo */
	Kwave::VorbisCommentMap m_comments_map;

	/** file info, set in open(...) */
	Kwave::FileInfo m_info;

	/** take physical pages, weld into a logical stream of packets */
	ogg_stream_state m_os;

	/** one Ogg bitstream page.  Vorbis packets are inside */
	ogg_page         m_og;

	/** one raw packet of data for decode */
	ogg_packet       m_op;

	/** struct that stores all the static vorbis bitstream settings */
	vorbis_info      m_vi;

	/** struct that stores all the user comments */
	vorbis_comment   m_vc;

	/** central working state for the packet->PCM decoder */
	vorbis_dsp_state m_vd;

	/** local working space for packet->PCM decode */
	vorbis_block     m_vb;
    };
}

#endif /* VORBIS_ENCODER_H */

//***************************************************************************
//***************************************************************************
