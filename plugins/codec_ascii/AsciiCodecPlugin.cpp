/*************************************************************************
   AsciiCodecPlugin.cpp  -  import/export of ASCII data
                             -------------------
    begin                : Sun Nov 28 2006
    copyright            : (C) 2006 by Thomas Eschenbacher
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

#include "AsciiCodecPlugin.h"
#include "AsciiDecoder.h"
#include "AsciiEncoder.h"

KWAVE_PLUGIN(Kwave::AsciiCodecPlugin, "codec_ascii", "2.3",
             I18N_NOOP("ASCII Codec"), "Thomas Eschenbacher");

// static instance of the codec container
Kwave::CodecPlugin::Codec Kwave::AsciiCodecPlugin::m_codec = {0, 0, 0};

/***************************************************************************/
Kwave::AsciiCodecPlugin::AsciiCodecPlugin(Kwave::PluginManager &plugin_manager)
    :Kwave::CodecPlugin(plugin_manager, m_codec)
{
}

/***************************************************************************/
Kwave::AsciiCodecPlugin::~AsciiCodecPlugin()
{
}

/***************************************************************************/
Kwave::Decoder *Kwave::AsciiCodecPlugin::createDecoder()
{
    return new Kwave::AsciiDecoder();
}

/***************************************************************************/
Kwave::Encoder *Kwave::AsciiCodecPlugin::createEncoder()
{
    return new Kwave::AsciiEncoder();
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
