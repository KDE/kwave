/*************************************************************************
     AsciiCodecPlugin.h  -  import/export of ASCII data
                             -------------------
    begin                : Sun Nov 26 2006
    copyright            : (C) 2006 by Thomas Eschenbacher
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

#ifndef ASCII_CODEC_PLUGIN_H
#define ASCII_CODEC_PLUGIN_H

#include "config.h"

#include "libkwave/CodecPlugin.h"
#include "libkwave/Compression.h"

namespace Kwave
{

    class AsciiCodecPlugin: public Kwave::CodecPlugin
    {
	Q_OBJECT
    public:

	/**
	 * Constructor
	 * @param parent reference to our plugin manager
	 * @param args argument list [unused]
	 */
	AsciiCodecPlugin(QObject *parent, const QVariantList &args);

	/** Destructor */
	virtual ~AsciiCodecPlugin();

	/** Creates a new decoder instance */
	virtual Kwave::Decoder *createDecoder();

	/** Creates a new encoder instance */
	virtual Kwave::Encoder *createEncoder();

    private:
	/** static codec container */
	static CodecPlugin::Codec m_codec;
    };
}

#define LOAD_MIME_TYPES                          \
    addMimeType("audio/x-audio-ascii",           \
        i18n("ASCII encoded audio"), "*.ascii");

#define REGISTER_COMPRESSION_TYPES               \
    addCompression(Kwave::Compression::NONE);

/** prefix used for encoding metadata / properties */
#define META_PREFIX _("## ")

#endif /* ASCII_CODEC_PLUGIN_H */

//***************************************************************************
//***************************************************************************
