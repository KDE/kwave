/*************************************************************************
     OggCodecPlugin.cpp  -  import/export of Ogg/Vorbis data
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

#include <klocale.h>
#include "libkwave/CodecManager.h"

#include "OggCodecPlugin.h"
#include "OggEncoder.h"
#include "OggDecoder.h"

KWAVE_PLUGIN(Kwave::OggCodecPlugin, "codec_ogg", "2.3",
             I18N_NOOP("Ogg Codec"), "Thomas Eschenbacher");

/***************************************************************************/
Kwave::OggCodecPlugin::OggCodecPlugin(const Kwave::PluginContext &c)
    :Kwave::Plugin(c), m_decoder(0), m_encoder(0)
{
}

/***************************************************************************/
Kwave::OggCodecPlugin::~OggCodecPlugin()
{
}

/***************************************************************************/
void Kwave::OggCodecPlugin::load(QStringList &/* params */)
{
    use();

    if (!m_encoder) m_encoder = new Kwave::OggEncoder();
    Q_ASSERT(m_encoder);
    if (m_encoder) Kwave::CodecManager::registerEncoder(*m_encoder);

    if (!m_decoder) m_decoder = new Kwave::OggDecoder();
    Q_ASSERT(m_decoder);
    if (m_decoder) Kwave::CodecManager::registerDecoder(*m_decoder);
}

/***************************************************************************/
void Kwave::OggCodecPlugin::unload()
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
#include "OggCodecPlugin.moc"
//***************************************************************************
//***************************************************************************
