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

#ifndef _OGG_DECODER_H_
#define _OGG_DECODER_H_

#include "config.h"

#include <vorbis/codec.h>

#include "libkwave/Decoder.h"
#include "libkwave/FileInfo.h"

class OggDecoder: public Decoder
{
public:
    /** Constructor */
    OggDecoder();

    /** Destructor */
    virtual ~OggDecoder();

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
     * Decodes a stream of bytes into a MultiWriter
     * @param widget a widget that can be used for displaying
     *        message boxes or dialogs
     * @param dst MultiWriter that receives the audio data
     * @return true if succeeded, false on errors
     */
    virtual bool decode(QWidget *widget, Kwave::MultiWriter &dst);

    /**
     * Closes the source.
     */
    virtual void close();

protected:

    /**
     * Searches for a vorbis comment and renders it into Kwave's FileInfo.
     * If more than one occurance is found, they are concatenated as a
     * semicolon separated list.
     * @param info the file info object to add the tag value
     * @param tag name of the field to search for
     * @param property specifies the FileProperty for storing the result
     */
    void parseTag(FileInfo &info, const char *tag, FileProperty property);

    /**
     * Try to parse header frames.
     * @param widget a QWidget for displaying error messages
     * @return -1 for error (return false)
     *         0 for break (no error)
     *         1 if ready to continue
     */
    int parseHeader(QWidget *widget);

private:

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

    /** struct that stores all the static vorbis bitstream settings */
    vorbis_info m_vi;

    /** struct that stores all the bitstream user comments */
    vorbis_comment m_vc;

    /** central working state for the packet->PCM decoder */
    vorbis_dsp_state m_vd;

    /** local working space for packet->PCM decode */
    vorbis_block m_vb;

    /** buffer for reading from the QIODevice */
    char *m_buffer;

};

#endif /* _OGG_DECODER_H_ */
