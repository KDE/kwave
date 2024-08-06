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

#include "WavCodecPlugin.h"
#include "WavDecoder.h"
#include "WavEncoder.h"

KWAVE_PLUGIN(codec_wav, WavCodecPlugin)

// static instance of the codec container
Kwave::CodecPlugin::Codec Kwave::WavCodecPlugin::m_codec = EMPTY_CODEC;

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
QList<Kwave::Decoder *> Kwave::WavCodecPlugin::createDecoder()
{
    return singleDecoder<Kwave::WavDecoder>();
}

/***************************************************************************/
QList<Kwave::Encoder *> Kwave::WavCodecPlugin::createEncoder()
{
    return singleEncoder<Kwave::WavEncoder>();
}

//***************************************************************************
#include "WavCodecPlugin.moc"
//***************************************************************************
//***************************************************************************

#include "moc_WavCodecPlugin.cpp"
