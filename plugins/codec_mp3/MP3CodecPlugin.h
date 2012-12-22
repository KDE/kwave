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

#ifndef _MP3_CODEC_PLUGIN_H_
#define _MP3_CODEC_PLUGIN_H_

#include "config.h"
#include "libkwave/CompressionType.h"
#include "libkwave/Plugin.h"

namespace Kwave
{

    class Decoder;
    class Encoder;

    class MP3CodecPlugin: public Kwave::Plugin
    {
	Q_OBJECT
    public:

	/** Constructor */
	MP3CodecPlugin(const Kwave::PluginContext &c);

	/** Destructor */
	virtual ~MP3CodecPlugin();

	/**
	 * Gets called when the plugin is first loaded.
	 */
	virtual void load(QStringList &/* params */);

	/**
	 * Gets called before the plugin is unloaded.
	 */
	virtual void unload();

	/**
	 * Shows a dialog to set up the plugin, configure all paths,
	 * presets and other parameters...
	 * @param previous_params the parameters of a previous call
	 * @return a string list with all parameters or null if the
	 *         setup (dialog) has been canceled
	 */
	virtual QStringList *setup(QStringList &previous_params);

    private:

	/** decoder used as factory */
	Kwave::Decoder *m_decoder;

	/** encoder used as factory */
	Kwave::Encoder *m_encoder;
    };

}

#define REGISTER_MIME_TYPES {                                                \
    addMimeType("audio/x-mp3",    i18n("MPEG layer III audio"), "*.mp3");    \
                                                                             \
    /* like defined in RFC3003 */                                            \
    addMimeType("audio/mpeg",     i18n("MPEG audio"), "*.mpga *.mpg *.mp1"); \
    addMimeType("audio/mpeg",     i18n("MPEG layer II audio"), "*.mp2");     \
    addMimeType("audio/mpeg",     i18n("MPEG layer III audio"), "*.mp3");    \
                                                                             \
    /* included in KDE: */                                                   \
    addMimeType("audio/x-mpga",   i18n("MPEG layer I audio"),                \
                "*.mpga *.mpg *.mp1");                                       \
    addMimeType("audio/x-mp2",    i18n("MPEG layer II audio"), "*.mp2");     \
}

#define REGISTER_COMPRESSION_TYPES {     \
    addCompression(Kwave::CompressionType::MPEG_LAYER_I);   \
    addCompression(Kwave::CompressionType::MPEG_LAYER_II);  \
    addCompression(Kwave::CompressionType::MPEG_LAYER_III); \
}

#endif /* _MP3_CODEC_PLUGIN_H_ */

//***************************************************************************
//***************************************************************************
