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
#include "WavEncoder.h"
#include "WavDecoder.h"

KWAVE_PLUGIN(Kwave::WavCodecPlugin, "codec_wav", "2.3",
             I18N_NOOP("WAV Codec"), "Thomas Eschenbacher");

/***************************************************************************/
Kwave::WavCodecPlugin::WavCodecPlugin(Kwave::PluginManager &plugin_manager)
    :Kwave::Plugin(plugin_manager), m_decoder(0), m_encoder(0)
{
}

/***************************************************************************/
Kwave::WavCodecPlugin::~WavCodecPlugin()
{
    m_encoder = 0;
    m_decoder = 0;
}

/***************************************************************************/
void Kwave::WavCodecPlugin::load(QStringList &/* params */)
{
    use();

    if (!m_encoder) m_encoder = new Kwave::WavEncoder();
    Q_ASSERT(m_encoder);
    if (m_encoder) Kwave::CodecManager::registerEncoder(*m_encoder);

    if (!m_decoder) m_decoder = new Kwave::WavDecoder();
    Q_ASSERT(m_decoder);
    if (m_decoder) Kwave::CodecManager::registerDecoder(*m_decoder);
}

/***************************************************************************/
void Kwave::WavCodecPlugin::unload()
{
    Kwave::CodecManager::unregisterDecoder(m_decoder);
    delete m_decoder;
    m_decoder = 0;

    Kwave::CodecManager::unregisterEncoder(m_encoder);
    delete m_encoder;
    m_encoder = 0;

    release();
}

//***************************************************************************
#include "WavCodecPlugin.moc"
//***************************************************************************
//***************************************************************************
