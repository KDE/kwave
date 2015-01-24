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

#ifndef _OGG_CODEC_PLUGIN_H_
#define _OGG_CODEC_PLUGIN_H_

#include <config.h>

#include "libkwave/CodecPlugin.h"
#include "libkwave/Compression.h"

namespace Kwave
{

    class OggCodecPlugin: public Kwave::CodecPlugin
    {
	Q_OBJECT
    public:

	/** Constructor */
	OggCodecPlugin(Kwave::PluginManager &plugin_manager);

	/** Destructor */
	virtual ~OggCodecPlugin();

	/** Returns the name of the plugin. */
	virtual QString name() const;

	/** Creates a new decoder instance */
	virtual Kwave::Decoder *createDecoder();

	/** Creates a new encoder instance */
	virtual Kwave::Encoder *createEncoder();

    private:
	/** static codec container */
	static CodecPlugin::Codec m_codec;
    };
}

#define REGISTER_OGG_OPUS_MIME_TYPES                                           \
    /* Ogg audio, as per RFC5334 */                                            \
    addMimeType("audio/ogg",         i18n("Ogg Opus audio"),   "*.opus");      \
    addMimeType("application/ogg",   i18n("Ogg Opus audio"),   "*.opus");      \
    /* Ogg audio, as per RFC4288 and RFC4855 */                                \
    addMimeType("audio/opus",        i18n("Ogg Opus audio"),   "*.opus");      \

#define REGISTER_OGG_VORBIS_MIME_TYPES                                         \
    /* original from Ogg Vorbis documentation: */                              \
    addMimeType("audio/ogg",         i18n("Ogg Vorbis audio"), "*.ogg");       \
    addMimeType("audio/x-ogg",       i18n("Ogg Vorbis audio"), "*.ogg");       \
    /* included in KDE: */                                                     \
    addMimeType("application/x-ogg", i18n("Ogg Vorbis audio"), "*.ogg");       \
    /* found in KDE 4: */                                                      \
    addMimeType("audio/x-vorbis+ogg",i18n("Ogg Vorbis audio"), "*.ogg");

#define REGISTER_COMPRESSION_TYPE_OGG_OPUS \
    addCompression(Kwave::Compression::OGG_OPUS);

#define REGISTER_COMPRESSION_TYPE_OGG_VORBIS \
    addCompression(Kwave::Compression::OGG_VORBIS);

#define DEFAULT_MIME_TYPE "audio/ogg"

#endif /* _OGG_CODEC_PLUGIN_H_ */

//***************************************************************************
//***************************************************************************
