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
#include <qmap.h>

extern "C" {
#include "libaudiofile/audiofile.h" // from libaudiofile
#include "libaudiofile/af_vfs.h"    // from libaudiofile
}

/**
 * This class builds an interface between a QIODevice and a virtual
 * file in libaudiofile.
 */
class VirtualAudioFile
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
    virtual void open(VirtualAudioFile *x, AFfilesetup setup);

    /** Returns the handle for use in libaudiofile */
    inline AFfilehandle &handle() { return m_file_handle; };

    /** Returns the virtual file for use in libaudiofile */
    inline AFvirtualfile *file() { return m_virtual_file; };

    /** Returns the las error from libaudiofile (-1 means "no error") */
    inline long lastError() { return m_last_error; };

    /** reads a block of data */
    virtual unsigned int read(char *data, unsigned int nbytes);

    /** returns the length of the file */
    virtual long length();

    /** writes a block of data */
    virtual unsigned int write(const char *data, unsigned int nbytes);

    /** called to close the source */
    virtual void destroy();

    /** seek to a file position */
    virtual long seek(long offset, int is_relative);

    /** returns the file position */
    virtual long tell();

    /** returns a VirtualAudioFile for a libasound virtual file */
    static VirtualAudioFile *adapter(AFvirtualfile *vfile);

private:

    /** i/o device to Qt */
    QIODevice &m_device;

    /** file handle used in libaudiofile */
    AFfilehandle m_file_handle;

    /** virtual file, used in libaudiofile */
    AFvirtualfile *m_virtual_file;

    /** last error code from libaudiofile */
    long m_last_error;

};

#endif /* _VIRTUAL_AUDIO_FILE_H_ */
