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

#ifndef DECODER_H
#define DECODER_H

#include "config.h"

#include <QtGlobal>
#include <QObject>

#include "libkwave/CodecBase.h"
#include "libkwave/MetaDataList.h"

class QIODevice;
class QWidget;

namespace Kwave
{

    class MultiWriter;

    class Q_DECL_EXPORT Decoder: public QObject, public Kwave::CodecBase
    {
        Q_OBJECT
    public:

        /** Constructor */
        Decoder();

        /** Destructor */
        virtual ~Decoder() override;

        /** Returns a new instance of the decoder */
        virtual Decoder *instance() = 0;

        /**
         * Opens the source and decodes the header information.
         * @param widget a widget that can be used for displaying
         *        message boxes or dialogs
         * @param source file or other source with a stream of bytes
         * @return true if succeeded, false on errors
         */
        virtual bool open(QWidget *widget, QIODevice &source) = 0;

        /**
         * Decodes a stream of bytes into a signal
         * @param widget a widget that can be used for displaying
         *        message boxes or dialogs
         * @param dst writer that receives the audio data
         * @return true if succeeded, false on errors
         */
        virtual bool decode(QWidget *widget, Kwave::MultiWriter &dst) = 0;

        /**
         * Closes the io device.
         */
        virtual void close() = 0;

        /**
         * Returns the meta data of the file, only valid after
         * open() has successfully been called.
         */
        virtual inline Kwave::MetaDataList &metaData() { return m_meta_data; }

    signals:

        /**
         * Can be used to signal the current position within the source
         * when a stream without info about the resulting signal is
         * processed.
         * @param pos current position within the source, in bytes!
         */
        void sourceProcessed(quint64 pos);

    protected:

        /** meta data of the file */
        Kwave::MetaDataList m_meta_data;

    };
}

#endif /* DECODER_H */

//***************************************************************************
//***************************************************************************
