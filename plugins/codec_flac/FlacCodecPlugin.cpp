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

#include "config.h"

#include <KLocalizedString>

#include "libkwave/PluginManager.h"

#include "FlacCodecPlugin.h"
#include "FlacDecoder.h"
#include "FlacEncoder.h"

// static instance of the codec container
Kwave::CodecPlugin::Codec Kwave::FlacCodecPlugin::m_codec = EMPTY_CODEC;

KWAVE_PLUGIN(codec_flac, FlacCodecPlugin)

/***************************************************************************/
Kwave::FlacCodecPlugin::FlacCodecPlugin(QObject *parent,
                                        const QVariantList &args)
    :Kwave::CodecPlugin(parent, args, m_codec)
{
}

/***************************************************************************/
Kwave::FlacCodecPlugin::~FlacCodecPlugin()
{
}

/***************************************************************************/
QList<Kwave::Decoder *> Kwave::FlacCodecPlugin::createDecoder()
{
    return singleDecoder<Kwave::FlacDecoder>();
}

/***************************************************************************/
QList<Kwave::Encoder *> Kwave::FlacCodecPlugin::createEncoder()
{
    return singleEncoder<Kwave::FlacEncoder>();
}

/***************************************************************************/
#include "FlacCodecPlugin.moc"
/***************************************************************************/
/***************************************************************************/

#include "moc_FlacCodecPlugin.cpp"
