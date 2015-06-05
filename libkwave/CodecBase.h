/*************************************************************************
            CodecBase.h  -  base class for Encoder and Decoder
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

#ifndef CODEC_BASE_H
#define CODEC_BASE_H

#include "config.h"

#include <QtGlobal>
#include <QList>
#include <QString>
#include <QStringList>

class QMimeType;
class QUrl;

namespace Kwave
{

    class Q_DECL_EXPORT CodecBase
    {
    public:

	/** simplified mime type: contains only name and list of patterns */
	typedef struct {
	    QString name;
	    QString description;
	    QStringList patterns;
	} MimeType;

	/** Constructor */
	CodecBase();

	/** Destructor */
	virtual ~CodecBase();

	/** Returns true if the given mime type is supported */
	virtual bool supports(const QMimeType &mimetype);

	/** Returns true if the given mime type is supported */
	virtual bool supports(const QString &mimetype_name);

	/** Returns a list of supported file extensions */
	virtual QStringList extensions(const QString &mimetype_name) const;

	/** Returns a list of supported mime types */
	virtual const QList<CodecBase::MimeType> mimeTypes();

	/** Returns a list of supported compression types */
	virtual const QList<int> compressionTypes();

	/**
	 * Adds a new mime type to the internal list of supported mime
	 * types. First it tries to find the mime type in the system,
	 * if none was found, a new mime type is created, using the
	 * passed parameters. The system's mime types are always preferred
	 * over the passed 'built-ins'.
	 * @param name the mime type's name
	 * @param description verbose description
	 * @param patterns list of file patterns, passed as a single string,
	 *                 separated by "; "
	 */
	virtual void addMimeType(const char *name,
	                         const QString &description,
	                         const char *patterns);

	/**
	 * Adds a new compression type to the internal list of supported
	 * compression types.
	 * @param compression the compression type
	 */
	virtual void addCompression(int compression);

	/**
	 * Tries to find the name of a mime type by a URL. If not found, it
	 * returns the default mime type, never an empty string.
	 * @param url a QUrl, only the filename's extension will be inspected
	 * @return name of the mime type or the default mime type
	 */
	virtual QString whatContains(const QUrl &url);

    private:

	/** list of supported mime types */
	QList<MimeType> m_supported_mime_types;

	/** list of supported compression types */
	QList<int> m_supported_compression_types;

    };
}

#endif /* CODEC_BASE_H */

//***************************************************************************
//***************************************************************************
