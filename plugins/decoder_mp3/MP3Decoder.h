/*************************************************************************
           MP3Decoder.h  -  decoder for MP3 data
                             -------------------
    begin                : Wed Aug 07 2002
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

#ifndef _MP3_DECODER_H_
#define _MP3_DECODER_H_

#include "config.h"
#include <qstring.h>
#include "libkwave/Decoder.h"
#include "libkwave/FileInfo.h"

#include "libmad/mad.h"

class Mp3_Headerinfo;
class ID3_Frame;
class ID3_Tag;

class MP3Decoder: public Decoder
{
public:

    /** Constructor */
    MP3Decoder();

    /** Destructor */
    virtual ~MP3Decoder();

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

    /* Callback for filling libmad's input buffer */
    enum mad_flow fillInput(struct mad_stream *stream);

    /* Calback for processing libmad's output */
    enum mad_flow processOutput(void *data,
                                struct mad_header const *header,
                                struct mad_pcm *pcm);

private:

    /** parse MP3 headers */
    bool parseMp3Header(const Mp3_Headerinfo &header, QWidget *widget);
    
    /** parse all known ID3 tags */
    bool parseID3Tags(ID3_Tag &tag);

    /** parse a FileInfo property from a ID3 frame */
    void parseId3Frame(ID3_Frame *frame, FileProperty property);

private:

    /** source of the raw mp3 data */
    QIODevice *m_source;

    /** destination of the audio data */
    MultiTrackWriter *m_dest;

    /** buffer for libmad */
    unsigned char *m_buffer;

    /** size of m_buffer in bytes */
    int m_buffer_size;

    /** number of prepended bytes / id3v2 tag */
    unsigned int m_prepended_bytes;

    /** number of appended bytes / id3v1 tag */
    unsigned int m_appended_bytes;

};

#endif /* _MP3_DECODER_H_ */
