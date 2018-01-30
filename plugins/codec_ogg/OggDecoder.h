/*************************************************************************
          OggDecoder.h  -  decoder for Ogg/Vorbis data
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

#ifndef OGG_DECODER_H
#define OGG_DECODER_H

#include "config.h"

#include <ogg/ogg.h>

#include "libkwave/Decoder.h"
#include "libkwave/FileInfo.h"

namespace Kwave
{
    class OggSubDecoder;

    class OggDecoder: public Kwave::Decoder
    {
    public:
	/** Constructor */
	OggDecoder();

	/** Destructor */
	virtual ~OggDecoder();

	/** Returns a new instance of the decoder */
        virtual Kwave::Decoder *instance() Q_DECL_OVERRIDE;

	/**
	 * Opens the source and decodes the header information.
	 * @param widget a widget that can be used for displaying
	 *        message boxes or dialogs
	 * @param source file or other source with a stream of bytes
	 * @return true if succeeded, false on errors
	 */
        virtual bool open(QWidget *widget, QIODevice &source) Q_DECL_OVERRIDE;

	/**
	 * Decodes a stream of bytes into a MultiWriter
	 * @param widget a widget that can be used for displaying
	 *        message boxes or dialogs
	 * @param dst MultiWriter that receives the audio data
	 * @return true if succeeded, false on errors
	 */
        virtual bool decode(QWidget *widget, Kwave::MultiWriter &dst)
            Q_DECL_OVERRIDE;

	/**
	 * Closes the source.
	 */
        virtual void close() Q_DECL_OVERRIDE;

    protected:

	/**
	 * Try to parse header frames.
	 * @param widget a QWidget for displaying error messages
	 * @return -1 for error (return false)
	 *          1 if ready to continue
	 */
	int parseHeader(QWidget *widget);

    private:

	/** sub decoder, can be Vorbis, Opus, Speex or whatever... */
	Kwave::OggSubDecoder *m_sub_decoder;

	/** source of the audio data */
	QIODevice *m_source;

	/** sync and verify incoming physical bitstream */
	ogg_sync_state m_oy;

	/** take physical pages, weld into a logical stream of packets */
	ogg_stream_state m_os;

	/** one Ogg bitstream page.  Vorbis packets are inside */
	ogg_page m_og;

	/** one raw packet of data for decode */
	ogg_packet m_op;

    };
}

#endif /* OGG_DECODER_H */

//***************************************************************************
//***************************************************************************
