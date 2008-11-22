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

#include <QMimeData>
#include <QRegExp>

#include "libkwave/CodecManager.h"
#include "libkwave/Decoder.h"
#include "libkwave/Encoder.h"

//***************************************************************************
/* static initializers */
QList<Encoder *> CodecManager::m_encoders;
QList<Decoder *> CodecManager::m_decoders;

//***************************************************************************
//***************************************************************************
CodecManager::CodecManager()
{
}

//***************************************************************************
CodecManager::~CodecManager()
{
    Q_ASSERT(m_encoders.isEmpty());
    Q_ASSERT(m_decoders.isEmpty());
}

//***************************************************************************
void CodecManager::registerEncoder(Encoder &encoder)
{
    if (m_encoders.contains(&encoder)) return; /* already known */
    m_encoders.append(&encoder);
}

//***************************************************************************
void CodecManager::registerDecoder(Decoder &decoder)
{
    if (m_decoders.contains(&decoder)) return; /* already known */
    m_decoders.append(&decoder);
}

//***************************************************************************
bool CodecManager::canDecode(const KMimeType &mimetype)
{
    foreach (Decoder *d, m_decoders)
	if (d && d->supports(mimetype)) return true;
    return false;
}

//***************************************************************************
bool CodecManager::canDecode(const QString &mimetype_name)
{
    foreach (Decoder *d, m_decoders)
	if (d && d->supports(mimetype_name)) return true;
    return false;
}

//***************************************************************************
QString CodecManager::whatContains(const KUrl &url)
{
    foreach (Decoder *d, m_decoders) {
	if (!d) continue;
	QString mime_type = d->whatContains(url);
	if (mime_type != KMimeType::defaultMimeType()) return mime_type;
    }
    foreach (Encoder *e, m_encoders) {
	if (!e) continue;
	QString mime_type = e->whatContains(url);
	if (mime_type != KMimeType::defaultMimeType()) return mime_type;
    }
    return KMimeType::findByUrl(url)->name();
}

//***************************************************************************
Decoder *CodecManager::decoder(const QString &mimetype_name)
{
    foreach (Decoder *d, m_decoders)
	if (d && d->supports(mimetype_name)) return d->instance();
    return 0;
}

//***************************************************************************
Decoder *CodecManager::decoder(const KMimeType &mimetype)
{
    return decoder(mimetype.name());
}

//***************************************************************************
Decoder *CodecManager::decoder(const QMimeData *mime_data)
{
    if (!mime_data) return 0;

    foreach (QString format, mime_data->formats()) {
	Decoder *d = decoder(format);
	if (d) return d;
    }
    return 0;
}

//***************************************************************************
Encoder *CodecManager::encoder(const QString &mimetype_name)
{
    foreach (Encoder *e, m_encoders)
	if (e && e->supports(mimetype_name)) return e->instance();
    return 0;
}

//***************************************************************************
QString CodecManager::encodingFilter()
{
    QStringList list;
    foreach (Encoder *e, m_encoders) {
	// loop over all mime types that the encoder supports
	QList<CodecBase::MimeType> types = e->mimeTypes();
	QListIterator<CodecBase::MimeType> ti(types);
	while (ti.hasNext()) {
	    CodecBase::MimeType type = ti.next();
	    QString extensions = type.patterns.join(" ");

	    // skip if extensions are already known/present
	    if (list.join("\n").contains(extensions)) continue;

	    // otherwise append to the list
	    QString entry = extensions;
	    QString comment = type.description.replace(QRegExp("/"), ",");
	    entry += "|" + comment;
	    list.append(entry + " (" + extensions + ")");
	}
    }
    list.sort();
    QString str_list = list.join("\n");
    Q_ASSERT(!str_list.contains('/'));
    if (str_list.contains('/')) {
	qWarning("CodecManager::encodingFilter() -> '%s'",
	         str_list.toLocal8Bit().data());
    }

    return str_list;
}

//***************************************************************************
QString CodecManager::decodingFilter()
{
    QStringList list;
    QStringList all_extensions;

    foreach (Decoder *d, m_decoders) {
	// loop over all mime types that the decoder supports
	QList<CodecBase::MimeType> types = d->mimeTypes();
	QListIterator<CodecBase::MimeType> ti(types);
	while (ti.hasNext()) {
	    CodecBase::MimeType type = ti.next();
	    QString extensions = type.patterns.join(" ");

	    // skip if extensions are already known/present
	    if (list.join("\n").contains(extensions)) continue;

	    // otherwise append to the list
	    all_extensions += type.patterns;
	    QString entry = extensions;
	    QString comment = type.description.replace(QRegExp("/"), ",");
	    entry += "|" + comment;
	    list.append(entry + " (" + extensions + ")");
	}
    }
    list.sort();
    list.prepend("*|" + i18n("All Files"));
    list.prepend(all_extensions.join(" ") + "|" + i18n("All Supported Files"));

    QString str_list = list.join("\n");
    Q_ASSERT(!str_list.contains('/'));
    if (str_list.contains('/')) {
	qWarning("CodecManager::decodingFilter() -> '%s'",
	         str_list.toLocal8Bit().data());
    }

    return str_list;
}

//***************************************************************************
#include "CodecManager.moc"
//***************************************************************************
//***************************************************************************
