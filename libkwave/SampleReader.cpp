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

#include <string.h> // for some speed-ups like memmove, memcpy ...

#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SampleLock.h"
#include "libkwave/Stripe.h"
#include "libkwave/Track.h"

// define this for using only slow Qt array functions
#undef STRICTLY_QT

//***************************************************************************
SampleReader::SampleReader(Track &track, QList<Stripe> &stripes,
	SampleLock *lock, unsigned int left, unsigned int right)
    :m_track(track), m_stripes(stripes), m_lock(lock),
    m_position(left), m_first(left), m_last(right), m_buffer(16*1024),
    m_buffer_used(0), m_buffer_position(0), m_eof(false)
{
}

//***************************************************************************
SampleReader::~SampleReader()
{
    if (m_lock) delete m_lock;
}

//***************************************************************************
void SampleReader::reset()
{
    m_position = m_first;
    m_buffer_used = 0;
    m_buffer_position = 0;
    m_eof = false;
}

//***************************************************************************
void SampleReader::fillBuffer()
{
    m_buffer_used = 0;
    m_buffer_position = 0;
    if (m_position > m_last) m_eof = true;
    if (m_eof) return;

    QListIterator<Stripe> it(m_stripes);
    unsigned int rest = m_buffer.size();

    for (; it.current(); ++it) {
	Stripe *s = it.current();
	unsigned int st = s->start();
	unsigned int len = s->length();
	if (!len) continue; // skip zero-length sripes
	
	if (m_position >= st+len) continue; // after our range
	
	if (m_position >= st) {
	    unsigned int offset = m_position - st;
	    unsigned int length = rest;
	    if (offset+length > len) length = len - offset;
	
	    // read from the stripe
	    unsigned int cnt = s->read(m_buffer, 0, offset, length);
	    ASSERT(cnt <= rest);
	    m_buffer_used += cnt;
	    m_position += cnt;
	    rest -= cnt;
	    if (!rest) break; // done, nothing left
	}
    }

    // we are also at eof if nothing was read
    if (!m_buffer_used) m_eof = true;
}

//***************************************************************************
unsigned int SampleReader::read(QArray<sample_t> &buffer,
	unsigned int dstoff, unsigned int length)
{
    if (m_eof || !length) return 0; // already done or nothing to do

    // just a sanity check
    ASSERT(dstoff < buffer.size());
    if (dstoff >= buffer.size()) return 0;

    unsigned int count = 0;
    unsigned int rest = length;

    // first try to read from the current buffer
    if (m_buffer_position < m_buffer_used) {
	unsigned int cnt = rest;
	unsigned int src = m_buffer_position;
	unsigned int dst = dstoff;
	
	if (m_buffer_position + cnt > m_buffer_used)
	    cnt = m_buffer_used - m_buffer_position;
	
	m_buffer_position += cnt;
	count = cnt;
	rest -= cnt;
	dstoff += cnt;
#ifdef STRICTLY_QT
	while (cnt--) {
	    buffer[dst++] = m_buffer[src++];
	}
#else
	memmove(&(buffer[dst]), &(m_buffer[src]), cnt*sizeof(sample_t));
#endif
	
	if (m_buffer_position >= m_buffer_used) {
	    // buffer is empty now
	    m_buffer_position = m_buffer_used = 0;
	}
	if (!rest) return count; // done
    }

    // take the rest directly out of the stripe(s)
    QListIterator<Stripe> it(m_stripes);
    for (; rest && it.current(); ++it) {
	Stripe *s = it.current();
	unsigned int st = s->start();
	unsigned int len = s->length();
	if (!len) continue; // skip zero-length stripes
	
	if (m_position >= st+len) break; // end of range reached
	
	if (m_position >= st) {
	    unsigned int offset = m_position - st;
	    unsigned int cnt = rest;
	    if (offset+cnt > len) cnt = len - offset;
	
	    // read from the stripe
	    cnt = s->read(buffer, dstoff, offset, cnt);
	
	    m_position += cnt;
	    dstoff += cnt;
	    rest -= cnt;
	    count += cnt;
	}
    }

    // if something remained, we have reached eof
    if (rest) m_eof = true;
    if (m_position > m_last) m_eof = true;

    return count;
}

//***************************************************************************
void SampleReader::skip(unsigned int count)
{
    if (m_buffer_position+count < m_buffer_used) {
	// skip within the buffer
	m_buffer_position += count;
    } else {
	// skip out of the buffer
	m_buffer_position = m_buffer_used;
	count -= m_buffer_used;
	m_position += count;
	if (m_position > m_last) m_eof = true;
    }
}

//***************************************************************************
unsigned int SampleReader::pos()
{
    return (m_position + m_buffer_position - m_buffer_used);
}

//***************************************************************************
SampleReader &SampleReader::operator >> (sample_t &sample)
{
    // get new buffer if end of last buffer reached
    if (m_buffer_position >= m_buffer_used) fillBuffer();
    sample = (m_buffer_position < m_buffer_used) ?
              m_buffer[m_buffer_position++] : 0;
    return *this;
}

//***************************************************************************
SampleReader &SampleReader::operator >> (QArray<sample_t> &buffer)
{
    unsigned int size = buffer.size();
    unsigned int count = read(buffer, 0, size);
    if (count != size) buffer.resize(count);
    return *this;
}

//***************************************************************************
//***************************************************************************
