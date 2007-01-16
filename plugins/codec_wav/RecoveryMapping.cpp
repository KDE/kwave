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

#include <qiodevice.h>
#include "RecoveryMapping.h"

//***************************************************************************
RecoveryMapping::RecoveryMapping(unsigned int offset, unsigned int length,
                                 QIODevice &dev, unsigned int dev_offset)
    :RecoverySource(offset, length), m_dev(dev), m_dev_offset(dev_offset)
{
}

//***************************************************************************
unsigned int RecoveryMapping::read(unsigned int offset, char *data,
                                   unsigned int bytes)
{
    if (offset < this->offset()) return 0;
    if (offset > end()) return 0;

    unsigned int off = offset - this->offset();
    unsigned int len = length() - off;
    if (bytes < len) len = bytes;
    if (!len) return 0;

    m_dev.at(m_dev_offset + off);
    m_dev.readBlock(data, len);

    return len;
}

//***************************************************************************
//***************************************************************************
