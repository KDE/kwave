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

#define BUFFER_SIZE (256*1024)

//***************************************************************************
SampleWriter::SampleWriter(Track &track, QPtrList<Stripe> &stripes,
	SampleLock *lock, InsertMode mode, unsigned int left,
	unsigned int right)
    :QObject(), m_first(left), m_last(right), m_mode(mode), m_track(track),
     m_stripes(stripes), m_lock(lock), m_position(left),
     m_buffer(BUFFER_SIZE), m_buffer_used(0)
{
}

//***************************************************************************
SampleWriter::~SampleWriter()
{
    flush();
    Q_ASSERT(m_position <= m_last+1);

    // inform others that we proceeded
    emit sigSamplesWritten(m_position - m_first);
    if (m_lock) delete m_lock;
    m_lock = 0;
}

//***************************************************************************
SampleWriter &SampleWriter::operator << (const QMemArray<sample_t> &samples)
{
    // first flush the single-sample buffer before doing block operation
    if (m_buffer_used) flush();

    // now flush the block that we received as parameter (pass-through)
    unsigned int count = samples.size();
    flush(samples, count);
    Q_ASSERT(!count);

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
	if (m_position+buflen-1 > m_last) buflen = (m_last-m_position)+1;

	m_buffer_used = reader.read(m_buffer, 0, buflen);
	Q_ASSERT(m_buffer_used);
	if (!m_buffer_used) break;

	flush();
    }

    // pad the rest with zeroes
    Q_ASSERT(m_position <= m_last+1);
    while (m_buffer_used + m_position <= m_last) {
	*this << static_cast<sample_t>(0);
	m_position++;
    }
    Q_ASSERT(m_position <= m_last+1);

    return *this;
}

//***************************************************************************
void SampleWriter::flush(const QMemArray<sample_t> &buffer,
                         unsigned int &count)
{
    if (count == 0) return; // nothing to flush

    Q_ASSERT(m_position <= m_last+1);
    switch (m_mode) {
	case Append: {
	    Stripe *s = m_stripes.last();
	    Q_ASSERT(s);
	    if (!s) break;
	    unsigned int cnt = s->append(buffer, count);
	    Q_ASSERT(cnt == count);
	    m_position += count;
	    if (m_position+1 > m_last) m_last = m_position-1;
	    break;
	}
	case Insert: {
	    // in insert mode we only have one stripe and it is clear
	    // where to insert
	    Q_ASSERT(m_stripes.count() == 1);
	    Stripe *s = m_stripes.first();
	    Q_ASSERT(s);
	    if (!s) break;

	    // insert samples after the last insert position
	    unsigned int ofs = s->start();
	    Q_ASSERT(ofs <= m_position);
	    if (ofs > m_position) break;
	    ofs = m_position - ofs;

	    unsigned int cnt = s->insert(buffer, ofs, count);
	    Q_ASSERT(cnt == count);
	    m_position += count;
	    Q_ASSERT(m_position <= m_last+1);
	    break;
	}
	case Overwrite: {
	    // find the first stripe that contains the current position
	    QPtrListIterator<Stripe> it(m_stripes);
	    unsigned int buf_offset = 0;

	    Q_ASSERT(m_position <= m_last+1);
	    for (; it.current(); ++it) {
		if (!count) break; // nothing to do
		if (m_position > m_last) break;

		Stripe *s = it.current();
		unsigned int st = s->start();
		unsigned int len = s->length();
		if (!len) continue; // skip zero-length stripes

		if (m_position >= st+len) continue; // not yet in range

		if (m_position >= st) {
		    unsigned int offset = m_position - st;
		    unsigned int length = len - offset;
		    if (length > count) length = count;
		    if (m_position+length-1 > m_last)
			length = (m_last-m_position)+1;
		    Q_ASSERT(length);

		    // copy the portion of our buffer to the target
		    s->overwrite(offset, buffer, buf_offset, length);

		    count -= length;
		    buf_offset += length;
		    m_position += length;
		    Q_ASSERT(m_position <= m_last+1);
		}
	    }
	    count = 0;
	    break;
	}
    }

    Q_ASSERT(m_position <= m_last+1);
    count = 0;

    // inform others that we proceeded
    emit proceeded();
}

//***************************************************************************
bool SampleWriter::eof()
{
    return (m_mode == Overwrite) ? (m_position > m_last) : false;
}

//***************************************************************************
SampleWriter &flush(SampleWriter &s)
{
    s.flush();
    return s;
}

//***************************************************************************
//***************************************************************************
