/*************************************************************************
     RecoveryBuffer.cpp  - buffer for reconstructed audio file data
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
#include <string.h>
#include "RecoveryBuffer.h"

//***************************************************************************
RecoveryBuffer::RecoveryBuffer(unsigned int offset, unsigned int length,
                               char *buffer)
    :RecoverySource(offset, length), m_buffer(buffer, length)
{
}

//***************************************************************************
unsigned int RecoveryBuffer::read(unsigned int offset, char *data,
                                  unsigned int bytes)
{
    if (offset < this->offset()) return 0;
    if (offset > end()) return 0;

    unsigned int off = offset - this->offset();
    unsigned int len = length() - off;
    if (bytes < len) len = bytes;
    if (!len) return 0;

    char *src = m_buffer.data();
    src += off;
    memcpy(data, src, len);

    return len;
}

//***************************************************************************
//***************************************************************************
