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
#include <qlist.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qobject.h>

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

};

#endif /* _OGG_DECODER_H_ */
