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
#include <qmime.h>

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
bool CodecManager::canDecode(const KMimeType &mimetype)
{
    QListIterator<Decoder> it(m_decoders);
    for (; it.current(); ++it) {
	Decoder *d = it.current();
	ASSERT(d);
	if (d && d->supports(mimetype)) return true;
    }
    return false;
}

/***************************************************************************/
bool CodecManager::canDecode(const QString &mimetype_name)
{
    QListIterator<Decoder> it(m_decoders);
    for (; it.current(); ++it) {
	Decoder *d = it.current();
	ASSERT(d);
	if (d && d->supports(mimetype_name)) return true;
    }
    return false;
}

/***************************************************************************/
Decoder *CodecManager::decoder(const QString &mimetype_name)
{
    QListIterator<Decoder> it(m_decoders);
    for (; it.current(); ++it) {
	Decoder *d = it.current();
	ASSERT(d);
	if (d && d->supports(mimetype_name)) return d->instance();
    }
    return 0;
}

/***************************************************************************/
Decoder *CodecManager::decoder(const KMimeType &mimetype)
{
    return decoder(mimetype.name());
}

/***************************************************************************/
Decoder *CodecManager::decoder(const QMimeSource *mime_source)
{
    if (!mime_source) return false;
    int i = 0;
    const char *format;
    for (i=0; (format = mime_source->format(i)); ++i) {
	Decoder *d = decoder(format);
	ASSERT(d);
	if (d) return d;
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
	if (e && e->supports(mimetype)) return e->instance();
    }
    return 0;
}

/***************************************************************************/
/***************************************************************************/
