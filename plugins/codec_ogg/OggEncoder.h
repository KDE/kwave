/*************************************************************************
          OggEncoder.h  -  encoder for Ogg/Vorbis data
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

#ifndef OGG_ENCODER_H
#define OGG_ENCODER_H

#include "config.h"

#include <QList>

#include "libkwave/Encoder.h"
#include "libkwave/VorbisCommentMap.h"

class QWidget;

namespace Kwave
{

    class OggEncoder: public Kwave::Encoder
    {
    public:
        /** Constructor */
        OggEncoder();

        /** Destructor */
        ~OggEncoder() override;

        /** Returns a new instance of the encoder */
        Kwave::Encoder *instance() override;

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
                            const Kwave::MetaDataList &meta_data)
            override;

        /** Returns a list of supported file properties */
        virtual QList<Kwave::FileProperty> supportedProperties()
            override;

    private:

        /** map for translating Opus comments to Kwave FileInfo */
        Kwave::VorbisCommentMap m_comments_map;
    };
}

#endif /* OGG_ENCODER_H */

//***************************************************************************
//***************************************************************************
