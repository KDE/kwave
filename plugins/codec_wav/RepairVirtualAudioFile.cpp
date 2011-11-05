/*************************************************************************
    RepairVirtualAudioFile.cpp  - emulation of a repaired sane audio file
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

#include "config.h"

#include <QFile>
#include <QIODevice>

#include "RecoverySource.h"
#include "RepairVirtualAudioFile.h"

//***************************************************************************
RepairVirtualAudioFile::RepairVirtualAudioFile(QIODevice &device,
    QList<RecoverySource *> *repair_list)
    :VirtualAudioFile(device), m_position(0),
     m_repair_list(repair_list)
{
}

//***************************************************************************
RepairVirtualAudioFile::~RepairVirtualAudioFile()
{
    if (m_repair_list) {
	while (!m_repair_list->isEmpty()) {
	    RecoverySource *src = m_repair_list->takeLast();
	    if (src) delete src;
	}
	delete m_repair_list;
    }
}

//***************************************************************************
unsigned int RepairVirtualAudioFile::read(char *data, unsigned int nbytes)
{
    Q_ASSERT(m_repair_list);
    Q_ASSERT(data);
    if (!m_repair_list) return 0;
    if (!nbytes) return 0;
    if (!data) return 0;

    bzero(data, nbytes);
    size_t read_bytes = 0;
    foreach (RecoverySource *src, *m_repair_list) {
	Q_ASSERT(src);
	if (!src) continue;
	unsigned int len = src->read(m_position, data, nbytes);
	Q_ASSERT(len <= nbytes);
	nbytes     -= len;
	m_position += len;
	data       += len;
	read_bytes += len;

	if (!nbytes) break;
    }

    return read_bytes;
}

//***************************************************************************
qint64 RepairVirtualAudioFile::length()
{
    Q_ASSERT(m_repair_list);
    if (!m_repair_list) return 0;
    RecoverySource *last = m_repair_list->last();
    Q_ASSERT(last);
    if (!last) return 0;

    return static_cast<qint64>(last->offset() + last->length());
}

//***************************************************************************
unsigned int RepairVirtualAudioFile::write(const char */*data*/,
                                           unsigned int /*nbytes*/)
{
    qWarning("RepairVirtualAudioFile::write() is forbidden !");
    return 0;
}

//***************************************************************************
qint64 RepairVirtualAudioFile::seek(qint64 offset, bool is_relative)
{
    if (is_relative)
	m_position += offset;
    else
	m_position = offset;

    return (static_cast<qint64>(m_position) < length()) ? m_position : -1;
}

//***************************************************************************
qint64 RepairVirtualAudioFile::tell()
{
    return static_cast<qint64>(m_position);
}

//***************************************************************************
//***************************************************************************
