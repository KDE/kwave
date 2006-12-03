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
#include <qstring.h>
#include <qstringlist.h>
#include <kmimetype.h>
#include <kfile.h>
#include <kurl.h>

#include "CodecBase.h"

/***************************************************************************/
CodecBase::CodecBase()
    :m_supported_mime_types()
{
    m_supported_mime_types.setAutoDelete(true);
}

/***************************************************************************/
CodecBase::~CodecBase()
{
    m_supported_mime_types.setAutoDelete(true);
    m_supported_mime_types.clear();
}

/***************************************************************************/
void CodecBase::addMimeType(const QString &name, const QString &description,
                            const QString &patterns)
{
    KMimeType *type;
    type = new KMimeType(*KMimeType::mimeType(name));
    Q_ASSERT(type);
    if (type && (type->name() == type->defaultMimeType())) {
//	qWarning("mime type '"+name+"' not registered, using built-in!");
	delete type;
	type = 0;

	QStringList p = QStringList::split("; ", patterns, false);
	type = new KMimeType(0, name, "sound", description, p);
    }
    if (type) m_supported_mime_types.append(type);
}

/***************************************************************************/
bool CodecBase::supports(const KMimeType &mimetype)
{
    QPtrListIterator<KMimeType> it(m_supported_mime_types);
    const QString &name = mimetype.name();
    for (; it.current(); ++it) {
	if (it.current()->name() == name) return true;
    }
    return false;
}

/***************************************************************************/
bool CodecBase::supports(const QString &mimetype_name)
{
    QPtrListIterator<KMimeType> it(m_supported_mime_types);
    for (; it.current(); ++it) {
	if (it.current()->name() == mimetype_name) return true;
    }
    return false;
}

/***************************************************************************/
const QPtrList<KMimeType> &CodecBase::mimeTypes()
{
    return m_supported_mime_types;
}

/***************************************************************************/
QString CodecBase::whatContains(const KURL &url)
{
    // get the extension of the file
    QFileInfo file(url.fileName());
    QString extension = file.extension(false);
    if (!extension.length()) return KMimeType::defaultMimeType();
    extension = "*."+extension;

    // try to find in the list of supported mime types
    QPtrListIterator<KMimeType> it(mimeTypes());
    for (; it.current(); ++it) {
	KMimeType &mime_type = *(it.current());
	if (mime_type.patterns().contains(extension))
	    return mime_type.name();
    }
    return KMimeType::defaultMimeType();
}

/***************************************************************************/
/***************************************************************************/
