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
#include "libkwave/PluginManager.h"

#include "AudiofileCodecPlugin.h"
#include "AudiofileDecoder.h"

// static instance of the codec container
Kwave::CodecPlugin::Codec Kwave::AudiofileCodecPlugin::m_codec = {0, 0, 0};

KWAVE_PLUGIN(codec_audiofile, AudiofileCodecPlugin)

/***************************************************************************/
Kwave::AudiofileCodecPlugin::AudiofileCodecPlugin(QObject *parent,
                                                  const QVariantList &args)
    :Kwave::CodecPlugin(parent, args, m_codec)
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
#include "AudiofileCodecPlugin.moc"
/***************************************************************************/
/***************************************************************************/
