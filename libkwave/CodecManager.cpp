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

#include <QLatin1Char>
#include <QMimeData>
#include <QRegExp>
#include <QMimeDatabase>
#include <QMimeType>

#include "libkwave/CodecManager.h"
#include "libkwave/Decoder.h"
#include "libkwave/Encoder.h"
#include "libkwave/String.h"

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
void Kwave::CodecManager::unregisterEncoder(Kwave::Encoder *encoder)
{
    if (!m_encoders.contains(encoder)) return; /* unknown */
    m_encoders.removeAll(encoder);
}

//***************************************************************************
void Kwave::CodecManager::registerDecoder(Kwave::Decoder &decoder)
{
    if (m_decoders.contains(&decoder)) return; /* already known */
    m_decoders.append(&decoder);
}

//***************************************************************************
void Kwave::CodecManager::unregisterDecoder(Kwave::Decoder *decoder)
{
    if (!m_decoders.contains(decoder)) return; /* unknown */
    m_decoders.removeAll(decoder);
}

//***************************************************************************
bool Kwave::CodecManager::canDecode(const QMimeType &mimetype)
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
QString Kwave::CodecManager::whatContains(const QUrl &url)
{
    foreach (Kwave::Decoder *d, m_decoders) {
	if (!d) continue;
	QString mime_type = d->whatContains(url);
	if (mime_type != QMimeType().name()) return mime_type;
    }
    foreach (Kwave::Encoder *e, m_encoders) {
	if (!e) continue;
	QString mime_type = e->whatContains(url);
	if (mime_type != QMimeType().name()) return mime_type;
    }

    QMimeDatabase db;
    return db.mimeTypeForUrl(url).name();
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
Kwave::Decoder *Kwave::CodecManager::decoder(const QMimeType &mimetype)
{
    return decoder(mimetype.name());
}

//***************************************************************************
Kwave::Decoder *Kwave::CodecManager::decoder(const QMimeData *mime_data)
{
    if (!mime_data) return 0;

    foreach (const QString &format, mime_data->formats()) {
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
	    QString extensions = type.patterns.join(_(" "));

	    // skip if extensions are already known/present
	    if (!list.isEmpty() && list.join(_("\n")).contains(extensions))
		continue;

	    // otherwise append to the list
	    QString entry = extensions;
	    QString comment = type.description.replace(
		QRegExp(_("/")), _(","));
	    entry += _("|") + comment;
	    list.append(entry + _(" (") + extensions + _(")"));
	}
    }
    list.sort();
    QString str_list = list.join(_("\n"));
    Q_ASSERT(!str_list.contains(QLatin1Char('/')));
    if (str_list.contains(QLatin1Char('/'))) {
	qWarning("CodecManager::encodingFilter() -> '%s'", DBG(str_list));
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
	    QString extensions = type.patterns.join(_(" "));

	    // skip if extensions are already known/present
	    if (!list.isEmpty() && list.join(_("\n")).contains(extensions))
		continue;

	    // otherwise append to the list
	    all_extensions += type.patterns;
	    QString entry = extensions;
	    QString comment =
		type.description.replace(QRegExp(_("/")), _(","));
	    entry += _("|") + comment;
	    list.append(entry +	_(" (") + extensions + _(")"));
	}
    }

    // builtin type for macro files
    all_extensions += _("*.kwave");
    list.append(_("*.kwave|") + i18n("Kwave Macro Files") + _(" (*.kwave)"));

    // special entries for "all" and "all supported"
    list.sort();
    list.prepend(_("*|") + i18n("All Files"));
    list.prepend(all_extensions.join(_(" ")) + _("|") +
                 i18n("All Supported Files"));

    QString str_list = list.join(_("\n"));
    Q_ASSERT(!str_list.contains(QLatin1Char('/')));
    if (str_list.contains(QLatin1Char('/'))) {
	qWarning("CodecManager::decodingFilter() -> '%s'", DBG(str_list));
    }

    return str_list;
}

//***************************************************************************
//***************************************************************************
