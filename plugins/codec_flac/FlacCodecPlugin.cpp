/*************************************************************************
    FlacCodecPlugin.cpp  -  import/export of FLAC data
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

#include <config.h>

#include <klocale.h>

#include "libkwave/CodecManager.h"

#include "FlacCodecPlugin.h"
#include "FlacEncoder.h"
#include "FlacDecoder.h"

KWAVE_PLUGIN(Kwave::FlacCodecPlugin, "codec_flac", "2.3",
             I18N_NOOP("FLAC Codec"), "Thomas Eschenbacher");

// static instance of the codec container
Kwave::CodecPlugin::Codec Kwave::FlacCodecPlugin::m_codec = {0, 0, 0};

/***************************************************************************/
Kwave::FlacCodecPlugin::FlacCodecPlugin(Kwave::PluginManager &plugin_manager)
    :Kwave::CodecPlugin(plugin_manager, m_codec)
{
}

/***************************************************************************/
Kwave::FlacCodecPlugin::~FlacCodecPlugin()
{
}

/***************************************************************************/
Kwave::Decoder *Kwave::FlacCodecPlugin::createDecoder()
{
    return new Kwave::FlacDecoder();
}

/***************************************************************************/
Kwave::Encoder *Kwave::FlacCodecPlugin::createEncoder()
{
    return new Kwave::FlacEncoder();
}

/***************************************************************************/
#include "FlacCodecPlugin.moc"
/***************************************************************************/
/***************************************************************************/
