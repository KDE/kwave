/*************************************************************************
       MP3CodecPlugin.h  -  import and export of MP3 data
                             -------------------
    begin                : Mon May 28 2012
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

#ifndef MP3_CODEC_PLUGIN_H
#define MP3_CODEC_PLUGIN_H

#include "config.h"

#include "libkwave/CodecPlugin.h"
#include "libkwave/Compression.h"

namespace Kwave
{

    class MP3CodecPlugin: public Kwave::CodecPlugin
    {
        Q_OBJECT
    public:

        /**
         * Constructor
         * @param parent reference to our plugin manager
         * @param args argument list [unused]
         */
        MP3CodecPlugin(QObject *parent, const QVariantList &args);

        /** Destructor */
        ~MP3CodecPlugin() override;

        /** @see Kwave::Plugin::load() */
        void load(QStringList &params) override;

        /**
         * Shows a dialog to set up the plugin, configure all paths,
         * presets and other parameters...
         * @param previous_params the parameters of a previous call
         * @return a string list with all parameters or null if the
         *         setup (dialog) has been canceled
         */
        virtual QStringList *setup(QStringList &previous_params)
            override;

        /** Creates a new decoder */
        QList<Kwave::Decoder *> createDecoder() override;

        /** Creates a new encoder */
        QList<Kwave::Encoder *> createEncoder() override;

    private:
        /** static codec container */
        static CodecPlugin::Codec m_codec;
    };

}

/* see RFC3003 */
#define REGISTER_MIME_TYPES {                           \
    addMimeType(                                        \
        "audio/x-mp3, audio/mpeg",                      \
        i18n("MPEG layer III audio"),                   \
        "*.mp3"                                         \
    );                                                  \
                                                        \
    addMimeType(                                        \
        "audio/mpeg, audio/x-mp2",                      \
        i18n("MPEG layer II audio"),                    \
        "*.mp2"                                         \
    );                                                  \
                                                        \
    addMimeType(                                        \
        "audio/mpeg, audio/x-mpga",                     \
        i18n("MPEG layer I audio"),                     \
        "*.mpga *.mpg *.mp1"                            \
    );                                                  \
}

#define REGISTER_COMPRESSION_TYPES {                    \
    addCompression(Kwave::Compression::MPEG_LAYER_I);   \
    addCompression(Kwave::Compression::MPEG_LAYER_II);  \
    addCompression(Kwave::Compression::MPEG_LAYER_III); \
}

#endif /* MP3_CODEC_PLUGIN_H */

//***************************************************************************
//***************************************************************************
