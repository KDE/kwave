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

#ifndef CODEC_PLUGIN_H
#define CODEC_PLUGIN_H

#include "config.h"

#include <QtGlobal>
#include <QList>

#include "libkwave/Plugin.h"

namespace Kwave
{
    class Decoder;
    class Encoder;

    class Q_DECL_EXPORT CodecPlugin: public Kwave::Plugin
    {
    public:

	/** container for codecs */
	typedef struct {
	    int                     m_use_count; /**< use count        */
	    QList<Kwave::Encoder *> m_encoder;   /**< list of encoders */
	    QList<Kwave::Decoder *> m_decoder;   /**< list of decoders */
	} Codec;

	/**
	 * Constructor
	 * @param parent pointer to the corresponding plugin manager
	 * @param args argument list, containts internal meta data
	 * @param codec reference to a static container for the codec
	 */
	CodecPlugin(QObject *parent, const QVariantList &args, Codec &codec);

	/** Destructor */
	virtual ~CodecPlugin();

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

	/**
         * Create a new set of decoders
         * @return list of decoders, may be  empty
         */
	virtual QList<Kwave::Decoder *> createDecoder() = 0;

	/**
         * Create a new set of encoders
         * @return list of encoders, may be  empty
         */
	virtual QList<Kwave::Encoder *> createEncoder() = 0;

    protected:

        /**
         * helper template to return a list with a single decoder,
         * for use within createDecoder()
         */
        template<class T> QList<Kwave::Decoder *> singleDecoder()
        {
            QList<Kwave::Decoder *> list;
            list.append(new(std::nothrow) T);
            return list;
        }

        /**
         * helper template to return a list with a single encoder,
         * for use within createEncoder()
         */
        template<class T> QList<Kwave::Encoder *> singleEncoder()
        {
            QList<Kwave::Encoder *> list;
            list.append(new(std::nothrow) T);
            return list;
        }

    private:

	/** reference to the static container with encoder/decoder/usecount */
	Codec &m_codec;
    };
}

/** initializer for an empty Kwave::CodecPlugin::Codec */
#define EMPTY_CODEC {0, QList<Kwave::Encoder *>(), QList<Kwave::Decoder *>() }

#endif /* CODEC_PLUGIN_H */

//***************************************************************************
//***************************************************************************
