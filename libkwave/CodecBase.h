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

#ifndef _CODEC_BASE_H_
#define _CODEC_BASE_H_

#include "config.h"
#include <qptrlist.h>

class KMimeType;
class QString;

class CodecBase
{
public:

    /** Constructor */
    CodecBase();

    /** Destructor */
    virtual ~CodecBase();

    /** Returns true if the given mime type is supported */
    virtual bool supports(const KMimeType &mimetype);

    /** Returns true if the given mime type is supported */
    virtual bool supports(const QString &mimetype_name);

    /** Returns a list of supported mime types */
    virtual const QPtrList<KMimeType> &mimeTypes();

    /**
     * Adds a new mime type to the internal list of supported mime
     * types. First it tries to find the mime type in the system,
     * if none was found, a new mime type is created, using the
     * passed parameters. The system's mime types are always preferred
     * over the passed 'built-ins'.
     * @param name the mime type's name
     * @param description verbose description
     * @patterns list of file patterns, passed as a single string,
     *           separated by "; "
     */
    virtual void addMimeType(const QString &name,
                             const QString &description,
                             const QString &patterns);

private:
    /** list of supported mime types */
    QPtrList<KMimeType> m_supported_mime_types;

};

#endif /* _CODEC_BASE_H_ */
