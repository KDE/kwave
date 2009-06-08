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

#include <klocale.h>

#include "libkwave/CodecManager.h"

#include "FlacCodecPlugin.h"
#include "FlacEncoder.h"
#include "FlacDecoder.h"

KWAVE_PLUGIN(FlacCodecPlugin,"codec_flac","2.1","Thomas Eschenbacher");

/***************************************************************************/
FlacCodecPlugin::FlacCodecPlugin(const PluginContext &c)
    :Kwave::Plugin(c), m_decoder(0), m_encoder(0)
{
    i18n("codec_flac");
}

/***************************************************************************/
FlacCodecPlugin::~FlacCodecPlugin()
{
    m_encoder = 0;
    m_decoder = 0;
}

/***************************************************************************/
void FlacCodecPlugin::load(QStringList &/* params */)
{
    if (!m_encoder) m_encoder = new FlacEncoder();
    Q_ASSERT(m_encoder);
    if (m_encoder) CodecManager::registerEncoder(*m_encoder);

    if (!m_decoder) m_decoder = new FlacDecoder();
    Q_ASSERT(m_decoder);
    if (m_decoder) CodecManager::registerDecoder(*m_decoder);
}

/***************************************************************************/
#include "FlacCodecPlugin.moc"
/***************************************************************************/
/***************************************************************************/
