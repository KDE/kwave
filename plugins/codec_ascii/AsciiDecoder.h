/*************************************************************************
         AsciiDecoder.h  -  decoder for ASCII data
                             -------------------
    begin                : Sun Nov 26 2006
    copyright            : (C) 2006 by Thomas Eschenbacher
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

#ifndef _ASCII_DECODER_H_
#define _ASCII_DECODER_H_

#include "config.h"

#include "libkwave/Decoder.h"
#include "libkwave/FileInfo.h"

class QIODevice;
class QWidget;

class AsciiDecoder: public Kwave::Decoder
{
public:
    /** Constructor */
    AsciiDecoder();

    /** Destructor */
    virtual ~AsciiDecoder();

    /** Returns a new instance of the decoder */
    virtual Kwave::Decoder *instance();

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

private:

    /** source of the audio data */
    QIODevice *m_source;

    /** destination of the audio data */
    Kwave::MultiWriter *m_dest;

};

#endif /* _ASCII_DECODER_H_ */
