/***************************************************************************
           FileLoader.h  -  loader for loading and buffering small files
			     -------------------
    begin                : Jan 20 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#ifndef _FILE_LOADER_H_
#define _FILE_LOADER_H_

#include <qcstring.h>

class QString;

class FileLoader {
public:
    /**
     * Constructor. Allocates a buffer and reads a file into it.
     * The last byte of the file will not be read and will be
     * internally set to zero, so that the file can be processed
     * like a large string.
     * @param name path to the file to be loaded.
     */
    FileLoader (const QString &name);

    /** Destructor, frees the buffer with the content of the file */
    ~FileLoader ();

    /** Returns the buffer with the file's content. */
    const QByteArray &buffer();

private:
    /** internal buffer */
    QByteArray m_buf;
};

#endif /* _FILE_LOADER_H_ */
