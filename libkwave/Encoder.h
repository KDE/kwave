/***************************************************************************
              Encoder.h  -  abstract base class of all encoders
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

#ifndef _ENCODER_H_
#define _ENCODER_H_

#include "config.h"
#include <qlist.h>
#include <qobject.h>

class QIODevice;
class Signal;

class Encoder: public QObject
{
    Q_OBJECT
public:
    /** Constructor */
    Encoder() {};

    /** Destructor */
    virtual ~Encoder() {};

    /** Returns true if the given mime type is supported */
    virtual bool supports(const KMimeType &mimetype) = 0;

    /** Returns a list of supported mime types */
    virtual QList<KMimeType> mimeTypes() = 0;

    /** Returns a new instance of the encoder */
    virtual Encoder *instance() = 0;

    /**
     * Encodes a signal into a stream of bytes.
     * @param src Signal that contains the audio data
     * @param dst file or other source to receive a stream of bytes
     * @return true if succeeded, false on errors
     */
    virtual bool encode(Signal &src, QIODevice &dst) = 0;
};

#endif /* _ENCODER_H_ */
