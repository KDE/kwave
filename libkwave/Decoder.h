/***************************************************************************
              Decoder.h  -  abstract base class of all decoders
			     -------------------
    begin                : Mar 10 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DECODER_H_
#define _DECODER_H_

#include "config.h"
#include <qlist.h>
#include <qobject.h>

#include "CodecBase.h"
#include "FileInfo.h"

class FileInfo;
class KMimeType;
class MultiTrackWriter;
class QIODevice;
class QWidget;

class Decoder: public QObject, public CodecBase
{
    Q_OBJECT
public:
    /** Constructor */
    Decoder() :QObject(), CodecBase() {};

    /** Destructor */
    virtual ~Decoder() {};

    /** Returns a new instance of the decoder */
    virtual Decoder *instance() = 0;

    /**
     * Opens the source and decodes the header information.
     * @param source file or other source with a stream of bytes
     * @return true if succeeded, false on errors
     */
    virtual bool open(QIODevice &source) = 0;

    /**
     * Decodes a stream of bytes into a signal
     * @param widget a widget that can be used for displaying
     *        message boxes or dialogs
     * @param dst Signal that receives the audio data
     * @return true if succeeded, false on errors
     */
    virtual bool decode(QWidget *widget, MultiTrackWriter &dst) = 0;

    /**
     * Closes the io device.
     */
    virtual void close() = 0;

    /**
     * Returns the information of the file, only valid after
     * open() has successfully been called.
     */
    virtual inline FileInfo &info() { return m_info;};

protected:

    /** information about the file */
    FileInfo m_info;

};

#endif /* _DECODER_H_ */
