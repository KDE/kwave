/*************************************************************************
     RecoverySource.cpp  - base class for recovered audio file data
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

#include "RecoverySource.h"

//***************************************************************************
Kwave::RecoverySource::RecoverySource(quint64 offset, quint64 length)
    :m_offset(offset), m_length(length)
{
}

//***************************************************************************
quint64 Kwave::RecoverySource::offset() const
{
    return m_offset;
}

//***************************************************************************
quint64 Kwave::RecoverySource::length() const
{
    return m_length;
}

//***************************************************************************
quint64 Kwave::RecoverySource::end() const
{
    return m_offset + ((m_length) ? (m_length - 1) : 0);
}

//***************************************************************************
//***************************************************************************
