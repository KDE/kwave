/*************************************************************************
          CodecPlugin.h  -  base class for codec plugins
                             -------------------
    begin                : Fri Dec 28 2012
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

#ifndef _CODEC_PLUGIN_H_
#define _CODEC_PLUGIN_H_

#include <config.h>

#include <kdemacros.h>

#include "libkwave/Plugin.h"

namespace Kwave
{
    class Decoder;
    class Encoder;

    class KDE_EXPORT CodecPlugin: public Kwave::Plugin
    {
    public:

	/** container for codecs */
	typedef struct {
	    int             m_use_count; /**< use count */
	    Kwave::Encoder *m_encoder;   /**< pointer to a Kwave::Encoder */
	    Kwave::Decoder *m_decoder;   /**< pointer to a Kwave::Decoder */
	} Codec;

	/**
	 * Constructor
	 *
	 * @param plugin_manager reference to the corresponding plugin manager
	 * @param codec reference to a static container for the codec
	 */
	CodecPlugin(PluginManager &plugin_manager, Codec &codec);

	/** Destructor */
	virtual ~CodecPlugin();

	/** Returns the name of the plugin. */
	virtual QString name() const = 0;

	/**
	 * Gets called when the plugin is first loaded. Registers new encoder
	 * and decoder on first call, all subsequenct calls only increment
	 * the reference count of the existing encoder/decoder instances.
	 */
	virtual void load(QStringList &/* params */);

	/**
	 * Gets called before the plugin is unloaded. Decrements the use count
	 * of existing encoder/decoder instances and removes them if zero
	 * gets reached.
	 */
	virtual void unload();

	/** Creates a new decoder instance */
	virtual Kwave::Decoder *createDecoder() = 0;

	/** Creates a new encoder instance */
	virtual Kwave::Encoder *createEncoder() = 0;

    private:

	/** reference to the static container with encoder/decoder/usecount */
	Codec &m_codec;
    };
}

#endif /* _CODEC_PLUGIN_H_ */

//***************************************************************************
//***************************************************************************
