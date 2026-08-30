/*************************************************************************
    AudiofileCodecPlugin.h  -  import/export through libaudiofile
                             -------------------
    begin                : Tue May 28 2002
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

#ifndef AUDIOFILE_CODEC_PLUGIN_H
#define AUDIOFILE_CODEC_PLUGIN_H

#include "config.h"
#include "libkwave/CodecPlugin.h"

namespace Kwave
{

    class AudiofileCodecPlugin: public Kwave::CodecPlugin
    {
        Q_OBJECT
    public:

        /**
         * Constructor
         * @param parent reference to our plugin manager
         * @param args argument list [unused]
         */
        AudiofileCodecPlugin(QObject *parent, const QVariantList &args);

        /** Destructor */
        ~AudiofileCodecPlugin() override;

        /** Creates a new decoder */
        QList<Kwave::Decoder::Instance> createDecoder() override;

        /** Creates a new encoder */
        QList<Kwave::Encoder::Instance> createEncoder() override;

    private:
        /** static codec container */
        static CodecPlugin::Codec m_codec;
    };
}

#define REGISTER_MIME_TYPES \
    /* defined in RFC 1521 */                                       \
    addAfMimeType("audio/basic",                                    \
                  i18n("NeXT, Sun Audio"),                          \
                  "*.au; *.snd",                                    \
                  AF_FILE_NEXTSND);                                 \
    /* some others, mime types might be wrong (I found no RFC) */   \
    addAfMimeType("audio/x-8svx",                                   \
                  i18n("Amiga IFF/8SVX Sound File Format"),         \
                  "*.iff; *.8svx",                                  \
                  AF_FILE_IFF_8SVX);                                \
    addAfMimeType("audio/x-aifc",                                   \
                  i18n("Compressed Audio Interchange Format"),      \
                  "*.aifc",                                         \
                  AF_FILE_AIFFC);                                   \
    addAfMimeType("audio/x-aiff", /* included in KDE */             \
                  i18n("Audio Interchange Format"),                 \
                  "*.aif; *.aiff",                                  \
                  AF_FILE_AIFF);                                    \
    addAfMimeType("audio/x-avr",                                    \
                  i18n("Audio Visual Research File Format"),        \
                  "*.avr",                                          \
                  AF_FILE_AVR);                                     \
    addAfMimeType("audio/x-caf",                                    \
                  i18n("Core Audio File Format"),                   \
                  "*.caf",                                          \
                  AF_FILE_CAF);                                     \
    addAfMimeType("audio/x-ircam",                                  \
                  i18n("Berkeley, IRCAM, Carl Sound Format"),       \
                  "*.sf",                                           \
                  AF_FILE_IRCAM);                                   \
    addAfMimeType("audio/x-nist",                                   \
                  i18n("NIST SPHERE Audio File Format"),            \
                  "*.nist",                                         \
                  AF_FILE_NIST_SPHERE);                             \
    addAfMimeType("audio/x-smp",                                    \
                  i18n("Sample Vision Format"),                     \
                  "*.smp",                                          \
                  AF_FILE_SAMPLEVISION);                            \
    addAfMimeType("audio/x-voc",                                    \
                  i18n("Creative Voice"),                           \
                  "*.voc",                                          \
                  AF_FILE_VOC);

#endif /* AUDIOFILE_CODEC_PLUGIN_H */

//***************************************************************************
//***************************************************************************
