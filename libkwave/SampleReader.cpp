/***************************************************************************
       SampleReader.cpp  -  stream for reading samples from a track
			     -------------------
    begin                : Apr 25 2001
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

#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SampleLock.h"
#include "libkwave/Stripe.h"
#include "libkwave/Track.h"

//***************************************************************************
SampleReader::SampleReader(Track &track, QList<Stripe> &stripes,
	SampleLock *lock, unsigned int left, unsigned int right)
    :m_track(track), m_stripes(stripes), m_lock(lock),
    m_position(left), m_last(right)
{
}

//***************************************************************************
SampleReader::~SampleReader()
{
    if (m_lock) delete m_lock;
}

//***************************************************************************
unsigned int SampleReader::read(QArray<sample_t> &buffer,
	unsigned int dstoff, unsigned int length)
{
//    debug("SampleReader::read(buffer,%u,%u)", dstoff, length); // ###
    unsigned int count = 0;
    if (m_position >= m_last) return 0; // already done

    // just a sanity check
    ASSERT(dstoff < buffer.size());
    if (dstoff >= buffer.size()) return 0;

    QListIterator<Stripe> it(m_stripes);

    for (; it.current(); ++it) {
	Stripe *s = it.current();
	unsigned int st = s->start();
	unsigned int len = s->length();
	if (!len) continue; // skip zero-length tracks
	
	if (m_position >= st+len) break; // end of range reached
	
	if (m_position >= st) {
	    unsigned int offset = m_position - st;
	    unsigned int cnt;
	    if (length > (len-offset)) length = len - offset;
	
	    // read from the stripe
//	    debug("SampleReader::read(): s->read(dstoff=%u, offset=%u, length=%u)",
//	        dstoff, offset, length); // ###
	    cnt = s->read(buffer, dstoff, offset, length);
//	    debug("SampleReader::read(): cnt=%u", cnt); // ###
	
	    count += cnt;
	    m_position += cnt;
	}
    }

    return count;
}

//***************************************************************************
//***************************************************************************
