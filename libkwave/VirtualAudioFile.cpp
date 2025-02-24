/*************************************************************************
   VirtualAudioFile.cpp  -  adapter between QIODevice and libaudiofile
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

#include "config.h"
#include <QIODevice>
#include <stdlib.h> // for calloc()
#include <unistd.h>

#include <new>

#include "libkwave/Utils.h"
#include "libkwave/VirtualAudioFile.h"

/**
 * map for finding the corresponding VirtualAudioFile
 * adapter to a AFvirtualfile from libaudiofile
 */
static QMap<AFvirtualfile*, Kwave::VirtualAudioFile*> *_adapter_map = nullptr;

/** Last error number from libaudiofile. -1 if no error occurred */
static long _last_audiofile_error = -1;

/** Last error text from libaudiofile, empty if no error occurred */
static QString _last_audiofile_error_text;

//***************************************************************************
/**
 * Error handler for libaudiofile
 * @warning NOT THREADSAFE!
 * @param error the numeric error code, >=0, defined in audiofile.h,
 *        something starting with AF_BAD_...
 * @param str text, not localized, so not usable for us :-(
 */
static void _handle_audiofile_error(long int error, const char *str)
{
    qDebug("libaudiofile error %ld: '%s'", error, str);
    _last_audiofile_error = error;
    _last_audiofile_error_text = QString::fromLatin1(str);
}

//***************************************************************************
/** Returns the last libaudiofile error and resets it to -1 */
static long _lastAudiofileError()
{
    long err = _last_audiofile_error;
    _last_audiofile_error = -1;

    // ignore "bad alloc", which might occur on a "malloc(0)"
    if (err == AF_BAD_MALLOC) err = -1;
    return err;
}

//***************************************************************************
static ssize_t af_file_read(AFvirtualfile *vfile, void *data,
                            size_t nbytes)
{
    Kwave::VirtualAudioFile *adapter = Kwave::VirtualAudioFile::adapter(vfile);
    return (adapter) ?
        static_cast<ssize_t>(adapter->read(
            static_cast<char *>(data),
            Kwave::toUint(nbytes)
        )) : 0;
}

//***************************************************************************
static AFfileoffset af_file_length(AFvirtualfile *vfile)
{
    Kwave::VirtualAudioFile *adapter = Kwave::VirtualAudioFile::adapter(vfile);
    return (adapter) ? static_cast<AFfileoffset>(adapter->length()) : -1;
}

//***************************************************************************
static ssize_t af_file_write(AFvirtualfile *vfile, const void *data,
                             size_t nbytes)
{
    Kwave::VirtualAudioFile *adapter = Kwave::VirtualAudioFile::adapter(vfile);
    return (adapter) ?
        static_cast<ssize_t>(adapter->write(
            static_cast<const char *>(data),
            Kwave::toUint(nbytes)
        )) : 0;
}

//***************************************************************************
static void af_file_destroy(AFvirtualfile */*vfile*/)
{
}

//***************************************************************************
static AFfileoffset af_file_seek(AFvirtualfile *vfile, AFfileoffset offset,
                                 int is_relative)
{
    Kwave::VirtualAudioFile *adapter = Kwave::VirtualAudioFile::adapter(vfile);
    return (adapter) ?
        static_cast<AFfileoffset>(adapter->seek(
            static_cast<qint64>(offset),
            (is_relative != 0)
        )) : -1;
}

//***************************************************************************
static AFfileoffset af_file_tell(AFvirtualfile *vfile)
{
    Kwave::VirtualAudioFile *adapter = Kwave::VirtualAudioFile::adapter(vfile);
    return (adapter) ? static_cast<AFfileoffset>(adapter->tell()) : -1;
}

//***************************************************************************
/**
 * Replacement of af_virtual_file_new from original libaudiofile code.
 * Unfortunately the original is not usable because it is not available
 * through the shared library API of some libaudiofile versions.
 *
 * original version: see libaudiofile/af_vfs.c (GPL 2+)
 * original author: Copyright (C) 1999, Elliot Lee <sopwith@redhat.com>
 */
static AFvirtualfile *__af_virtual_file_new(void)
{
    return static_cast<AFvirtualfile *>(calloc(1, sizeof(AFvirtualfile)));
}

//***************************************************************************
//***************************************************************************
Kwave::VirtualAudioFile::VirtualAudioFile(QIODevice &device)
     :m_device(device), m_file_handle(nullptr), m_virtual_file(nullptr),
      m_last_error(-1), m_last_error_text()
{
    // create the virtual file structure for libaudiofile
    m_virtual_file = __af_virtual_file_new();
    Q_ASSERT(m_virtual_file);
    if (!m_virtual_file) return;

    // enter our wrapper functions
    m_virtual_file->closure = nullptr;
    m_virtual_file->read    = af_file_read;
    m_virtual_file->write   = af_file_write;
    m_virtual_file->length  = af_file_length;
    m_virtual_file->destroy = af_file_destroy;
    m_virtual_file->seek    = af_file_seek;
    m_virtual_file->tell    = af_file_tell;
}

//***************************************************************************
void Kwave::VirtualAudioFile::open(Kwave::VirtualAudioFile *x,
                                   AFfilesetup setup)
{
    // register ourself
    adapter(nullptr); // dummy lookup, for creating a new map if needed
    Q_ASSERT(_adapter_map);
    if (_adapter_map) _adapter_map->insert(m_virtual_file, x);

    // determine the mode: rw/w/r
    const char *mode = nullptr;
    if      (m_device.isWritable()) mode = "w";
    else if (m_device.isReadable()) mode = "r";
    Q_ASSERT(mode);

    AFerrfunc old_handler;
    old_handler = afSetErrorHandler(_handle_audiofile_error);

    // reset the file position when opening the device, otherwise libaudiofile
    // might fail when seeking to the current position and the position of
    // the device currently is at EOF (in libaudiofile, File::canSeek)
    m_device.seek(0);

    // open the virtual file and get a handle for it
    m_file_handle     = afOpenVirtualFile(m_virtual_file, mode, setup);
    m_last_error      = _lastAudiofileError();
    m_last_error_text = _last_audiofile_error_text;

    afSetErrorHandler(old_handler);
}

//***************************************************************************
void Kwave::VirtualAudioFile::close()
{
    // close libaudiofile stuff
    afCloseFile(m_file_handle);

    // de-register ourself
    if (_adapter_map) _adapter_map->remove(m_virtual_file);

    m_virtual_file = nullptr;
    m_file_handle  = nullptr;
}

//***************************************************************************
Kwave::VirtualAudioFile::~VirtualAudioFile()
{
    if (m_virtual_file) close();
}

//***************************************************************************
qint64 Kwave::VirtualAudioFile::read(char *data, unsigned int nbytes)
{
    Q_ASSERT(data);
    if (!data) return 0;
    return m_device.read(data, nbytes);
}

//***************************************************************************
qint64 Kwave::VirtualAudioFile::length()
{
    return m_device.size();
}

//***************************************************************************
qint64 Kwave::VirtualAudioFile::write(const char *data, unsigned int nbytes)
{
    Q_ASSERT(data);
    if (!data) return 0;
    return m_device.write(data, nbytes);
}

//***************************************************************************
void Kwave::VirtualAudioFile::destroy()
{
}

//***************************************************************************
qint64 Kwave::VirtualAudioFile::seek(qint64 offset, bool is_relative)
{
    qint64 abs_pos = (is_relative) ? (m_device.pos() + offset) : offset;
    if (abs_pos >= m_device.size())
        return -1; // avoid seek after EOF
    bool ok = m_device.seek(abs_pos);
    return (ok) ? m_device.pos() : -1;
}

//***************************************************************************
qint64 Kwave::VirtualAudioFile::tell()
{
    return m_device.pos();
}

//***************************************************************************
Kwave::VirtualAudioFile *Kwave::VirtualAudioFile::adapter(AFvirtualfile *vfile)
{
    // create a new empty map if necessary
    if (!_adapter_map) _adapter_map =
        new(std::nothrow) QMap<AFvirtualfile*,VirtualAudioFile*>();
    Q_ASSERT(_adapter_map);
    if (!_adapter_map) return nullptr;

    // lookup in the map
    return _adapter_map->contains(vfile) ? (*_adapter_map)[vfile] : nullptr;
}

//***************************************************************************
//***************************************************************************
