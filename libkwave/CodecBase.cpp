/*************************************************************************
         CodecBase.cpp  -  base class for Encoder and Decoder
                             -------------------
    begin                : Mon Mar 11 2002
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

#include <algorithm>

#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>
#include <QString>
#include <QStringList>
#include <QtGlobal>
#include <QUrl>

#include "libkwave/CodecBase.h"
#include "libkwave/String.h"

/***************************************************************************/
Kwave::CodecBase::CodecBase()
    :m_supported_mime_types(), m_supported_compression_types()
{
}

/***************************************************************************/
Kwave::CodecBase::~CodecBase()
{
    m_supported_mime_types.clear();
    m_supported_compression_types.clear();
}

/***************************************************************************/
void Kwave::CodecBase::addMimeType(const char *name,
                                   const QString &description,
                                   const char *patterns)
{
    const QString type_name = _(name);
    if (type_name.contains(_(","))) {
        // list of mime types -> call recursively for each of them
        QStringList types = type_name.split(_(","), Qt::SkipEmptyParts);
        foreach (const QString &mt, types) {
            addMimeType(mt.trimmed().toUtf8().data(), description, patterns);
        }
        return;
    }

    Kwave::CodecBase::MimeType type;
    QMimeDatabase db;
    QMimeType t = db.mimeTypeForName(type_name);

    if (t.isDefault() || t.name().isEmpty()) {
// 	qWarning("mime type '%s' not registered, using built-in!", name);
	type.name        = type_name;
	type.description = description;
	type.patterns    = _(patterns).split(_("; "), Qt::SkipEmptyParts);
    } else {
	type.description = t.comment();
	type.patterns    = t.globPatterns();

	if (t.name() != type_name) {
	    // type has been translated (maybe un-alias'ed)
	    // manually add the original name
	    type.name    = type_name;
	    m_supported_mime_types.append(type);
	}

	if  (!supports(t.name())) {
	    // new type or new alias
	    type.name    = t.name();
	}
    }
    m_supported_mime_types.append(type);
}

/***************************************************************************/
void Kwave::CodecBase::addCompression(Kwave::Compression::Type compression)
{
    if (m_supported_compression_types.contains(compression)) return;

    m_supported_compression_types.append(compression);
    std::sort(m_supported_compression_types.begin(),
              m_supported_compression_types.end(), std::greater<int>());
}

/***************************************************************************/
bool Kwave::CodecBase::supports(const QMimeType &mimetype)
{
    return supports(mimetype.name());
}

/***************************************************************************/
bool Kwave::CodecBase::supports(const QString &mimetype_name)
{
    foreach (const Kwave::CodecBase::MimeType &mime, m_supported_mime_types) {
	if (mime.name == mimetype_name) return true;
    }
    return false;
}

/***************************************************************************/
QStringList Kwave::CodecBase::extensions(const QString &mimetype_name) const
{
    QStringList result;
    foreach (const Kwave::CodecBase::MimeType &mime, m_supported_mime_types) {
	if (mime.name == mimetype_name) {
	    foreach (const QString &ext, mime.patterns)
                if (!result.contains(ext)) result.append(ext);
	}
    }
    return result;
}

/***************************************************************************/
const QList<Kwave::CodecBase::MimeType> Kwave::CodecBase::mimeTypes()
{
    return m_supported_mime_types;
}

/***************************************************************************/
const QList<Kwave::Compression::Type> Kwave::CodecBase::compressionTypes()
{
    return m_supported_compression_types;
}

/***************************************************************************/
QString Kwave::CodecBase::mimeTypeOf(const QUrl &url)
{
    // get the extension of the file
    QFileInfo file(url.fileName());
    QString suffix = file.suffix();

    if (!suffix.length()) return QMimeType().name();
    suffix = _("*.") + suffix;

    // try to find in the list of supported mime types
    QListIterator<Kwave::CodecBase::MimeType> it(m_supported_mime_types);
    while (it.hasNext()) {
	Kwave::CodecBase::MimeType mime_type = it.next();
	if (mime_type.patterns.contains(suffix.toLower()))
	    return mime_type.name;
    }
    return QMimeType().name();
}

/***************************************************************************/
/***************************************************************************/
