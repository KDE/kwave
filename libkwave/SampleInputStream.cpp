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
    :m_mode(mode), m_track(track), m_stripes(stripes), m_locks(),
     m_position(left), m_buffer(65536), m_buffer_used(0)
{
    m_locks.takeOver(locks);
    debug("SampleInputStream::SampleInputStream(track, mode, %d, %d)",left,right);
}

//***************************************************************************
SampleInputStream::~SampleInputStream()
{
    flush();
    debug("SampleInputStream::~SampleInputStream()");
}

//***************************************************************************
SampleInputStream &SampleInputStream::operator << (
	const QArray<sample_t> &samples)
{
    unsigned int count = samples.size();
    unsigned int i;
    for (i=0; i < count; i++) {
	*this << samples[i];
    }
    return *this;
}

//***************************************************************************
SampleInputStream &SampleInputStream::operator << (const sample_t &sample)
{
    if (m_buffer_used >= m_buffer.size()) flush();
    m_buffer[m_buffer_used++] = sample;
    m_position++;
    if (m_buffer_used >= m_buffer.size()) flush();
    return *this;
}

//***************************************************************************
SampleInputStream &SampleInputStream::flush()
{
    if (m_buffer_used == 0) return *this; // nothing to flush

    switch (m_mode) {
	case Append: {
	    Stripe *s = m_stripes.last();
	    ASSERT(s);
	    if (s) s->append(m_buffer, m_buffer_used);
	    break;
	}
	case Insert:
	    ASSERT("not implemented yet");
	    break;
	case Overwrite:
	    ASSERT("not implemented yet");
	    break;
    }

    m_buffer_used = 0;
    return *this;
}

//***************************************************************************
SampleInputStream &flush(SampleInputStream &s)
{
    return s.flush();
}

//***************************************************************************
//***************************************************************************
