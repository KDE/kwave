/*************************************************************************
          WavDecoder.h  -  decoder for wav data
                             -------------------
    begin                : Sun Mar 10 2002
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

#ifndef _WAV_DECODER_H_
#define _WAV_DECODER_H_

#include "config.h"
#include <qobject.h>
#include "libkwave/Decoder.h"

class WavDecoder: public Decoder
{
public:
    /** Constructor */
    WavDecoder();

    /** Destructor */
    virtual ~WavDecoder();

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

    /** source of the audio data */
    QIODevice *m_source;

};

#endif /* _WAV_DECODER_H_ */
