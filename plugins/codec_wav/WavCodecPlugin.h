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

#ifndef _WAV_CODEC_PLUGIN_H_
#define _WAV_CODEC_PLUGIN_H_

#include "config.h"
#include "libkwave/KwavePlugin.h"

class Encoder;
class Decoder;

class WavCodecPlugin: public KwavePlugin
{
    Q_OBJECT
public:

    /** Constructor */
    WavCodecPlugin(const PluginContext &c);

    /** Destructor */
    virtual ~WavCodecPlugin();

    /**
     * This plugin needs to be unique!
     * @see KwavePlugin::isUnique()
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

#endif /* _WAV_CODEC_PLUGIN_H_ */
