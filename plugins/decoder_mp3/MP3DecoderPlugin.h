/*************************************************************************
     MP3DecoderPlugin.h  -  import of MP3 data
                             -------------------
    begin                : Wed Aug 07 2002
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

#ifndef _MP3_DECODER_PLUGIN_H_
#define _MP3_DECODER_PLUGIN_H_

#include "config.h"
#include "libkwave/KwavePlugin.h"

class Decoder;

class MP3DecoderPlugin: public KwavePlugin
{
    Q_OBJECT
public:

    /** Constructor */
    MP3DecoderPlugin(PluginContext &c);

    /** Destructor */
    virtual ~MP3DecoderPlugin();

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

};

#endif /* _MP3_DECODER_PLUGIN_H_ */

