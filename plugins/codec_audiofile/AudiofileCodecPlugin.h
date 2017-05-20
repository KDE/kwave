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
	virtual ~AudiofileCodecPlugin();

	/** Creates a new decoder */
	virtual QList<Kwave::Decoder *> createDecoder();

	/** Creates a new encoder */
	virtual QList<Kwave::Encoder *> createEncoder();

    private:
	/** static codec container */
	static CodecPlugin::Codec m_codec;
    };
}

#endif /* AUDIOFILE_CODEC_PLUGIN_H */

//***************************************************************************
//***************************************************************************
