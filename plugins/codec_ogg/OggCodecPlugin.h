/*************************************************************************
       OggCodecPlugin.h  -  import/export of audio in an Ogg container
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

#ifndef OGG_CODEC_PLUGIN_H
#define OGG_CODEC_PLUGIN_H

#include "config.h"

#include "libkwave/CodecPlugin.h"
#include "libkwave/Compression.h"

namespace Kwave
{

    class OggCodecPlugin: public Kwave::CodecPlugin
    {
	Q_OBJECT
    public:

	/**
	 * Constructor
	 * @param parent reference to our plugin manager
	 * @param args argument list [unused]
	 */
	OggCodecPlugin(QObject *parent, const QVariantList &args);

	/** Destructor */
	virtual ~OggCodecPlugin();

	/** Creates a new decoder */
	virtual QList<Kwave::Decoder *> createDecoder();

	/** Creates a new encoder */
	virtual QList<Kwave::Encoder *> createEncoder();

    private:
	/** static codec container */
	static CodecPlugin::Codec m_codec;
    };
}

#define REGISTER_OGG_OPUS_MIME_TYPES                                     \
    /* Ogg audio, as per RFC5334, RFC4288 and RFC4855 */                 \
    addMimeType(                                                         \
        "audio/ogg, application/ogg, audio/opus",                        \
        i18n("Ogg Opus audio"),                                          \
        "*.opus"                                                         \
    );

#define REGISTER_OGG_VORBIS_MIME_TYPES                                   \
    addMimeType(                                                         \
        "audio/ogg, audio/x-ogg, application/x-ogg, audio/x-vorbis+ogg", \
        i18n("Ogg Vorbis audio"),                                        \
        "*.ogg"                                                          \
    );

#define REGISTER_COMPRESSION_TYPE_OGG_OPUS \
    addCompression(Kwave::Compression::OGG_OPUS);

#define REGISTER_COMPRESSION_TYPE_OGG_VORBIS \
    addCompression(Kwave::Compression::OGG_VORBIS);

#define DEFAULT_MIME_TYPE "audio/ogg"

#endif /* OGG_CODEC_PLUGIN_H */

//***************************************************************************
//***************************************************************************
