/*************************************************************************
   MP3DecoderPlugin.cpp  -  import of MP3 data
                             -------------------
    begin                : Wed Aug 07 2002
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

#include "kwave/CodecManager.h"
#include "MP3DecoderPlugin.h"
#include "MP3Decoder.h"

KWAVE_PLUGIN(MP3DecoderPlugin,"decoder_mp3","Thomas Eschenbacher");

/***************************************************************************/
MP3DecoderPlugin::MP3DecoderPlugin(const PluginContext &c)
    :KwavePlugin(c), m_decoder(0)
{
    i18n("decoder_mp3");
}

/***************************************************************************/
MP3DecoderPlugin::~MP3DecoderPlugin()
{
}

/***************************************************************************/
void MP3DecoderPlugin::load(QStringList &/* params */)
{
    if (!m_decoder) m_decoder = new MP3Decoder();
    Q_ASSERT(m_decoder);
    if (m_decoder) CodecManager::registerDecoder(*m_decoder);
}

/***************************************************************************/
/***************************************************************************/
