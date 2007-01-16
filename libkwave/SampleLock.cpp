/***************************************************************************
 libkwave/SampleLock.cpp  -  Lock object for a range of samples
                             -------------------
    begin                : Fri Apr 13 2001
    copyright            : (C) 2000 by Thomas Eschenbacher
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

#include "libkwave/SampleLock.h"
#include "libkwave/Track.h"

//***************************************************************************
SampleLock::SampleLock(Track & /* track */, unsigned int offset,
                       unsigned int length, LockMode mode)
    :m_mode(mode), m_offset(offset), m_length(length)
{
    // lock
}

//***************************************************************************
bool SampleLock::conflictsWith(SampleLock &other)
{
    unsigned int x;

    // our start is after the other's end ?
    x = other.length();
    if (m_offset > other.offset()+(x ? x-1 : 0)) return false;

    // our end is before the other's start ?
    x = m_length;
    if (m_offset+(x ? x-1 : 0) < other.offset()) return false;

    // areas overlap, so check access mode
    return !(((other.mode()) & 0x0F) & ((m_mode) >> 8));
}

//***************************************************************************
SampleLock::~SampleLock()
{
    // release
}

//***************************************************************************
//***************************************************************************
