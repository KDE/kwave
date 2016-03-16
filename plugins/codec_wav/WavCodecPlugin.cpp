/*************************************************************************
     WavCodecPlugin.cpp  -  import/export of wav data
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

#include "config.h"

#include "libkwave/CodecManager.h"

#include "WavCodecPlugin.h"
#include "WavDecoder.h"
#include "WavEncoder.h"

KWAVE_PLUGIN(codec_wav, WavCodecPlugin)

// static instance of the codec container
Kwave::CodecPlugin::Codec Kwave::WavCodecPlugin::m_codec = {0, 0, 0};

/***************************************************************************/
Kwave::WavCodecPlugin::WavCodecPlugin(QObject *parent,
                                      const QVariantList &args)
    :Kwave::CodecPlugin(parent, args, m_codec)
{
}

/***************************************************************************/
Kwave::WavCodecPlugin::~WavCodecPlugin()
{
}

/***************************************************************************/
Kwave::Decoder *Kwave::WavCodecPlugin::createDecoder()
{
    return new Kwave::WavDecoder();
}

/***************************************************************************/
Kwave::Encoder *Kwave::WavCodecPlugin::createEncoder()
{
    return new Kwave::WavEncoder();
}

//***************************************************************************
#include "WavCodecPlugin.moc"
//***************************************************************************
//***************************************************************************
