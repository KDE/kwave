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

#include <audiofile.h>

#include <KLocalizedString>

#include "libkwave/PluginManager.h"

#include "AudiofileCodecPlugin.h"
#include "AudiofileDecoder.h"
#include "AudiofileEncoder.h"

// static instance of the codec container
Kwave::CodecPlugin::Codec Kwave::AudiofileCodecPlugin::m_codec = EMPTY_CODEC;

KWAVE_PLUGIN(codec_audiofile, AudiofileCodecPlugin)

/***************************************************************************/
static bool audiofileSupports(int format_id)
{
    return (afQueryLong(AF_QUERYTYPE_FILEFMT, AF_QUERY_IMPLEMENTED,
                        format_id, 0, 0) == 1);
}

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
QList<Kwave::Decoder::Instance> Kwave::AudiofileCodecPlugin::createDecoder()
{
    return singleDecoder<Kwave::AudiofileDecoder>();
}

/***************************************************************************/
QList<Kwave::Encoder::Instance> Kwave::AudiofileCodecPlugin::createEncoder()
{
    QList<Kwave::Encoder::Instance> list;

    #define addAfMimeType(m, d, e, t)                                  \
        if (audiofileSupports(t))                                      \
            list.append(                                               \
                std::make_shared<Kwave::AudiofileEncoder>(m, d, e, t))
    REGISTER_MIME_TYPES
    #undef addAfMimeType

    return list;
}

/***************************************************************************/
#include "AudiofileCodecPlugin.moc"
/***************************************************************************/
/***************************************************************************/

#include "moc_AudiofileCodecPlugin.cpp"
