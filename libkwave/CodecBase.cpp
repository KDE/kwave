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

#include <QString>
#include <QStringList>
#include <QtGlobal>

#include <kmimetype.h>
#include <kfile.h>
#include <kurl.h>

#include "CodecBase.h"

/***************************************************************************/
CodecBase::CodecBase()
    :m_supported_mime_types(), m_supported_compression_types()
{
}

/***************************************************************************/
CodecBase::~CodecBase()
{
    m_supported_mime_types.clear();
    m_supported_compression_types.clear();
}

/***************************************************************************/
void CodecBase::addMimeType(const QString &name, const QString &description,
                            const QString &patterns)
{
    // check for duplicates
    if (supports(name)) return;

    CodecBase::MimeType type;
    KMimeType::Ptr t = KMimeType::mimeType(name);

    if (!t || (t && t->isDefault())) {
// 	qWarning("mime type '%s' not registered, using built-in!",
// 	         name.toLocal8Bit().data());
	type.name        = name;
	type.description = description;
	type.patterns    = patterns.split("; ", QString::SkipEmptyParts);
    } else {
	type.description = t->comment();
	type.patterns    = t->patterns();

	if (t->name() != name) {
	    // type has been translated (maybe un-alias'ed)
	    // manually add the original name
	    type.name    = name;
	    m_supported_mime_types.append(type);
	}

	if  (!supports(t->name())) {
	    // new type or new alias
	    type.name        = t->name();
	}
    }
    m_supported_mime_types.append(type);
}

/***************************************************************************/
void CodecBase::addCompression(int compression)
{
    if (m_supported_compression_types.contains(compression)) return;

    m_supported_compression_types.append(compression);
    qSort(m_supported_compression_types);
}

/***************************************************************************/
bool CodecBase::supports(const KMimeType &mimetype)
{
    return supports(mimetype.name());
}

/***************************************************************************/
bool CodecBase::supports(const QString &mimetype_name)
{
    foreach (const CodecBase::MimeType &mime, m_supported_mime_types) {
	if (mime.name == mimetype_name) return true;
    }
    return false;
}

/***************************************************************************/
QStringList CodecBase::extensions(const QString &mimetype_name) const
{
    foreach (const CodecBase::MimeType &mime, m_supported_mime_types) {
	if (mime.name == mimetype_name) {
	    return mime.patterns;
	}
    }
    return QStringList();
}

/***************************************************************************/
const QList<CodecBase::MimeType> CodecBase::mimeTypes()
{
    return m_supported_mime_types;
}

/***************************************************************************/
const QList<int> CodecBase::compressionTypes()
{
    return m_supported_compression_types;
}

/***************************************************************************/
QString CodecBase::whatContains(const KUrl &url)
{
    // get the extension of the file
    QFileInfo file(url.fileName());
    QString suffix = file.suffix();
    if (!suffix.length()) return KMimeType::defaultMimeType();
    suffix = "*."+suffix;

    // try to find in the list of supported mime types
    QListIterator<CodecBase::MimeType> it(m_supported_mime_types);
    while (it.hasNext()) {
	CodecBase::MimeType mime_type = it.next();
	if (mime_type.patterns.contains(suffix))
	    return mime_type.name;
    }
    return KMimeType::defaultMimeType();
}

/***************************************************************************/
/***************************************************************************/
