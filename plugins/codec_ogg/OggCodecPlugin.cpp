/*************************************************************************
     OggCodecPlugin.cpp  -  import/export of audio in an Ogg container
                             -------------------
    begin                : Tue Sep 10 2002
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

#include <klocale.h>
#include "libkwave/CodecManager.h"

#include "OggCodecPlugin.h"
#include "OggEncoder.h"
#include "OggDecoder.h"

KWAVE_PLUGIN(Kwave::OggCodecPlugin, "codec_ogg", "2.3",
             I18N_NOOP("Ogg Codec"), "Thomas Eschenbacher");

// static instance of the codec container
Kwave::CodecPlugin::Codec Kwave::OggCodecPlugin::m_codec = {0, 0, 0};

/***************************************************************************/
Kwave::OggCodecPlugin::OggCodecPlugin(Kwave::PluginManager &plugin_manager)
    :Kwave::CodecPlugin(plugin_manager, m_codec)
{
}

/***************************************************************************/
Kwave::OggCodecPlugin::~OggCodecPlugin()
{
}

/***************************************************************************/
Kwave::Decoder *Kwave::OggCodecPlugin::createDecoder()
{
    return new Kwave::OggDecoder();
}

/***************************************************************************/
Kwave::Encoder *Kwave::OggCodecPlugin::createEncoder()
{
    return new Kwave::OggEncoder();
}

//***************************************************************************
#include "OggCodecPlugin.moc"
//***************************************************************************
//***************************************************************************
