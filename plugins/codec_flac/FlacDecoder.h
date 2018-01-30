/*************************************************************************
          FlacDecoder.h  -  decoder for FLAC data
                             -------------------
    begin                : Tue Feb 28 2004
    copyright            : (C) 2004 by Thomas Eschenbacher
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

#ifndef FLAC_DECODER_H
#define FLAC_DECODER_H

#include "config.h"

#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>

#include <FLAC++/decoder.h>
#include <FLAC++/metadata.h>
#include <FLAC/format.h>

#include "libkwave/Decoder.h"
#include "libkwave/FileInfo.h"
#include "libkwave/VorbisCommentMap.h"

class QWidget;
class QIODevice;

namespace Kwave
{
    class FlacDecoder: public Kwave::Decoder,
                       protected FLAC::Decoder::Stream
    {
    public:
	/** Constructor */
	FlacDecoder();

	/** Destructor */
	virtual ~FlacDecoder();

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
	 * Parse information about the stream, like sample rate, resolution,
	 * channels etc...
	 *
	 * @param stream_info FLAC metadata with stream information
	 */
	void parseStreamInfo(const FLAC::Metadata::StreamInfo &stream_info);

	/**
	 * Parse vorbis comments
	 *
	 * @param vorbis_comments list of vorbis comments, can be empty
	 */
	void parseVorbisComments(
	    const FLAC::Metadata::VorbisComment &vorbis_comments);

	/**
	 * FLAC decoder interface: read callback.
	 *
	 * @param buffer the byte buffer to be filled
	 * @param bytes pointer with the number of bytes to be read,
	 *        can be modified
	 * @return read state
	 */
        virtual ::FLAC__StreamDecoderReadStatus read_callback(
	    FLAC__byte buffer[], size_t *bytes) Q_DECL_OVERRIDE;

	/**
	 * FLAC decoder interface: write callback.
	 *
	 * @param frame a FLAC frame structure
	 * @param buffer a buffer that contains the decoded samples
	 * @return FLAC stream decoder write state
	 */
        virtual ::FLAC__StreamDecoderWriteStatus write_callback(
	    const ::FLAC__Frame *frame,
	    const FLAC__int32 *const buffer[]) Q_DECL_OVERRIDE;

	/**
	 * FLAC decoder interface: callback for processing meta data.
	 *
	 * @param metadata the FLAC meta data to be parsed
	 */
        virtual void metadata_callback(const ::FLAC__StreamMetadata *metadata)
            Q_DECL_OVERRIDE;

	/**
	 * FLAC decoder interface: error callback.
	 *
	 * @param status the FLAC status
	 */
        virtual void error_callback(::FLAC__StreamDecoderErrorStatus status)
            Q_DECL_OVERRIDE;

    private:

	/** source of the audio data */
	QIODevice *m_source;

	/** destination of the audio data */
	Kwave::MultiWriter *m_dest;

	/** map for translating vorbis comments to FileInfo properties */
	Kwave::VorbisCommentMap m_vorbis_comment_map;

    };
}

#endif /* FLAC_DECODER_H */

//***************************************************************************
//***************************************************************************
