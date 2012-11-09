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
#include "libkwave/Plugin.h"

class QStringList;
class Encoder;
class Decoder;

class FlacCodecPlugin: public Kwave::Plugin
{
    Q_OBJECT
public:

    /** Constructor */
    FlacCodecPlugin(const PluginContext &c);

    /** Destructor */
    virtual ~FlacCodecPlugin();

    /**
     * This plugin needs to be unique!
     * @see Kwave::Plugin::isUnique()
     */
    virtual bool isUnique() { return true; };

    /**
     * Gets called when the plugin is first loaded.
     */
    virtual void load(QStringList &/* params */);

private:
    /** decoder used as factory */
    Decoder *m_decoder;

    /** encoder used as factory */
    Encoder *m_encoder;
};

#define REGISTER_MIME_TYPES { \
    /* included in KDE: */ \
    addMimeType("audio/x-flac", i18n("FLAC audio"), "*.flac; *.FLAC"); \
}

#define REGISTER_COMPRESSION_TYPES { \
    addCompression(CompressionType::FLAC);   \
}

#define DEFAULT_MIME_TYPE "audio/x-flac"

#endif /* _FLAC_CODEC_PLUGIN_H_ */
