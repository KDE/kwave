/*************************************************************************
     MP3CodecPlugin.cpp  -  import and export of MP3 data
                             -------------------
    begin                : Mon May 28 2012
    copyright            : (C) 2012 by Thomas Eschenbacher
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

#include "MP3CodecPlugin.h"
#include "MP3Decoder.h"
#include "MP3Encoder.h"

KWAVE_PLUGIN(Kwave::MP3CodecPlugin, "codec_mp3", "2.1",
             I18N_NOOP("MP3 Codec"), "Thomas Eschenbacher");

/***************************************************************************/
Kwave::MP3CodecPlugin::MP3CodecPlugin(const PluginContext &c)
    :Kwave::Plugin(c), m_decoder(0), m_encoder(0)
{
}

/***************************************************************************/
Kwave::MP3CodecPlugin::~MP3CodecPlugin()
{
    m_encoder = 0;
    m_decoder = 0;
}

/***************************************************************************/
void Kwave::MP3CodecPlugin::load(QStringList &/* params */)
{
    if (!m_decoder) m_decoder = new Kwave::MP3Decoder();
    Q_ASSERT(m_decoder);
    if (m_decoder) CodecManager::registerDecoder(*m_decoder);

    if (!m_encoder) m_encoder = new Kwave::MP3Encoder();
    Q_ASSERT(m_encoder);
    if (m_encoder) CodecManager::registerEncoder(*m_encoder);

}

//***************************************************************************
#include "MP3CodecPlugin.moc"
//***************************************************************************
//***************************************************************************
