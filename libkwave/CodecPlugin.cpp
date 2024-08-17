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
Kwave::CodecPlugin::CodecPlugin(QObject *parent,
                                const QVariantList &args,
                                Codec &codec)
    :Kwave::Plugin(parent, args), m_codec(codec)
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
        if (!m_codec.m_encoder.isEmpty()) {
            foreach (Kwave::Encoder *enc, m_codec.m_encoder)
                if (enc) Kwave::CodecManager::registerEncoder(*enc);
        }

        m_codec.m_decoder = createDecoder();
        if (!m_codec.m_decoder.isEmpty()) {
            foreach (Kwave::Decoder *dec, m_codec.m_decoder)
                Kwave::CodecManager::registerDecoder(*dec);
        }
    }
}

/***************************************************************************/
void Kwave::CodecPlugin::unload()
{
    m_codec.m_use_count--;

    if (m_codec.m_use_count < 1)
    {
        while (!m_codec.m_decoder.isEmpty()) {
            Kwave::Decoder *dec = m_codec.m_decoder.takeLast();
            Kwave::CodecManager::unregisterDecoder(dec);
            delete dec;
        }

        while (!m_codec.m_encoder.isEmpty()) {
            Kwave::Encoder *enc = m_codec.m_encoder.takeLast();
            Kwave::CodecManager::unregisterEncoder(enc);
            delete enc;
        }
    }

    release();
}

/***************************************************************************/
/***************************************************************************/
