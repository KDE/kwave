/***************************************************************************
  SampleWriter.cpp  -  stream for inserting samples into a track
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

#include "libkwave/InsertMode.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SampleWriter.h"
#include "libkwave/SampleLock.h"
#include "libkwave/Stripe.h"
#include "libkwave/Track.h"

//***************************************************************************
SampleWriter::SampleWriter(Track &track, QList<Stripe> &stripes,
	SampleLock *lock, InsertMode mode, unsigned int left,
	unsigned int right)
    :QObject(), m_first(left), m_last(right), m_mode(mode), m_track(track),
     m_stripes(stripes), m_lock(lock), m_position(left), m_buffer(64*1024),
     m_buffer_used(0)
{
}

//***************************************************************************
SampleWriter::~SampleWriter()
{
    flush();
    emit sigSamplesWritten(m_position - m_first);
    if (m_lock) delete m_lock;
}

//***************************************************************************
SampleWriter &SampleWriter::operator << (const QArray<sample_t> &samples)
{
    // first flush the single-sample buffer before doing block operation
    if (m_buffer_used) flush();

    // now flush the block that we received as parameter (pass-through)
    unsigned int count = samples.size();
    flush(samples, count);
    ASSERT(!count);

    return *this;
}

//***************************************************************************
SampleWriter &SampleWriter::operator << (const sample_t &sample)
{
    m_buffer[m_buffer_used++] = sample;
    if (m_buffer_used >= m_buffer.size()) flush();
    return *this;
}

//***************************************************************************
SampleWriter &SampleWriter::operator << (SampleReader &reader)
{
    if (m_buffer_used) flush();

    // transfer data, using our internal buffer
    unsigned int buflen = m_buffer.size();
    while (!reader.eof() && (m_position <= m_last)) {
	if (m_position + buflen > m_last+1) buflen = m_last - m_position + 1;
	
	m_buffer_used = reader.read(m_buffer, 0, buflen);
	flush();
    }

    // pad the rest with zeroes
    while (m_buffer_used + m_position++ <= m_last) {
	*this << static_cast<sample_t>(0);
    }

    return *this;
}

//***************************************************************************
void SampleWriter::flush(const QArray<sample_t> &buffer, unsigned int &count)
{
    if (count == 0) return; // nothing to flush

    switch (m_mode) {
	case Append: {
	    Stripe *s = m_stripes.last();
	    ASSERT(s);
	    if (s) s->append(buffer, count);
	    m_position += count;
	    break;
	}
	case Insert: {
	    // in insert mode we only have one stripe and it is clear
	    // where to insert
	    ASSERT(m_stripes.count() == 1);
	    Stripe *s = m_stripes.first();
	    ASSERT(s);
	    if (!s) break;
	
	    // insert samples after the last insert position
	    unsigned int ofs = s->start();
	    ASSERT(ofs <= m_position);
	    if (ofs > m_position) break;
	    ofs = m_position - ofs;
	
	    s->insert(buffer, ofs, count);
	    m_position += count;
	    break;
	}
	case Overwrite: {
	    // find the first stripe that contains the current position
	    QListIterator<Stripe> it(m_stripes);
	    unsigned int buf_offset = 0;

	    for (; it.current(); ++it) {
		if (!count) break; // nothing to do
		
		Stripe *s = it.current();
		unsigned int st = s->start();
		unsigned int len = s->length();
		if (!len) continue; // skip zero-length stripes
		
		if (m_position >= st+len) continue; // not yet in range
		
		if (m_position >= st) {
		    unsigned int offset = m_position - st;
		    unsigned int length = len - offset;
		    if (length > count) length = count;
		    if (m_position+length > m_last+1)
			length = m_last-m_position+1;
		    ASSERT(length);
		
		    // copy the portion of our buffer to the target
		    s->overwrite(offset, buffer, buf_offset, length);
		
		    count = 0;
		    buf_offset += length;
		    m_position += length;
		
		    break;
		}
	    }
	    ASSERT(count == 0);
	    break;
	}
    }

    count = 0;
}

//***************************************************************************
SampleWriter &flush(SampleWriter &s)
{
    s.flush();
    return s;
}

//***************************************************************************
//***************************************************************************
