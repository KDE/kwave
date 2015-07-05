/*************************************************************************
    AudiofileCodecPlugin.cpp  -  import/export through libaudiofile
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

#include "config.h"
#include <KLocalizedString>

#include "libkwave/CodecManager.h"

#include "AudiofileCodecPlugin.h"
#include "AudiofileDecoder.h"

KWAVE_PLUGIN(Kwave::AudiofileCodecPlugin, "codec_audiofile", "2.3",
             I18N_NOOP("Audiofile Codec"), "Thomas Eschenbacher");

// static instance of the codec container
Kwave::CodecPlugin::Codec Kwave::AudiofileCodecPlugin::m_codec = {0, 0, 0};

/***************************************************************************/
Kwave::AudiofileCodecPlugin::AudiofileCodecPlugin(
    Kwave::PluginManager &plugin_manager
)
    :Kwave::CodecPlugin(plugin_manager, m_codec)
{
}

/***************************************************************************/
Kwave::AudiofileCodecPlugin::~AudiofileCodecPlugin()
{
}

/***************************************************************************/
Kwave::Decoder *Kwave::AudiofileCodecPlugin::createDecoder()
{
    return new Kwave::AudiofileDecoder();
}

/***************************************************************************/
Kwave::Encoder *Kwave::AudiofileCodecPlugin::createEncoder()
{
    return 0; /* not implemented */
}

/***************************************************************************/
/***************************************************************************/
