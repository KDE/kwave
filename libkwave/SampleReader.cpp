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

#define BUFFER_SIZE (256*1024)

//***************************************************************************
SampleReader::SampleReader(Track &track, QPtrList<Stripe> &stripes,
	SampleLock *lock, unsigned int left, unsigned int right)
    :m_track(track), m_stripes(stripes), m_lock(lock),
    m_src_position(left), m_first(left), m_last(right), m_buffer(BUFFER_SIZE),
    m_buffer_used(0), m_buffer_position(0)
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
    m_src_position = m_first;
    m_buffer_used = 0;
    m_buffer_position = 0;

    emit proceeded();
}

//***************************************************************************
void SampleReader::fillBuffer()
{
    Q_ASSERT(m_buffer_position >= m_buffer_used);
    m_buffer_used = 0;
    m_buffer_position = 0;
    if (eof()) return;

    QPtrListIterator<Stripe> it(m_stripes);
    unsigned int rest = m_buffer.size();/* - m_buffer_used (is 0) */
    if (m_src_position+rest-1 > m_last) rest = (m_last-m_src_position+1);
    Q_ASSERT(rest);

    for (; rest && it.current(); ++it) {
	Stripe *s = it.current();
	unsigned int st  = s->start();
	unsigned int len = s->length();
	if (!len) continue; // skip zero-length sripes

	if (m_src_position >= st+len) continue; // before our range

	if (m_src_position >= st) {
	    unsigned int offset = m_src_position - st;
	    unsigned int length = rest;
	    if (offset+length > len) length = len - offset;

	    // read from the stripe
	    unsigned int cnt = s->read(m_buffer, m_buffer_used,
	                               offset, length);
	    Q_ASSERT(cnt <= rest);
	    m_buffer_used  += cnt;
	    m_src_position += cnt;
	    rest -= cnt;
	}
    }

    emit proceeded();
}

//***************************************************************************
unsigned int SampleReader::read(QMemArray<sample_t> &buffer,
	unsigned int dstoff, unsigned int length)
{
    if (eof() || !length) return 0; // already done or nothing to do

    // just a sanity check
    Q_ASSERT(dstoff < buffer.size());
    if (dstoff >= buffer.size()) return 0;

    unsigned int count = 0;
    unsigned int rest = length;
    if (dstoff+rest > buffer.size()) rest = buffer.size() - dstoff;
    Q_ASSERT(rest);
    if (!rest) return 0;

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
    QPtrListIterator<Stripe> it(m_stripes);
    if (m_src_position+rest-1 > m_last) rest = (m_last - m_src_position)+1;

    for (; rest && it.current(); ++it) {
	Stripe *s = it.current();
	unsigned int st  = s->start();
	unsigned int len = s->length();
	if (!len) continue; // skip zero-length stripes

	if (m_src_position >= st+len) continue; // not yet in range

	if (m_src_position >= st) {
	    unsigned int offset = m_src_position - st;
	    unsigned int cnt = rest;
	    if (offset+cnt > len) cnt = len - offset;

	    // read from the stripe
	    cnt = s->read(buffer, dstoff, offset, cnt);

	    m_src_position += cnt;
	    dstoff += cnt;
	    rest -= cnt;
	    count += cnt;
	}
    }

    emit proceeded();
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
	count -= m_buffer_used;
	m_src_position += count;
	m_buffer_position = m_buffer_used = 0;
    }
}

//***************************************************************************
void SampleReader::seek(unsigned int pos)
{
    const unsigned int current_pos = m_src_position +
	m_buffer_position - m_buffer_used;

    if (pos == current_pos) return; // nothing to do

    if (pos < current_pos) {
	// seek backwards
	const unsigned int count = current_pos - pos;
	if (count <= m_buffer_position) {
	    // go back within the buffer
	    m_buffer_position -= count;
	} else {
	    // skip out of the buffer
	    m_src_position = pos;
	    m_buffer_position = m_buffer_used = 0;
	}
    } else {
	// seek forward
	skip(pos - current_pos);
    }
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
SampleReader &SampleReader::operator >> (QMemArray<sample_t> &buffer)
{
    unsigned int size = buffer.size();
    unsigned int count = read(buffer, 0, size);
    if (count != size) buffer.resize(count);
    return *this;
}

//***************************************************************************
//***************************************************************************
