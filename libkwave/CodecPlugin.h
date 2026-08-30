/*************************************************************************
          CodecPlugin.h  -  base class for codec plugins
                             -------------------
    begin                : Fri Dec 28 2012
    copyright            : (C) 2012 by Thomas Eschenbacher
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

#ifndef CODEC_PLUGIN_H
#define CODEC_PLUGIN_H

#include "config.h"
#include "libkwave_export.h"

#include <QtGlobal>
#include <QList>

#include "libkwave/Decoder.h"
#include "libkwave/Encoder.h"
#include "libkwave/Plugin.h"

namespace Kwave
{
    class LIBKWAVE_EXPORT CodecPlugin: public Kwave::Plugin
    {
    public:

        /** container for codecs */
        typedef struct {
            int                      m_use_count; /**< use count        */
            QList<Encoder::Instance> m_encoder;   /**< list of encoders */
            QList<Decoder::Instance> m_decoder;   /**< list of decoders */
        } Codec;

        /**
         * Constructor
         * @param parent pointer to the corresponding plugin manager
         * @param args argument list, contains internal meta data
         * @param codec reference to a static container for the codec
         */
        CodecPlugin(QObject *parent, const QVariantList &args, Codec &codec);

        /** Destructor */
        ~CodecPlugin() override;

        /**
         * Gets called when the plugin is first loaded. Registers new encoder
         * and decoder on first call, all subsequenct calls only increment
         * the reference count of the existing encoder/decoder instances.
         */
        void load(QStringList &/* params */) override;

        /**
         * Gets called before the plugin is unloaded. Decrements the use count
         * of existing encoder/decoder instances and removes them if zero
         * gets reached.
         */
        void unload() override;

        /**
         * Create a new set of decoders
         * @return list of decoders, may be  empty
         */
        virtual QList<Kwave::Decoder::Instance> createDecoder() = 0;

        /**
         * Create a new set of encoders
         * @return list of encoders, may be  empty
         */
        virtual QList<Kwave::Encoder::Instance> createEncoder() = 0;

    protected:

        /**
         * helper template to return a list with a single decoder,
         * for use within createDecoder()
         */
        template<class T> QList<Kwave::Decoder::Instance> singleDecoder()
        {
            QList<Kwave::Decoder::Instance> list;
            list.append(std::make_shared<T>());
            return list;
        }

        /**
         * helper template to return a list with a single encoder,
         * for use within createEncoder()
         */
        template<class T> QList<Kwave::Encoder::Instance> singleEncoder()
        {
            QList<Kwave::Encoder::Instance> list;
            list.append(std::make_shared<T>());
            return list;
        }

    private:

        /** reference to the static container with encoder/decoder/usecount */
        Codec &m_codec;
    };
}

/** initializer for an empty Kwave::CodecPlugin::Codec */
#define EMPTY_CODEC {0, QList<Kwave::Encoder::Instance>(), \
                        QList<Kwave::Decoder::Instance>() }

#endif /* CODEC_PLUGIN_H */

//***************************************************************************
//***************************************************************************
