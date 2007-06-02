/*************************************************************************
   AsciiCodecPlugin.cpp  -  import/export of ASCII data
                             -------------------
    begin                : Sun Nov 28 2006
    copyright            : (C) 2006 by Thomas Eschenbacher
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
#include "kwave/CodecManager.h"

#include "AsciiCodecPlugin.h"
#include "AsciiEncoder.h"
#include "AsciiDecoder.h"

KWAVE_PLUGIN(AsciiCodecPlugin,"codec_ascii","Thomas Eschenbacher");

/***************************************************************************/
AsciiCodecPlugin::AsciiCodecPlugin(const PluginContext &c)
    :KwavePlugin(c), m_decoder(0), m_encoder(0)
{
    i18n("codec_ascii");
}

/***************************************************************************/
AsciiCodecPlugin::~AsciiCodecPlugin()
{
    m_encoder = 0;
    m_decoder = 0;
}

/***************************************************************************/
void AsciiCodecPlugin::load(QStringList &/* params */)
{
    if (!m_encoder) m_encoder = new AsciiEncoder();
    Q_ASSERT(m_encoder);
    if (m_encoder) CodecManager::registerEncoder(*m_encoder);

    if (!m_decoder) m_decoder = new AsciiDecoder();
    Q_ASSERT(m_decoder);
    if (m_decoder) CodecManager::registerDecoder(*m_decoder);
}

/***************************************************************************/
#include "AsciiCodecPlugin.moc"
/***************************************************************************/
/***************************************************************************/
