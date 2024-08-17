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

#ifndef ENCODER_H
#define ENCODER_H

#include "config.h"
#include "libkwave_export.h"

#include <QtGlobal>
#include <QList>
#include <QObject>

#include "libkwave/CodecBase.h"
#include "libkwave/FileInfo.h"

class QIODevice;
class QWidget;

namespace Kwave
{

    class MultiTrackReader;

    class LIBKWAVE_EXPORT Encoder: public QObject, public Kwave::CodecBase
    {
        Q_OBJECT
    public:
        /** Constructor */
        Encoder();

        /** Destructor */
        ~Encoder() override {}

        /** Returns a new instance of the encoder */
        virtual Encoder *instance() = 0;

        /**
         * Encodes a signal into a stream of bytes.
         * @param widget a widget that can be used for displaying
         *        message boxes or dialogs
         * @param src MultiTrackReader used as source of the audio data
         * @param dst file or other source to receive a stream of bytes
         * @param meta_data meta data of the file to save
         * @return true if succeeded, false on errors
         */
        virtual bool encode(QWidget *widget,
                            Kwave::MultiTrackReader &src,
                            QIODevice &dst,
                            const Kwave::MetaDataList &meta_data) = 0;

        /** Returns a list of supported file properties */
        virtual QList<Kwave::FileProperty> supportedProperties() {
            QList<Kwave::FileProperty> empty;
            return empty;
        }

        /**
         * Returns a list of all properties within a list of properties
         * which are not supported by this encoder
         * @param properties_to_check list of properties to check
         * @return list of unsupported properties (may be empty)
         */
        virtual QList<Kwave::FileProperty> unsupportedProperties(
            const QList<Kwave::FileProperty> &properties_to_check
        );
    };
}

#endif /* ENCODER_H */

//***************************************************************************
//***************************************************************************
