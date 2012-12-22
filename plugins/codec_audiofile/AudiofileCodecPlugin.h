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

#ifndef _AUDIOFILE_CODEC_PLUGIN_H_
#define _AUDIOFILE_CODEC_PLUGIN_H_

#include "config.h"
#include "libkwave/Plugin.h"

namespace Kwave
{
    class Decoder;

    class AudiofileCodecPlugin: public Kwave::Plugin
    {
	Q_OBJECT
    public:

	/** Constructor */
	AudiofileCodecPlugin(Kwave::PluginManager &plugin_manager);

	/** Destructor */
	virtual ~AudiofileCodecPlugin();

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

    };
}

#endif /* _AUDIOFILE_CODEC_PLUGIN_H_ */

//***************************************************************************
//***************************************************************************
