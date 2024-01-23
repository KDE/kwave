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

#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/Stripe.h"
#include "libkwave/Utils.h"
#include "libkwave/memcpy.h"

/** minimum time between emitting the "progress()" signal [ms] */
#define MIN_PROGRESS_INTERVAL 500

//***************************************************************************
Kwave::SampleReader::SampleReader(Kwave::ReaderMode mode,
                                  Kwave::Stripe::List stripes)
    :m_mode(mode), m_stripes(stripes),
     m_src_position(stripes.left()), m_first(stripes.left()),
     m_last(stripes.right()), m_buffer(blockSize()),
     m_buffer_used(0), m_buffer_position(0),
     m_progress_time(), m_last_seek_pos(stripes.right())
{
    m_progress_time.start();
}

//***************************************************************************
Kwave::SampleReader::~SampleReader()
{
}

//***************************************************************************
void Kwave::SampleReader::reset()
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
    memset((&buffer[offset]), 0x00,
           len * sizeof(const_cast<const Kwave::SampleArray &>(buffer)[0]));
}

//***************************************************************************
void Kwave::SampleReader::fillBuffer()
{
    Q_ASSERT(m_buffer_position >= m_buffer_used);
    m_buffer_used = 0;
    m_buffer_position = 0;
    if (eof()) return;
    if (m_buffer.isEmpty()) return; // we had a OOM before?

    unsigned int rest = m_buffer.size();/* - m_buffer_used (is 0) */
    if (m_src_position + rest > m_last)
        rest = Kwave::toUint(m_last - m_src_position + 1);
    Q_ASSERT(rest <= m_buffer.size());
    if (rest > m_buffer.size()) rest = m_buffer.size();
    Q_ASSERT(rest);

    unsigned int len = readSamples(m_src_position, m_buffer, 0, rest);
    Q_ASSERT(len == rest);
    m_buffer_used  += len;

    // inform others that we proceeded
    if (m_progress_time.elapsed() > MIN_PROGRESS_INTERVAL) {
        m_progress_time.restart();
        emit proceeded();
        QApplication::sendPostedEvents();
    }
}

//***************************************************************************
void Kwave::SampleReader::minMax(sample_index_t first, sample_index_t last,
                                 sample_t &min, sample_t &max)
{
    bool empty = true;
    min = SAMPLE_MAX;
    max = SAMPLE_MIN;

    foreach (Kwave::Stripe s, m_stripes) {
        if (!s.length()) continue;
        sample_index_t start = s.start();
        sample_index_t end   = s.end();

        if (end < first) continue; // not yet in range
        if (start > last)  break;  // done

        // overlap -> not empty
        empty = false;

        // get min/max from the stripe
        unsigned int s1 = Kwave::toUint(
            (first > start) ? (first - start) : 0);
        unsigned int s2 = Kwave::toUint(
            (last < end) ? (last - start) : (end - start));
        s.minMax(s1, s2, min, max);
    }

    // special case: no signal in that range -> set to zero
    if (empty) {
        min = 0;
        max = 0;
    }
}

//***************************************************************************
unsigned int Kwave::SampleReader::read(Kwave::SampleArray &buffer,
                                       unsigned int dstoff,
                                       unsigned int length)
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
        const Kwave::SampleArray &in = m_buffer;
        MEMCPY(&(buffer[dst]), &(in[src]), cnt * sizeof(sample_t));

        if (m_buffer_position >= m_buffer_used) {
            // buffer is empty now
            m_buffer_position = m_buffer_used = 0;
        }

        if (!rest) {
            // inform others that we proceeded
            if (m_progress_time.elapsed() > MIN_PROGRESS_INTERVAL) {
                m_progress_time.restart();
                emit proceeded();
                QApplication::sendPostedEvents();
            }
            return count; // done
        }
    }

    // take the rest directly out of the stripe(s)
    if (m_src_position + rest > (m_last + 1)) // clip to end of reader range
        rest = Kwave::toUint((m_last + 1) - m_src_position);
    if (dstoff + rest > buffer.size()) // clip to end of buffer
        rest = buffer.size() - dstoff;
    Q_ASSERT(dstoff + rest <= buffer.size());
    unsigned int len = readSamples(m_src_position, buffer, dstoff, rest);
    Q_ASSERT(len == rest);
    count += len;

    // inform others that we proceeded
    if (m_progress_time.elapsed() > MIN_PROGRESS_INTERVAL) {
        m_progress_time.restart();
        emit proceeded();
        QApplication::sendPostedEvents();
    }
    return count;
}

//***************************************************************************
void Kwave::SampleReader::skip(sample_index_t count)
{
    if (m_buffer_position + count < m_buffer_used) {
        // skip within the buffer
        m_buffer_position += count;
    } else {
        // skip out of the buffer
        count -= m_buffer_used;
        m_src_position += count;
        m_buffer_position = m_buffer_used = 0;
    }

    // if this reader is of "single pass forward only" type: remove all
    // stripes that we have passed -> there is no way back!
    if (m_mode == Kwave::SinglePassForward) {
        while (!m_stripes.isEmpty() &&
               (m_stripes.first().end() < m_src_position))
        {
            m_stripes.removeFirst();
        }
    }
}

//***************************************************************************
void Kwave::SampleReader::seek(sample_index_t pos)
{
    const sample_index_t current_pos = m_src_position +
        m_buffer_position - m_buffer_used;

    if (pos == current_pos) return; // nothing to do

    if (pos < current_pos) {
        // if we are in SinglePassReverse mode, discard all stripes
        // that we already have passed, up to the end
        if (m_mode == Kwave::SinglePassReverse) {
            while (!m_stripes.isEmpty() &&
                (m_stripes.last().start() > m_last_seek_pos))
            {
//              qDebug("SampleReader: removing stripe [%9u ... %9u] (end=%9u)",
//                      m_stripes.last().start(),
//                      m_stripes.last().end(),
//                      m_last_seek_pos);
                m_stripes.removeLast();
            }
        }

        // seek backwards
        const sample_index_t count = current_pos - pos;
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
Kwave::SampleReader &Kwave::SampleReader::operator >> (sample_t &sample)
{
    // get new buffer if end of last buffer reached
    if (m_buffer_position >= m_buffer_used) fillBuffer();
    const Kwave::SampleArray &buffer = m_buffer;
    sample = (m_buffer_position < m_buffer_used) ?
              buffer[m_buffer_position++] : 0;
    return *this;
}

//***************************************************************************
Kwave::SampleReader &Kwave::SampleReader::operator >> (
    Kwave::SampleArray &buffer)
{
    unsigned int size = buffer.size();
    unsigned int count = read(buffer, 0, size);
    if (count != size) {
        bool ok = buffer.resize(count);
        Q_ASSERT(ok); // shrinking should always be possible
        if (!ok) {
            qWarning("Kwave::SampleReader::operator >> - OOM?");
        }
    }
    return *this;
}

//***************************************************************************
void Kwave::SampleReader::goOn()
{
    Kwave::SampleArray buffer(blockSize());
    (*this) >> buffer;
    emit output(buffer);
}

//***************************************************************************
unsigned int Kwave::SampleReader::readSamples(sample_index_t offset,
                                              Kwave::SampleArray &buffer,
                                              unsigned int buf_offset,
                                              unsigned int length)
{
    Q_ASSERT(length);
    if (!length) return 0; // nothing to do !?
    Q_ASSERT(buf_offset + length <= buffer.size());

    unsigned int   rest  = length;
    sample_index_t left  = offset;
    sample_index_t right = offset + length - 1;

    foreach (Kwave::Stripe s, m_stripes) {
        if (!s.length()) continue;
        sample_index_t start = s.start();
        sample_index_t end   = s.end();

        if (left < start) {
            // gap before the stripe -> pad
            sample_index_t pad = Kwave::toUint(start - left);
            if (pad > rest) pad = rest;
            padBuffer(buffer, buf_offset, Kwave::toUint(pad));
            buf_offset += pad;
            rest       -= pad;
            left       += pad;
            if (!rest) break;
        }

        if (start > right) break; // done, we are after the range

        if (left <= end) {
            // some kind of overlap
            Q_ASSERT(left >= start);
            unsigned int ofs = Kwave::toUint(left - start);
            unsigned int len = Kwave::toUint(end - left + 1);
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

    m_src_position += length;

    // if this reader is of "single pass forward only" type: remove all
    // stripes that we have passed -> there is no way back!
    if (m_mode == Kwave::SinglePassForward) {
        while (!m_stripes.isEmpty() &&
               (m_stripes.first().end() < m_src_position))
        {
            m_stripes.removeFirst();
        }
    }

    return length;
}

//***************************************************************************
//***************************************************************************
