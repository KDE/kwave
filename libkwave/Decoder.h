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

class QIODevice;
class Signal;

class Decoder: public QObject
{
    Q_OBJECT
public:
    /** Constructor */
    Decoder() {};

    /** Destructor */
    virtual ~Decoder() {};

    /** Returns true if the given mime type is supported */
    virtual bool supports(const KMimeType &mimetype) = 0;

    /** Returns a list of supported mime types */
    virtual QList<KMimeType> mimeTypes() = 0;

    /** Returns a new instance of the decoder */
    virtual Decoder *instance() = 0;

    /**
     * Decodes a stream of bytes into a signal
     * @param src file or other source with a stream of bytes
     * @param dst Signal that receives the audio data
     * @return true if succeeded, false on errors
     */
    virtual bool decode(QIODevice &src, Signal &dst) = 0;

};

#endif /* _DECODER_H_ */
