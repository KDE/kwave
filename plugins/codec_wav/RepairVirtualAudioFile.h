/*************************************************************************
    RepairVirtualAudioFile.h  - emulation of a repaired sane audio file
                             -------------------
    begin                : Sun May 12 2002
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

#ifndef _REPAIR_VIRTUAL_AUDIO_FILE_H_
#define _REPAIR_VIRTUAL_AUDIO_FILE_H_

#include "config.h"
#include <qptrlist.h>
#include "libkwave/VirtualAudioFile.h"

class RecoverySource;
class QIODevice;

class RepairVirtualAudioFile: public VirtualAudioFile
{
public:
    /**
     * Constructor
     * @param device QIODevice used as source
     * @param repair_list list of RecoverySource objects for
     *                    building the new file
     */
    RepairVirtualAudioFile(QIODevice &device,
                           QPtrList<RecoverySource> *repair_list);

    /** Destructor */
    virtual ~RepairVirtualAudioFile();

    /** reads a block of data */
    virtual unsigned int read(char *data, unsigned int nbytes);

    /** returns the length of the file */
    virtual long length();

    /** writes a block of data */
    virtual unsigned int write(const char *data, unsigned int nbytes);

    /** seek to a file position */
    virtual long seek(long offset, int is_relative);

    /** returns the file position */
    virtual long tell();

private:

    /** position within the virtual file */
    unsigned int m_position;

    /** list of sources for the recovered files */
    QPtrList<RecoverySource> *m_repair_list;
};

#endif /* _REPAIR_VIRTUAL_AUDIO_FILE_H_ */
