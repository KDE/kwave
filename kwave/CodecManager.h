/***************************************************************************
         CodecManager.h  -  manager for Kwave's coders and decoders
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

#ifndef _CODEC_MANAGER_H_
#define _CODEC_MANAGER_H_

#include "config.h"
#include <qlist.h>
#include <qstring.h>
#include <qstringlist.h>
#include <kmimetype.h>

class Decoder;
class Encoder;

class CodecManager: public QObject
{
    Q_OBJECT
public:
    /** Constructor */
    CodecManager();

    /** Destructor */
    virtual ~CodecManager();

    /**
     * Registers a new encoder.
     * @param encoder a KwaveEncoder that will be used as template for
     *                creating new encoder instances (used as factory)
     */
    static void registerEncoder(const Encoder &encoder);

    /**
     * Registers a new decoder.
     * @param decoder a KwaveDecoder that will be used as template for
     *                creating new decoder instances (used as factory)
     */
    static void registerDecoder(const Decoder &decoder);

    /**
     * Tries to find a decoder that matches to a given mime type.
     * @param mimetype mime type of the source
     * @return a new decoder for the mime type or null if none found.
     */
    static Decoder *decoder(const KMimeType &mimetype);

    /**
     * Tries to find an encoder that matches to a given mime type.
     * @param mimetype mime type of the destination
     * @return a new encoder for the mime type or null if none found.
     */
    static Encoder *encoder(const KMimeType &mimetype);

//    /**
//     * Tries to guess the mime type of a file by inspecting the file name
//     * and the first 128 bytes of the file.
//     * @param filename or URL of the file
//     * @param data array with the first 128 bytes of the file
//     * @return mime type of the file or "unknown/unknown" detection failed
//     */
//    QCString guessMimeType(const QString &filename, const QByteArray &data);

//private slots:
//
//    /** connected to each encoder for automatic de-registration */
//    void encoderDeleted(KwaveEncoder *encoder);
//
//    /** connected to each encoder for automatic de-registration */
//    void decoderDeleted(KwaveDecoder *decoder);

private:
    /** list of all encoders */
    static QList<Encoder> m_encoders;

    /** list of decoders */
    static QList<Decoder> m_decoders;
};

#endif /* _CODEC_MANAGER_H_ */
