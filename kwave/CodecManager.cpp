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
#include <qptrlist.h>
#include <qmime.h>
#include <qregexp.h>

#include "libkwave/Decoder.h"
#include "libkwave/Encoder.h"
#include "CodecManager.h"

/***************************************************************************/
/* static initializers */
QPtrList<Encoder> CodecManager::m_encoders;
QPtrList<Decoder> CodecManager::m_decoders;

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
    QPtrListIterator<Decoder> it(m_decoders);
    for (; it.current(); ++it) {
	Decoder *d = it.current();
	Q_ASSERT(d);
	if (d && d->supports(mimetype)) return true;
    }
    return false;
}

/***************************************************************************/
bool CodecManager::canDecode(const QString &mimetype_name)
{
    QPtrListIterator<Decoder> it(m_decoders);
    for (; it.current(); ++it) {
	Decoder *d = it.current();
	Q_ASSERT(d);
	if (d && d->supports(mimetype_name)) return true;
    }
    return false;
}

/***************************************************************************/
QString CodecManager::whatContains(const KURL &url)
{
    QPtrListIterator<Decoder> it(m_decoders);
    for (; it.current(); ++it) {
	Decoder &d = *(it.current());
	QString mime_type = d.whatContains(url);
	if (mime_type != KMimeType::defaultMimeType()) return mime_type;
    }
    return KMimeType::findByURL(url)->name();
}

/***************************************************************************/
Decoder *CodecManager::decoder(const QString &mimetype_name)
{
    QPtrListIterator<Decoder> it(m_decoders);
    for (; it.current(); ++it) {
	Decoder *d = it.current();
	Q_ASSERT(d);
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
	Q_ASSERT(d);
	if (d) return d;
    }
    return 0;
}

/***************************************************************************/
Encoder *CodecManager::encoder(const QString &mimetype_name)
{
    QPtrListIterator<Encoder> it(m_encoders);
    for (; it.current(); ++it) {
	Encoder *e = it.current();
	Q_ASSERT(e);
	if (e && e->supports(mimetype_name)) return e->instance();
    }
    return 0;
}

/***************************************************************************/
QString CodecManager::encodingFilter()
{
    QPtrListIterator<Encoder> it(m_encoders);
    QStringList list;
    for (; it.current(); ++it) {
	Encoder *e = it.current();

	// loop over all mime types that the encoder supports
	QPtrList<KMimeType> types = e->mimeTypes();
	QPtrListIterator<KMimeType> ti(types);
	for (; ti.current(); ++ti) {
	    KMimeType *type = ti.current();
	    QString extensions = type->patterns().join(" ");

	    // skip if extensions are already known/present
	    if (list.join("\n").contains(extensions)) continue;

	    // otherwise append to the list
	    QString entry = extensions;
	    QString comment = type->comment().replace(QRegExp("/"), ",");
	    entry += "|" + comment;
	    list.append(entry + " (" + extensions + ")");
	}
    }
    list.sort();
    QString str_list = list.join("\n");
    Q_ASSERT(!str_list.contains('/'));
    if (str_list.contains('/')) {
	qWarning("CodecManager::encodingFilter() -> '%s'",
	         str_list.data());
    }

    return str_list;
}
/***************************************************************************/
QString CodecManager::decodingFilter()
{
    QPtrListIterator<Decoder> it(m_decoders);
    QStringList list;
    for (; it.current(); ++it) {
	Decoder *d = it.current();

	// loop over all mime types that the decoder supports
	QPtrList<KMimeType> types = d->mimeTypes();
	QPtrListIterator<KMimeType> ti(types);
	for (; ti.current(); ++ti) {
	    KMimeType *type = ti.current();
	    QString extensions = type->patterns().join(" ");

	    // skip if extensions are already known/present
	    if (list.join("\n").contains(extensions)) continue;

	    // otherwise append to the list
	    QString entry = extensions;
	    QString comment = type->comment().replace(QRegExp("/"), ",");
	    entry += "|" + comment;
	    list.append(entry + " (" + extensions + ")");
	}
    }
    list.sort();
    QString str_list = list.join("\n");
    Q_ASSERT(!str_list.contains('/'));
    if (str_list.contains('/')) {
	qWarning("CodecManager::decodingFilter() -> '%s'",
	         str_list.data());
    }

    return str_list;
}

/***************************************************************************/
/***************************************************************************/
