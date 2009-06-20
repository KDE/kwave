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

#include "config.h"

#include <QApplication>

#include "libkwave/memcpy.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/Stripe.h"
#include "libkwave/Track.h"

// define this for using only slow Qt array functions
// #define STRICTLY_QT

/** minimum time between emitting the "progress()" signal [ms] */
#define MIN_PROGRESS_INTERVAL 500

//***************************************************************************
SampleReader::SampleReader(Kwave::ReaderMode mode, QList<Stripe> stripes,
                           unsigned int left, unsigned int right)
    :m_mode(mode), m_stripes(stripes),
     m_src_position(left), m_first(left), m_last(right),
     m_buffer(blockSize()),
     m_buffer_used(0), m_buffer_position(0),
     m_progress_time(), m_last_seek_pos(right)
{
    m_progress_time.start();
}

//***************************************************************************
SampleReader::~SampleReader()
{
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
static inline void padBuffer(Kwave::SampleArray &buffer,
                             unsigned int offset, unsigned int len)
{
#ifdef STRICTLY_QT
    while (len--)
	buffer[offset++] = 0;
#else
    memset((&buffer[offset]), 0x00, len * sizeof(buffer[0]));
#endif
}

//***************************************************************************
void SampleReader::fillBuffer()
{
    Q_ASSERT(m_buffer_position >= m_buffer_used);
    m_buffer_used = 0;
    m_buffer_position = 0;
    if (eof()) return;

    unsigned int rest = m_buffer.size();/* - m_buffer_used (is 0) */
    if (m_src_position + rest > m_last) rest = (m_last - m_src_position + 1);
    Q_ASSERT(rest <= m_buffer.size());
    if (rest > m_buffer.size()) rest = m_buffer.size();
    Q_ASSERT(rest);

    unsigned int len = readSamples(m_src_position, m_buffer, 0, rest);
    Q_ASSERT(len == rest);
    m_buffer_used  += len;
    m_src_position += len;
    rest           -= len;

    // inform others that we proceeded
    if (m_progress_time.elapsed() > MIN_PROGRESS_INTERVAL) {
	m_progress_time.restart();
	emit proceeded();
	QApplication::sendPostedEvents();
    }
}

//***************************************************************************
void SampleReader::minMax(unsigned int first, unsigned int last,
                          sample_t &min, sample_t &max)
{
    bool empty = true;

    foreach (Stripe s, m_stripes) {
	if (!s.length()) continue;
	unsigned int start = s.start();
	unsigned int end   = s.end();

	if (end < first) continue; // not yet in range
	if (start > last)  break;  // done

	// overlap -> not empty
	empty = false;

	// get min/max from the stripe
	unsigned int s1 = (first > start) ? (first - start) : 0;
	unsigned int s2 = (last < end) ? (last - start) : (end - start);
	s.minMax(s1, s2, min, max);
    }

    // special case: no signal in that range -> set to zero
    if (empty) {
	min = 0;
	max = 0;
    }
}

//***************************************************************************
unsigned int SampleReader::read(Kwave::SampleArray &buffer,
                                unsigned int dstoff, unsigned int length)
{
    if (eof() || !length) return 0; // already done or nothing to do

    // just a sanity check
    Q_ASSERT(buffer.size());
    Q_ASSERT(dstoff < buffer.size());
    if (dstoff >= buffer.size()) return 0;

    unsigned int count = 0;
    unsigned int rest = length;
    if (dstoff + rest > buffer.size()) rest = buffer.size() - dstoff;
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
	count  += cnt;
	rest   -= cnt;
	dstoff += cnt;
	qDebug("filling from buffer dstoff=%u, cnt=%u",dstoff,cnt);
#ifdef STRICTLY_QT
	while (cnt--) {
	    buffer[dst++] = m_buffer[src++];
	}
#else
	MEMCPY(&(buffer[dst]), &(m_buffer[src]), cnt * sizeof(sample_t));
#endif

	if (m_buffer_position >= m_buffer_used) {
	    // buffer is empty now
	    m_buffer_position = m_buffer_used = 0;
	}
	if (!rest) return count; // done
    }

    // take the rest directly out of the stripe(s)
    if (m_src_position + rest > (m_last + 1)) // clip to end of reader range
	rest = (m_last + 1) - m_src_position;
    if (dstoff + rest > buffer.size()) // clip to end of buffer
	rest = buffer.size() - dstoff;
    Q_ASSERT(dstoff + rest <= buffer.size());
    unsigned int len = readSamples(m_src_position, buffer, dstoff, rest);
    Q_ASSERT(len == rest);
    m_src_position += len;
    count += len;

    // inform others that we proceeded
    if (m_progress_time.elapsed() > MIN_PROGRESS_INTERVAL) {
	m_progress_time.restart();
	emit proceeded();
    }
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
	// if we are in SinglePassReverse mode, discard all stripes
	// that we already have passed, up to the end
	if (m_mode == Kwave::SinglePassReverse) {
	    while (!m_stripes.isEmpty() &&
		(m_stripes.last().start() > m_last_seek_pos))
	    {
// 		qDebug("SampleReader: removing stripe [%9u ... %9u] (end=%9u)",
// 			m_stripes.last().start(),
// 			m_stripes.last().end(),
// 			m_last_seek_pos);
		m_stripes.removeLast();
	    }
	}

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

    m_last_seek_pos = m_src_position;
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
SampleReader &SampleReader::operator >> (Kwave::SampleArray &buffer)
{
    unsigned int size = buffer.size();
    unsigned int count = read(buffer, 0, size);
    if (count != size) buffer.resize(count);
    return *this;
}

//***************************************************************************
void SampleReader::goOn()
{
    Kwave::SampleArray buffer(blockSize());
    read(buffer, 0, blockSize());
    emit output(buffer);
}

//***************************************************************************
unsigned int SampleReader::readSamples(unsigned int offset,
                                       Kwave::SampleArray &buffer,
                                       unsigned int buf_offset,
                                       unsigned int length)
{
    Q_ASSERT(length);
    if (!length) return 0; // nothing to do !?
    Q_ASSERT(buf_offset + length <= buffer.size());

    unsigned int rest  = length;
    unsigned int left  = offset;
    unsigned int right = offset + length - 1;

    foreach (Stripe s, m_stripes) {
	if (!s.length()) continue;
	unsigned int start = s.start();
	unsigned int end   = s.end();

	if (left < start) {
	    // gap before the stripe -> pad
	    unsigned int pad = start - left;
	    if (pad > rest) pad = rest;
	    padBuffer(buffer, buf_offset, pad);
	    buf_offset += pad;
	    rest       -= pad;
	    left       += pad;
	    if (!rest) break;
	}

	if (start > right) break; // done, we are after the range

	if (left <= end) {
	    // some kind of overlap
	    Q_ASSERT(left >= start);
	    unsigned int ofs = left - start;
	    unsigned int len = end - left + 1;
	    if (len > rest) len = rest;
	    unsigned int count = s.read(buffer, buf_offset, ofs, len);
	    Q_ASSERT(count == len);
	    buf_offset += count;
	    rest       -= count;
	    left       += count;
	    if (!rest) break;
	}
    }

    // pad at the end
    if (rest) padBuffer(buffer, buf_offset, rest);

    // if this reader is of "single pass forward only" type: remove all
    // stripes that we have passed -> there is no way back!
    if (m_mode == Kwave::SinglePassForward) {
	while (!m_stripes.isEmpty() && (m_stripes.first().end() < m_first)) {
// 	    qDebug("SampleReader: removing stripe [%9u ... %9u] (first=%9u)",
// 		    m_stripes.first().start(),
// 		    m_stripes.first().end(),
// 		    m_first);
	    m_stripes.removeFirst();
	}
    }

    return length;
}

//***************************************************************************
#include "SampleReader.moc"
//***************************************************************************
//***************************************************************************
