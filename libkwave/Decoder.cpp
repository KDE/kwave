/***************************************************************************
            Decoder.cpp  -  abstract base class of all decoders
			     -------------------
    begin                : Jun 01 2002
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

#include <kfile.h>
#include <kmimetype.h>
#include <kurl.h>

#include <Decoder.h>

/***************************************************************************/
Decoder::Decoder()
    :QObject(), CodecBase()
{
}

/***************************************************************************/
QString Decoder::whatContains(const KURL &url)
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
