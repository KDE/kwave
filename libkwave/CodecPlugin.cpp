/*************************************************************************
        CodecPlugin.cpp  -  base class for codec plugins
                             -------------------
    begin                : Fri Dec 28 2012
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
#include "libkwave/CodecPlugin.h"
#include "libkwave/Decoder.h"
#include "libkwave/Encoder.h"

/***************************************************************************/
Kwave::CodecPlugin::CodecPlugin(PluginManager &plugin_manager, Codec &codec)
    :Kwave::Plugin(plugin_manager), m_codec(codec)
{
}

/***************************************************************************/
Kwave::CodecPlugin::~CodecPlugin()
{
}

/***************************************************************************/
void Kwave::CodecPlugin::load(QStringList &/* params */)
{
    use();

    m_codec.m_use_count++;
    if (m_codec.m_use_count == 1)
    {
	m_codec.m_encoder = createEncoder();
	if (m_codec.m_encoder)
	    Kwave::CodecManager::registerEncoder(*m_codec.m_encoder);

	m_codec.m_decoder = createDecoder();
	if (m_codec.m_decoder)
	    Kwave::CodecManager::registerDecoder(*m_codec.m_decoder);
    }
}

/***************************************************************************/
void Kwave::CodecPlugin::unload()
{
    m_codec.m_use_count--;

    if (m_codec.m_use_count < 1)
    {
	if (m_codec.m_decoder) {
	    Kwave::CodecManager::unregisterDecoder(m_codec.m_decoder);
	    delete m_codec.m_decoder;
	    m_codec.m_decoder = 0;
	}

	if (m_codec.m_encoder) {
	    Kwave::CodecManager::unregisterEncoder(m_codec.m_encoder);
	    delete m_codec.m_encoder;
	    m_codec.m_encoder = 0;
	}
    }

    release();
}

/***************************************************************************/
/***************************************************************************/
