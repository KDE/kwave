/***************************************************************************
       CodecManager.cpp  -  manager for Kwave's coders and decoders
			     -------------------
    begin                : Mar 10 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
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
#include <qlist.h>
#include <kmimetype.h>

#include "libkwave/Decoder.h"
#include "libkwave/Encoder.h"
#include "CodecManager.h"

/***************************************************************************/
/* static initializers */
QList<Encoder> CodecManager::m_encoders;
QList<Decoder> CodecManager::m_decoders;

/***************************************************************************/
/***************************************************************************/
CodecManager::CodecManager()
{
}

/***************************************************************************/
CodecManager::~CodecManager()
{
    m_encoders.setAutoDelete(true);
    m_decoders.setAutoDelete(true);
    m_encoders.clear();
    m_decoders.clear();
}

/***************************************************************************/
void CodecManager::registerEncoder(const Encoder &encoder)
{
    if (m_encoders.contains(&encoder)) return; /* already known */
    m_encoders.append(&encoder);
}

/***************************************************************************/
void CodecManager::registerDecoder(const Decoder &decoder)
{
    if (m_decoders.contains(&decoder)) return; /* already known */
    m_decoders.append(&decoder);
}

/***************************************************************************/
Decoder *CodecManager::decoder(const KMimeType &mimetype)
{
    QListIterator<Decoder> it(m_decoders);
    for (; it.current(); ++it) {
	Decoder *d = it.current();
	ASSERT(d);
	if (d && d->supports(mimetype)) return d;
    }
    return 0;
}

/***************************************************************************/
Encoder *CodecManager::encoder(const KMimeType &mimetype)
{
    QListIterator<Encoder> it(m_encoders);
    for (; it.current(); ++it) {
	Encoder *e = it.current();
	ASSERT(e);
	if (e && e->supports(mimetype)) return e;
    }
    return 0;
}

/***************************************************************************/
/***************************************************************************/
