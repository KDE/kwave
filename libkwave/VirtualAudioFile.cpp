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
#include <stdlib.h>
#include <qiodevice.h>
#include "libkwave/VirtualAudioFile.h"

/**
 * map for finding the corresponding VirtualAudioFile
 * adapter to a AFvirtualfile from libaudiofile
 */
static QMap<AFvirtualfile*,VirtualAudioFile*> *_adapter_map = 0;

/** Last error number from libaudiofile. -1 if no error occurred */
static long _last_audiofile_error = -1;

//***************************************************************************
/**
 * Error handler for libaudiofile
 * @warning NOT THREADSAFE!
 * @param error the numeric error code, >=0, defined in audiofile.h,
 *        something starting with AF_BAD_...
 * @param error text, not localized, so not usable for us :-(
 */
static void _handle_audiofile_error(long error, const char *str)
{
    qDebug("libaudiofile error %ld: '%s'", error, str);
    _last_audiofile_error = error;
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
    VirtualAudioFile *adapter = VirtualAudioFile::adapter(vfile);
    return (adapter) ? adapter->read(static_cast<char *>(data), nbytes) : 0;
}

//***************************************************************************
static long af_file_length(AFvirtualfile *vfile)
{
    VirtualAudioFile *adapter = VirtualAudioFile::adapter(vfile);
    return (adapter) ? adapter->length() : -1;
}

//***************************************************************************
static ssize_t af_file_write(AFvirtualfile *vfile, const void *data,
	                     size_t nbytes)
{
    VirtualAudioFile *adapter = VirtualAudioFile::adapter(vfile);
    return (adapter) ?
	adapter->write(static_cast<const char *>(data), nbytes) : 0;
}

//***************************************************************************
static void af_file_destroy(AFvirtualfile */*vfile*/)
{
}

//***************************************************************************
static long af_file_seek(AFvirtualfile *vfile, long offset, int is_relative)
{
    VirtualAudioFile *adapter = VirtualAudioFile::adapter(vfile);
    return (adapter) ? adapter->seek(offset, is_relative) : -1;
}

//***************************************************************************
static long af_file_tell(AFvirtualfile *vfile)
{
    VirtualAudioFile *adapter = VirtualAudioFile::adapter(vfile);
    return (adapter) ? adapter->tell() : -1;
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
    return static_cast<AFvirtualfile *>(calloc(sizeof(AFvirtualfile), 1));
}

//***************************************************************************
//***************************************************************************
VirtualAudioFile::VirtualAudioFile(QIODevice &device)
     :m_device(device), m_file_handle(0), m_virtual_file(0),
      m_last_error(-1)
{
    // create the virtual file structure for libaudiofile
    m_virtual_file = __af_virtual_file_new();
    Q_ASSERT(m_virtual_file);
    if (!m_virtual_file) return;

    // enter our wrapper functions
    m_virtual_file->closure = 0;
    m_virtual_file->read    = af_file_read;
    m_virtual_file->write   = af_file_write;
    m_virtual_file->length  = af_file_length;
    m_virtual_file->destroy = af_file_destroy;
    m_virtual_file->seek    = af_file_seek;
    m_virtual_file->tell    = af_file_tell;
}

//***************************************************************************
void VirtualAudioFile::open(VirtualAudioFile *x, AFfilesetup setup)
{
    // register ourself
    adapter(0); // dummy lookup, for creating a new map if needed
    Q_ASSERT(_adapter_map);
    if (_adapter_map) _adapter_map->insert(m_virtual_file, x);

    // determine the mode: rw/w/r
    const char *mode = 0;
    if      (m_device.isWritable()) mode = "w";
    else if (m_device.isReadable()) mode = "r";
    Q_ASSERT(mode);

    AFerrfunc old_handler;
    old_handler = afSetErrorHandler(_handle_audiofile_error);

    // open the virtual file and get a handle for it
    m_file_handle = afOpenVirtualFile(m_virtual_file, mode, setup);
    m_last_error = _lastAudiofileError();

    afSetErrorHandler(old_handler);
}

//***************************************************************************
void VirtualAudioFile::close()
{
    // close libaudiofile stuff
    afCloseFile(m_file_handle);

    // de-register ourself
    if (_adapter_map) _adapter_map->remove(m_virtual_file);

    m_virtual_file = 0;
    m_file_handle = 0;
}

//***************************************************************************
VirtualAudioFile::~VirtualAudioFile()
{
    if (m_virtual_file) close();
}

//***************************************************************************
unsigned int VirtualAudioFile::read(char *data, unsigned int nbytes)
{
    Q_ASSERT(data);
    if (!data) return 0;
    return m_device.readBlock(data, nbytes);
}

//***************************************************************************
long VirtualAudioFile::length()
{
    return m_device.size();
}

//***************************************************************************
unsigned int VirtualAudioFile::write(const char *data, unsigned int nbytes)
{
    Q_ASSERT(data);
    if (!data) return 0;
    return m_device.writeBlock(data, nbytes);
}

//***************************************************************************
void VirtualAudioFile::destroy()
{
}

//***************************************************************************
long VirtualAudioFile::seek(long offset, int is_relative)
{
    if (is_relative == SEEK_CUR)
	m_device.at(m_device.at() + offset);
    else if (is_relative == SEEK_SET)
	m_device.at(offset);
    else
	return -1;
    return 0;
}

//***************************************************************************
long VirtualAudioFile::tell()
{
    return m_device.at();
}

//***************************************************************************
VirtualAudioFile *VirtualAudioFile::adapter(AFvirtualfile *vfile)
{
    // create a new empty map if necessary
    if (!_adapter_map) _adapter_map =
        new QMap<AFvirtualfile*,VirtualAudioFile*>();
    Q_ASSERT(_adapter_map);
    if (!_adapter_map) return 0;

    // lookup in the map
    return _adapter_map->contains(vfile) ? (*_adapter_map)[vfile] : 0;
}

//***************************************************************************
//***************************************************************************
