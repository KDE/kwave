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

#include <KLocalizedString>

#include "OggCodecPlugin.h"
#include "OggDecoder.h"
#include "OggEncoder.h"

KWAVE_PLUGIN(codec_ogg, OggCodecPlugin)

// static instance of the codec container
Kwave::CodecPlugin::Codec Kwave::OggCodecPlugin::m_codec = EMPTY_CODEC;

/***************************************************************************/
Kwave::OggCodecPlugin::OggCodecPlugin(QObject *parent,
                                      const QVariantList &args)
    :Kwave::CodecPlugin(parent, args, m_codec)
{
}

/***************************************************************************/
Kwave::OggCodecPlugin::~OggCodecPlugin()
{
}

/***************************************************************************/
QList<Kwave::Decoder *> Kwave::OggCodecPlugin::createDecoder()
{
    return singleDecoder<Kwave::OggDecoder>();
}

/***************************************************************************/
QList<Kwave::Encoder *> Kwave::OggCodecPlugin::createEncoder()
{
    return singleEncoder<Kwave::OggEncoder>();
}

//***************************************************************************
#include "OggCodecPlugin.moc"
//***************************************************************************
//***************************************************************************

#include "moc_OggCodecPlugin.cpp"
