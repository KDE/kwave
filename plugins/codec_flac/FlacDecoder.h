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

#ifndef _FLAC_DECODER_H_
#define _FLAC_DECODER_H_

#include "config.h"
#include <qptrlist.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qobject.h>

#include <FLAC/format.h>
#include <FLAC++/decoder.h>
#include <FLAC++/metadata.h>

#include "libkwave/Decoder.h"
#include "libkwave/FileInfo.h"

#include "VorbisCommentMap.h"

class MultiTrackWriter;

class FlacDecoder: public Decoder,
                   protected FLAC::Decoder::Stream
{
public:
    /** Constructor */
    FlacDecoder();

    /** Destructor */
    virtual ~FlacDecoder();

    /** Returns a new instance of the decoder */
    virtual Decoder *instance();

    /**
     * Opens the source and decodes the header information.
     * @param widget a widget that can be used for displaying
     *        message boxes or dialogs
     * @param source file or other source with a stream of bytes
     * @return true if succeeded, false on errors
     */
    virtual bool open(QWidget *widget, QIODevice &source);

    /**
     * Decodes a stream of bytes into a MultiTrackWriter
     * @param widget a widget that can be used for displaying
     *        message boxes or dialogs
     * @param dst MultiTrackWriter that receives the audio data
     * @return true if succeeded, false on errors
     */
    virtual bool decode(QWidget *widget, MultiTrackWriter &dst);

    /**
     * Closes the source.
     */
    virtual void close();

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
     * @param vendor_string vorbis comment with a vendor name
     * @param comments list of vorbis comments, can be empty
     * @param count number of vorbis comments in comments_list
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
        FLAC__byte buffer[], unsigned int *bytes);

    /**
     * FLAC decoder interface: write callback.
     *
     * @param frame a FLAC frame structure
     * @param buffer a buffer that contains the decoded samples
     * @return FLAC stream decoder write state
     */
    virtual ::FLAC__StreamDecoderWriteStatus write_callback(
        const ::FLAC__Frame *frame,
	const FLAC__int32 *const buffer[]);

    /**
     * FLAC decoder interface: callback for processing meta data.
     *
     * @param metadata the FLAC meta data to be parsed
     */
    virtual void metadata_callback(const ::FLAC__StreamMetadata *metadata);

    /**
     * FLAC decoder interface: error callback.
     *
     * @param status the FLAC status
     */
    virtual void error_callback(::FLAC__StreamDecoderErrorStatus status);

private:

    /** source of the audio data */
    QIODevice *m_source;

    /** destination of the audio data */
    MultiTrackWriter *m_dest;

    /** buffer for reading from the QIODevice */
    char *m_buffer;

    /** map for translating vorbis comments to FileInfo properties */
    VorbisCommentMap m_vorbis_comment_map;

};

#endif /* _FLAC_DECODER_H_ */
