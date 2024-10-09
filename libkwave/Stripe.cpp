/***************************************************************************
             Stripe.cpp  -  continuous block of samples
                             -------------------
    begin                : Feb 10 2001
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

#include <string.h> // for some speed-ups like memmove, memcpy ...

#include "libkwave/Stripe.h"
#include "libkwave/Utils.h"
#include "libkwave/memcpy.h"

//***************************************************************************
//***************************************************************************
Kwave::Stripe::Stripe()
    :m_lock(), m_start(0), m_data()
{
}

//***************************************************************************
Kwave::Stripe::Stripe(const Stripe &other)
    :m_lock(), m_start(other.m_start), m_data(other.m_data)
{
}

//***************************************************************************
Kwave::Stripe::Stripe(Stripe &&other)
    :m_lock(), m_start(other.m_start), m_data(other.m_data)
{
    other.m_start = 0;
    other.m_data.resize(0);
}

//***************************************************************************
Kwave::Stripe::Stripe(sample_index_t start)
    :m_lock(), m_start(start), m_data()
{
}

//***************************************************************************
Kwave::Stripe::Stripe(sample_index_t start, const Kwave::SampleArray &samples)
    :m_lock(), m_start(start), m_data(samples)
{
}

//***************************************************************************
Kwave::Stripe::Stripe(sample_index_t start,
                      Kwave::Stripe &stripe,
                      unsigned int offset)
    :m_lock(), m_start(start), m_data()
{
    Q_ASSERT(offset < stripe.length());
    if (offset >= stripe.length()) return;

    unsigned int length = stripe.length() - offset;
    if (!resize(length)) return; // out of memory

    const sample_t *src = stripe.m_data.constData();
    sample_t       *dst = m_data.data();
    unsigned int    len = length * sizeof(sample_t);
    MEMCPY(dst, src + offset, len);
}

//***************************************************************************
Kwave::Stripe::~Stripe()
{
    QMutexLocker lock(&m_lock);
}

//***************************************************************************
Kwave::Stripe & Kwave::Stripe::operator = (Kwave::Stripe &&other) noexcept
{
    if (this != &other) {
        m_start = other.m_start;
        m_data  = other.m_data;
        other.m_start = 0;
        other.m_data.resize(0);
    }
    return *this;
}

//***************************************************************************
sample_index_t Kwave::Stripe::start() const
{
    return m_start;
}

//***************************************************************************
void Kwave::Stripe::setStart(sample_index_t start)
{
    QMutexLocker lock(&m_lock);
    m_start = start;
}

//***************************************************************************
unsigned int Kwave::Stripe::length() const
{
    return m_data.size();
}

//***************************************************************************
sample_index_t Kwave::Stripe::end() const
{
    const sample_index_t size = m_data.size();
    return (size) ? (m_start + size - 1) : 0;
}

//***************************************************************************
unsigned int Kwave::Stripe::resize(unsigned int length)
{
    QMutexLocker lock(&m_lock);

    unsigned int old_length = m_data.size();
    if (old_length == length) return old_length; // nothing to do

    if (!m_data.resize(length)) {
        qWarning("Stripe::resize(%u) failed, out of memory ?", length);
        return m_data.size();
    }

    return length;
}

//***************************************************************************
unsigned int Kwave::Stripe::append(const Kwave::SampleArray &samples,
        unsigned int offset,
        unsigned int count)
{
    if (!count) return 0; // nothing to do
    Q_ASSERT(offset + count <= samples.size());
    if (offset + count > samples.size()) return 0;

    QMutexLocker lock(&m_lock);

    unsigned int old_length = m_data.size();
    unsigned int new_length = old_length + count;
    if (!m_data.resize(new_length))
        return 0; // out of memory

    // append to the end of the area
    unsigned int cnt = new_length - old_length;
    MEMCPY(m_data.data() + old_length,
           samples.constData() + offset,
           cnt * sizeof(sample_t)
    );

    return cnt;
}

//***************************************************************************
void Kwave::Stripe::deleteRange(unsigned int offset, unsigned int length)
{
    if (!length) return; // nothing to do

    QMutexLocker lock(&m_lock);

    const unsigned int size = m_data.size();
    if (!size) return;

    unsigned int first = offset;
    unsigned int last  = offset + length - 1;
    Q_ASSERT(first < size);
    if (first >= size) return;

    // put first/last into our area
    if (last >= size) last = size - 1;
    Q_ASSERT(last >= first);
    if (last < first) return;

    // move all samples after the deleted area to the left
    unsigned int src = last + 1;
    unsigned int len = size - src;
    if (len) {
        unsigned int dst = first;
        sample_t    *p   = m_data.data();
        memmove(p + dst, p + src, len * sizeof(sample_t));
    }

    // resize the buffer to it's new size
    m_data.resize(size - length);
}

//***************************************************************************
bool Kwave::Stripe::combine(unsigned int offset, Kwave::Stripe &other)
{
    // resize the storage if necessary
    const unsigned int combined_len = offset + other.length();
    if (!resize(combined_len))
        return false; // resizing failed, maybe OOM ?

    // copy the data from the other stripe
    QMutexLocker lock(&m_lock);
    const sample_t *src = other.m_data.constData();
    sample_t       *dst = this->m_data.data();
    unsigned int    len = other.length() * sizeof(sample_t);
    if (!src || !dst) return false; // src or dst does not exist
    MEMCPY(dst + offset, src, len);

    return true;
}

//***************************************************************************
void Kwave::Stripe::overwrite(unsigned int offset,
        const Kwave::SampleArray &source,
        unsigned int srcoff, unsigned int srclen)
{
    QMutexLocker lock(&m_lock);

    const sample_t *src = source.constData();
    sample_t       *dst = this->m_data.data();
    unsigned int    len = srclen * sizeof(sample_t);
    MEMCPY(dst + offset, src + srcoff, len);
}

//***************************************************************************
unsigned int Kwave::Stripe::read(Kwave::SampleArray &buffer,
                                 unsigned int dstoff,
                                 unsigned int offset,
                                 unsigned int length)
{
    QMutexLocker lock(&m_lock);
    if (!length || m_data.isEmpty()) return 0; // nothing to do !?

    unsigned int current_len = m_data.size();
    Q_ASSERT(offset < current_len);
    if (offset >= current_len) return 0;
    if ((offset + length) > current_len)
        length = current_len - offset;
    Q_ASSERT(length);
    if (!length) return 0;

    // directly memcpy
    const sample_t *src = m_data.constData();
    sample_t       *dst = buffer.data();
    unsigned int    len = length * sizeof(sample_t);
    MEMCPY(dst + dstoff, src + offset, len);

    return length;
}

//***************************************************************************
void Kwave::Stripe::minMax(unsigned int first, unsigned int last,
                           sample_t &min, sample_t &max)
{
    QMutexLocker lock(&m_lock);
    if (m_data.isEmpty()) return;

    const sample_t *buffer = m_data.constData();
    if (!buffer) return;

    // loop over the storage to get min/max
    sample_t lo = min;
    sample_t hi = max;
    Q_ASSERT(first < m_data.size());
    Q_ASSERT(first <= last);
    Q_ASSERT(last < m_data.size());
    buffer += first;
    unsigned int remaining = last - first + 1;

    // speedup: process a block of 8 samples at once, to allow loop unrolling
    const unsigned int block = 8;
    while (Q_LIKELY(remaining >= block)) {
        for (unsigned int count = 0; Q_LIKELY(count < block); count++) {
            sample_t s = *(buffer++);
            if (Q_UNLIKELY(s < lo)) lo = s;
            if (Q_UNLIKELY(s > hi)) hi = s;
        }
        remaining -= block;
    }
    while (Q_LIKELY(remaining)) {
        sample_t s = *(buffer++);
        if (Q_UNLIKELY(s < lo)) lo = s;
        if (Q_UNLIKELY(s > hi)) hi = s;
        remaining--;
    }
    min = lo;
    max = hi;
}

//***************************************************************************
Kwave::Stripe &Kwave::Stripe::operator << (const Kwave::SampleArray &samples)
{
    unsigned int appended = append(samples, 0, samples.size());
    if (appended != samples.size()) {
        qWarning("Stripe::operator << FAILED");
    }
    return *this;
}

//***************************************************************************
bool Kwave::Stripe::operator == (const Kwave::Stripe &other) const
{
    return ((start() == other.start()) &&
            (end()   == other.end()));
}

//***************************************************************************
Kwave::Stripe &Kwave::Stripe::operator = (const Kwave::Stripe &other)
{
    m_data = other.m_data;
    return *this;
}

//***************************************************************************
//***************************************************************************
