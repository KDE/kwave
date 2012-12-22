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

#ifndef _ASCII_CODEC_PLUGIN_H_
#define _ASCII_CODEC_PLUGIN_H_

#include "config.h"
#include "libkwave/Plugin.h"

namespace Kwave
{
    class Decoder;
    class Encoder;

    class AsciiCodecPlugin: public Kwave::Plugin
    {
	Q_OBJECT
    public:

	/** Constructor */
	AsciiCodecPlugin(Kwave::PluginManager &plugin_manager);

	/** Destructor */
	virtual ~AsciiCodecPlugin();

	/** Returns the name of the plugin. */
	virtual QString name() const;

	/**
	 * Gets called when the plugin is first loaded.
	 */
	virtual void load(QStringList &/* params */);

	/**
	 * Gets called before the plugin is unloaded.
	 */
	virtual void unload();

    private:
	/** decoder used as factory */
	Kwave::Decoder *m_decoder;

	/** encoder used as factory */
	Kwave::Encoder *m_encoder;
    };
}

#define LOAD_MIME_TYPES { \
    addMimeType("audio/x-audio-ascii", \
    i18n("ASCII encoded audio"), "*.ascii; *.ASCII"); \
}

/** prefix used for encoding metadata / properties */
#define META_PREFIX _("## ")

#endif /* _ASCII_CODEC_PLUGIN_H_ */

//***************************************************************************
//***************************************************************************
