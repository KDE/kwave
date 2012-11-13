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
#include <plugins/codec_wav/WavFileFormat.h>

//***************************************************************************
/* static initializers */
QList<Kwave::Encoder *> Kwave::CodecManager::m_encoders;
QList<Kwave::Decoder *> Kwave::CodecManager::m_decoders;

//***************************************************************************
//***************************************************************************
Kwave::CodecManager::CodecManager()
{
}

//***************************************************************************
Kwave::CodecManager::~CodecManager()
{
    Q_ASSERT(m_encoders.isEmpty());
    Q_ASSERT(m_decoders.isEmpty());
}

//***************************************************************************
void Kwave::CodecManager::registerEncoder(Kwave::Encoder &encoder)
{
    if (m_encoders.contains(&encoder)) return; /* already known */
    m_encoders.append(&encoder);
}

//***************************************************************************
void Kwave::CodecManager::registerDecoder(Kwave::Decoder &decoder)
{
    if (m_decoders.contains(&decoder)) return; /* already known */
    m_decoders.append(&decoder);
}

//***************************************************************************
bool Kwave::CodecManager::canDecode(const KMimeType &mimetype)
{
    foreach (Kwave::Decoder *d, m_decoders)
	if (d && d->supports(mimetype)) return true;
    return false;
}

//***************************************************************************
bool Kwave::CodecManager::canDecode(const QString &mimetype_name)
{
    foreach (Kwave::Decoder *d, m_decoders)
	if (d && d->supports(mimetype_name)) return true;
    return false;
}

//***************************************************************************
QString Kwave::CodecManager::whatContains(const KUrl &url)
{
    foreach (Kwave::Decoder *d, m_decoders) {
	if (!d) continue;
	QString mime_type = d->whatContains(url);
	if (mime_type != KMimeType::defaultMimeType()) return mime_type;
    }
    foreach (Kwave::Encoder *e, m_encoders) {
	if (!e) continue;
	QString mime_type = e->whatContains(url);
	if (mime_type != KMimeType::defaultMimeType()) return mime_type;
    }
    return KMimeType::findByUrl(url)->name();
}

//***************************************************************************
QStringList Kwave::CodecManager::encodingMimeTypes()
{
    QStringList list;
    foreach (Kwave::Encoder *e, m_encoders) {
	if (!e) continue;
	foreach (const Kwave::CodecBase::MimeType &mime_type, e->mimeTypes()) {
	    QString name = mime_type.name;
	    if (list.isEmpty() || !list.contains(name))
		list.append(name);
	}
    }
    return list;
}

//***************************************************************************
Kwave::Decoder *Kwave::CodecManager::decoder(const QString &mimetype_name)
{
    foreach (Kwave::Decoder *d, m_decoders)
	if (d && d->supports(mimetype_name)) return d->instance();
    return 0;
}

//***************************************************************************
Kwave::Decoder *Kwave::CodecManager::decoder(const KMimeType &mimetype)
{
    return decoder(mimetype.name());
}

//***************************************************************************
Kwave::Decoder *Kwave::CodecManager::decoder(const QMimeData *mime_data)
{
    if (!mime_data) return 0;

    foreach (QString format, mime_data->formats()) {
	Kwave::Decoder *d = decoder(format);
	if (d) return d;
    }
    return 0;
}

//***************************************************************************
Kwave::Encoder *Kwave::CodecManager::encoder(const QString &mimetype_name)
{
    foreach (Kwave::Encoder *e, m_encoders)
	if (e && e->supports(mimetype_name)) return e->instance();
    return 0;
}

//***************************************************************************
QString Kwave::CodecManager::encodingFilter()
{
    QStringList list;
    foreach (Kwave::Encoder *e, m_encoders) {
	// loop over all mime types that the encoder supports
	QList<Kwave::CodecBase::MimeType> types = e->mimeTypes();
	QListIterator<Kwave::CodecBase::MimeType> ti(types);
	while (ti.hasNext()) {
	    Kwave::CodecBase::MimeType type = ti.next();
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
QString Kwave::CodecManager::decodingFilter()
{
    QStringList list;
    QStringList all_extensions;

    foreach (Kwave::Decoder *d, m_decoders) {
	// loop over all mime types that the decoder supports
	QList<Kwave::CodecBase::MimeType> types = d->mimeTypes();
	QListIterator<Kwave::CodecBase::MimeType> ti(types);
	while (ti.hasNext()) {
	    Kwave::CodecBase::MimeType type = ti.next();
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

    // builtin type for macro files
    all_extensions += "*.kwave";
    list.append("*.kwave|" + i18n("Kwave Macro Files") + " (*.kwave)");

    // special entries for "all" and "all supported"
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
using namespace Kwave;
#include "CodecManager.moc"
//***************************************************************************
//***************************************************************************
