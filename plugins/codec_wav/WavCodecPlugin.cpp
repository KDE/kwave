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

KWAVE_PLUGIN(WavCodecPlugin,"codec_wav","Thomas Eschenbacher");

/***************************************************************************/
WavCodecPlugin::WavCodecPlugin(const PluginContext &c)
    :KwavePlugin(c), m_decoder(0), m_encoder(0)
{
    i18n("codec_wav");
}

/***************************************************************************/
WavCodecPlugin::~WavCodecPlugin()
{
    m_encoder = 0;
    m_decoder = 0;
}

/***************************************************************************/
void WavCodecPlugin::load(QStringList &/* params */)
{
    if (!m_encoder) m_encoder = new WavEncoder();
    Q_ASSERT(m_encoder);
    if (m_encoder) CodecManager::registerEncoder(*m_encoder);

    if (!m_decoder) m_decoder = new WavDecoder();
    Q_ASSERT(m_decoder);
    if (m_decoder) CodecManager::registerDecoder(*m_decoder);
}

//***************************************************************************
#include "WavCodecPlugin.moc"
//***************************************************************************
//***************************************************************************
