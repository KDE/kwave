/*************************************************************************
      FlacCodecPlugin.h  -  import/export of FLAC data
                             -------------------
    begin                : Tue Feb 28 2004
    copyright            : (C) 2004 by Thomas Eschenbacher
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

#ifndef _FLAC_CODEC_PLUGIN_H_
#define _FLAC_CODEC_PLUGIN_H_

#include "config.h"

#include "libkwave/CodecPlugin.h"
#include "libkwave/Compression.h"

class QStringList;

namespace Kwave
{

    class FlacCodecPlugin: public Kwave::CodecPlugin
    {
	Q_OBJECT
    public:

	/** Constructor */
	explicit FlacCodecPlugin(Kwave::PluginManager &plugin_manager);

	/** Destructor */
	virtual ~FlacCodecPlugin();

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

#define REGISTER_MIME_TYPES { \
    /* included in KDE: */ \
    addMimeType("audio/x-flac", i18n("FLAC audio"), "*.flac; *.FLAC"); \
}

#define REGISTER_COMPRESSION_TYPES { \
    addCompression(Kwave::Compression::FLAC);   \
}

#define DEFAULT_MIME_TYPE "audio/x-flac"

#endif /* _FLAC_CODEC_PLUGIN_H_ */

//***************************************************************************
//***************************************************************************
