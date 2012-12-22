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
#include <klocale.h>

#include "libkwave/CodecManager.h"

#include "AudiofileCodecPlugin.h"
#include "AudiofileDecoder.h"

KWAVE_PLUGIN(Kwave::AudiofileCodecPlugin, "codec_audiofile", "2.3",
             I18N_NOOP("Audiofile Codec"), "Thomas Eschenbacher");

/***************************************************************************/
Kwave::AudiofileCodecPlugin::AudiofileCodecPlugin(
    Kwave::PluginManager &plugin_manager)
    :Kwave::Plugin(plugin_manager), m_decoder(0)
{
}

/***************************************************************************/
Kwave::AudiofileCodecPlugin::~AudiofileCodecPlugin()
{
}

/***************************************************************************/
void Kwave::AudiofileCodecPlugin::load(QStringList &/* params */)
{
    use();

    if (!m_decoder) m_decoder = new Kwave::AudiofileDecoder();
    Q_ASSERT(m_decoder);
    if (m_decoder) Kwave::CodecManager::registerDecoder(*m_decoder);
}

/***************************************************************************/
void Kwave::AudiofileCodecPlugin::unload()
{
    Kwave::CodecManager::unregisterDecoder(m_decoder);
    delete m_decoder;
    m_decoder = 0;

    release();
}

/***************************************************************************/
#include "AudiofileCodecPlugin.moc"
/***************************************************************************/
/***************************************************************************/
