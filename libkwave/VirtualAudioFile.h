/*************************************************************************
     VirtualAudioFile.h  -  adapter between QIODevice and libaudiofile
                             -------------------
    begin                : Mon May 06 2002
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

#ifndef _VIRTUAL_AUDIO_FILE_H_
#define _VIRTUAL_AUDIO_FILE_H_

#include "config.h"

#include <kdemacros.h>

#include <QMap>

extern "C" {
#include <audiofile.h>
#include <af_vfs.h>
}

class QIODevice;

namespace Kwave
{

    /**
     * This class builds an interface between a QIODevice and a virtual
     * file in libaudiofile.
     */
    class KDE_EXPORT VirtualAudioFile
    {
    public:
	/**
	 * Constructor
	 * @param device QIODevice used as source/destination
	 */
	VirtualAudioFile(QIODevice &device);

	/** Destructor */
	virtual ~VirtualAudioFile();

	/** opens the file through libaudiofile */
	virtual void open(Kwave::VirtualAudioFile *x, AFfilesetup setup);

	/**
	 * Closes the file from libaudiofile side. The associated
	 * QIODevice will not be touched.
	 * @note This has not necessarily to be called, it will closed
	 *       automatically in the destructor.
	 */
	virtual void close();

	/** Returns the handle for use in libaudiofile */
	inline AFfilehandle &handle() { return m_file_handle; }

	/** Returns the virtual file for use in libaudiofile */
	inline AFvirtualfile *file() { return m_virtual_file; }

	/** Returns the last error from libaudiofile (-1 means "no error") */
	inline long int lastError() { return m_last_error; }

	/**
	 * returns the last error text from libaudiofile, not localized
	 * @note this is only valid in case of lastError is not -1
	 */
	inline QString lastErrorText() { return m_last_error_text; }

	/** reads a block of data */
	virtual unsigned int read(char *data, unsigned int nbytes);

	/** returns the length of the file */
	virtual qint64 length();

	/** writes a block of data */
	virtual unsigned int write(const char *data, unsigned int nbytes);

	/** called to close the source */
	virtual void destroy();

	/** seek to a file position */
	virtual qint64 seek(qint64 offset, bool is_relative);

	/** returns the file position */
	virtual qint64 tell();

	/** returns a VirtualAudioFile for a libasound virtual file */
	static Kwave::VirtualAudioFile *adapter(AFvirtualfile *vfile);

    private:

	/** i/o device to Qt */
	QIODevice &m_device;

	/** file handle used in libaudiofile */
	AFfilehandle m_file_handle;

	/** virtual file, used in libaudiofile */
	AFvirtualfile *m_virtual_file;

	/** last error code from libaudiofile */
	long int m_last_error;

	/** last error text from libaudiofile */
	QString m_last_error_text;

    };
}

#endif /* _VIRTUAL_AUDIO_FILE_H_ */

//***************************************************************************
//***************************************************************************
