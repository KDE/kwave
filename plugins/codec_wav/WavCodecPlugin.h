/*************************************************************************
       WavCodecPlugin.h  -  import/export of wav data
                             -------------------
    begin                : Sun Mar 10 2002
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

#ifndef WAV_CODEC_PLUGIN_H
#define WAV_CODEC_PLUGIN_H

#include "config.h"

#include "libkwave/CodecPlugin.h"

namespace Kwave
{

    class WavCodecPlugin: public Kwave::CodecPlugin
    {
	Q_OBJECT
    public:

	/**
	 * Constructor
	 * @param parent reference to our plugin manager
	 * @param args argument list [unused]
	 */
	WavCodecPlugin(QObject *parent, const QVariantList &args);

	/** Destructor */
	virtual ~WavCodecPlugin();

	/** Creates a new decoder */
	QList<Kwave::Decoder *> createDecoder() Q_DECL_OVERRIDE;

	/** Creates a new encoder */
	QList<Kwave::Encoder *> createEncoder() Q_DECL_OVERRIDE;

    private:
	/** static codec container */
	static CodecPlugin::Codec m_codec;
    };
}

#endif /* WAV_CODEC_PLUGIN_H */

//***************************************************************************
//***************************************************************************
