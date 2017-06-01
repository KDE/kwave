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

#ifndef REPAIR_VIRTUAL_AUDIO_FILE_H
#define REPAIR_VIRTUAL_AUDIO_FILE_H

#include "config.h"
#include "libkwave/VirtualAudioFile.h"
#include <QList>

class QIODevice;

namespace Kwave
{
    class RecoverySource;

    class RepairVirtualAudioFile: public Kwave::VirtualAudioFile
    {
    public:
	/**
	 * Constructor
	 * @param device QIODevice used as source
	 * @param repair_list list of RecoverySource objects for
	 *                    building the new file
	 */
	RepairVirtualAudioFile(QIODevice &device,
	                       QList<Kwave::RecoverySource *> *repair_list);

	/** Destructor */
	virtual ~RepairVirtualAudioFile();

	/** reads a block of data */
	qint64 read(char *data, unsigned int nbytes) Q_DECL_OVERRIDE;

	/** returns the length of the file */
	qint64 length() Q_DECL_OVERRIDE;

	/** writes a block of data */
	qint64 write(const char *data, unsigned int nbytes) Q_DECL_OVERRIDE;

	/** seek to a file position */
	qint64 seek(qint64 offset, bool is_relative) Q_DECL_OVERRIDE;

	/** returns the file position */
	qint64 tell() Q_DECL_OVERRIDE;

    private:

	/** position within the virtual file */
	qint64 m_position;

	/** list of sources for the recovered files */
	QList<Kwave::RecoverySource *> *m_repair_list;
    };
}

#endif /* REPAIR_VIRTUAL_AUDIO_FILE_H */

//***************************************************************************
//***************************************************************************
