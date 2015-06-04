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
Kwave::RepairVirtualAudioFile::RepairVirtualAudioFile(QIODevice &device,
    QList<Kwave::RecoverySource *> *repair_list)
    :Kwave::VirtualAudioFile(device), m_position(0),
     m_repair_list(repair_list)
{
}

//***************************************************************************
Kwave::RepairVirtualAudioFile::~RepairVirtualAudioFile()
{
    if (m_repair_list) {
	while (!m_repair_list->isEmpty()) {
	    Kwave::RecoverySource *src = m_repair_list->takeLast();
	    if (src) delete src;
	}
	delete m_repair_list;
    }
}

//***************************************************************************
qint64 Kwave::RepairVirtualAudioFile::read(char *data, unsigned int nbytes)
{
    Q_ASSERT(m_repair_list);
    Q_ASSERT(data);
    if (!m_repair_list) return 0;
    if (!nbytes) return 0;
    if (!data) return 0;

    bzero(data, nbytes);
    qint64 read_bytes = 0;
    foreach (Kwave::RecoverySource *src, *m_repair_list) {
	Q_ASSERT(src);
	if (!src) continue;
	qint64 len = src->read(m_position, data, nbytes);
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
qint64 Kwave::RepairVirtualAudioFile::length()
{
    Q_ASSERT(m_repair_list);
    if (!m_repair_list) return 0;
    Kwave::RecoverySource *last = m_repair_list->last();
    Q_ASSERT(last);
    if (!last) return 0;

    return static_cast<qint64>(last->offset() + last->length());
}

//***************************************************************************
qint64 Kwave::RepairVirtualAudioFile::write(const char *data,
                                            unsigned int nbytes)
{
    Q_UNUSED(data);
    Q_UNUSED(nbytes);
    qWarning("RepairVirtualAudioFile::write() is forbidden !");
    return 0;
}

//***************************************************************************
qint64 Kwave::RepairVirtualAudioFile::seek(qint64 offset, bool is_relative)
{
    if (is_relative)
	m_position += offset;
    else
	m_position = offset;

    return (m_position < length()) ? m_position : -1;
}

//***************************************************************************
qint64 Kwave::RepairVirtualAudioFile::tell()
{
    return m_position;
}

//***************************************************************************
//***************************************************************************
