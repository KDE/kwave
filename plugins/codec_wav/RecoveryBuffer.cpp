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

#include <config.h>
#include <string.h>

#include "libkwave/memcpy.h"
#include "libkwave/Utils.h"

#include "RecoveryBuffer.h"

//***************************************************************************
Kwave::RecoveryBuffer::RecoveryBuffer(quint64 offset,
                                      quint64 length,
                                      char *buffer)
    :Kwave::RecoverySource(offset, length),
     m_buffer(buffer, Kwave::toUint(length))
{
}

//***************************************************************************
qint64 Kwave::RecoveryBuffer::read(quint64 offset, char *data,
                                   unsigned int bytes)
{
    if (offset < this->offset()) return 0;
    if (offset > end()) return 0;

    quint64 off = offset - this->offset();
    qint64 len  = length() - off;
    if (bytes < len) len = bytes;
    if (!len) return 0;

    const char *src = m_buffer.constData();
    src += off;
    MEMCPY(data, src, static_cast<size_t>(len));

    return len;
}

//***************************************************************************
//***************************************************************************
