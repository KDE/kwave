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
     m_position(left), m_buffer(16*1024), m_buffer_used(0)
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
	    warning("---SampleInputStream::flush(): Insert not implemented yet---");
	    break;
	case Overwrite:
	    // find the first stripe that contains the current position
	    QListIterator<Stripe> it(m_stripes);
	    unsigned int buf_offset = 0;

	    for (; it.current(); ++it) {
		if (!m_buffer_used) break; // nothing to do

		Stripe *s = it.current();
		unsigned int st = s->start();
		unsigned int len = s->length();
		if (!len) continue; // skip zero-length tracks
		
		if (m_position > st+len-1) break; // end of range reached
		
		if ((m_position >= st) && (m_position <= st+len-1)) {
		    unsigned int offset = m_position - st;
		    unsigned int length = len - offset;
		    if (length > m_buffer_used) length = m_buffer_used;
		
		    // copy the portion of our buffer to the target
		    s->overwrite(offset, m_buffer, buf_offset, length);

		    m_buffer_used -= length;
		    buf_offset += length;
		    m_position += length;
		
		    break;
		}
	    }
	    ASSERT(m_buffer_used == 0);
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
