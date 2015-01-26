/*************************************************************************
    RecoveryMapping.cpp  - mapping of a recovered range in a audio file
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

#include <config.h>

#include <QtCore/QIODevice>

#include "RecoveryMapping.h"

//***************************************************************************
Kwave::RecoveryMapping::RecoveryMapping(quint64 offset,
                                        quint64 length,
                                        QIODevice &dev,
                                        quint64 dev_offset)
    :Kwave::RecoverySource(offset, length),
     m_dev(dev), m_dev_offset(dev_offset)
{
}

//***************************************************************************
qint64 Kwave::RecoveryMapping::read(quint64 offset, char *data,
                                    unsigned int bytes)
{
    if (offset < this->offset()) return 0;
    if (offset > end()) return 0;

    qint64 off = offset - this->offset();
    qint64 len = length() - off;
    if (bytes < len) len = bytes;
    if (!len) return 0;

    if (!m_dev.seek(m_dev_offset + off)) return 0;
    return m_dev.read(data, len);
}

//***************************************************************************
//***************************************************************************
