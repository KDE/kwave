/***************************************************************************
  SampleInputStream.cpp  -  stream for inserting samples into a track
			     -------------------
    begin                : Feb 11 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "mt/MutexSet.h"

#include "libkwave/InsertMode.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleInputStream.h"
#include "libkwave/Stripe.h"
#include "libkwave/Track.h"

//***************************************************************************
SampleInputStream::SampleInputStream(Track &track, QList<Stripe> &stripes,
	MutexSet &locks, InsertMode mode, unsigned int left,
	unsigned int right)
    :m_track(track), m_stripes(stripes), m_locks()
{
    m_locks.takeOver(locks);
    debug("SampleInputStream::SampleInputStream(track, mode, %d, %d)",left,right);
}

//***************************************************************************
SampleInputStream::~SampleInputStream()
{
}

//***************************************************************************
void SampleInputStream::operator << (const QArray<sample_t> &samples)
{
}

//***************************************************************************
//***************************************************************************
