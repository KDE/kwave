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

#include "libkwave/memcpy.h"
#include "libkwave/MemoryManager.h"
#include "libkwave/Stripe.h"

// define this for using only slow Qt array functions
#undef STRICTLY_QT

//***************************************************************************
//***************************************************************************
Stripe::MappedArray::MappedArray(Stripe &stripe)
    :m_stripe(stripe), m_storage(0), m_length(stripe.length())
{
    m_storage = m_stripe.mapStorage();
    Q_ASSERT(m_storage);
    Q_ASSERT(m_length);
    if (!m_storage) m_length = 0;

    if (m_length) setRawData(m_storage, m_length);
}

//***************************************************************************
Stripe::MappedArray::~MappedArray()
{
    if (m_length) resetRawData();
    m_stripe.unmapStorage();
}

//***************************************************************************
unsigned int Stripe::MappedArray::copy(unsigned int dst, unsigned int src,
                                       unsigned int cnt)
{
//  qDebug("    Stripe::MappedArray::copy(%u, %u, %u)", dst, src, cnt);
    Q_ASSERT(m_length);
    if (!m_length) return 0;

#ifdef STRICTLY_QT
    unsigned int rest = cnt;
    while (rest--) {
	(*this)[dst++] = (*this)[src++];
    }
#else
    sample_t *_samples = m_storage;
    Q_ASSERT(_samples);
    if (!_samples) return 0;
    // no MEMCPY here !
    memmove(&(_samples[dst]), &(_samples[src]), cnt * sizeof(sample_t));
#endif

    return cnt;
}

//***************************************************************************
unsigned int Stripe::MappedArray::copy(unsigned int dst,
    const Kwave::SampleArray &source, unsigned int offset,
    unsigned int cnt)
{
//  qDebug("    Stripe::MappedArray::copy(%u, ... ,%u, %u)", dst,
//         offset, cnt);

    Q_ASSERT(m_length);
    if (!m_length) return 0;

#ifdef STRICTLY_QT
    unsigned int rest = cnt;
    while (rest--) {
	(*this)[dst++] = source[offset++];
    }
#else
    sample_t *_samples = m_storage;
    Q_ASSERT(_samples);
    if (!_samples) return 0;

    MEMCPY(&(_samples[dst]), &(source[offset]),
	   cnt * sizeof(sample_t));
#endif

    return cnt;
}

//***************************************************************************
unsigned int Stripe::MappedArray::read(Kwave::SampleArray &buffer,
    unsigned int dstoff, unsigned int offset,  unsigned int length)
{
//  qDebug("    Stripe::MappedArray::read(..., %u, %u, %u)", dstoff,
//         offset, length);

    Q_ASSERT(m_length);
    if (!m_length) return 0;

#ifdef STRICTLY_QT
    unsigned int cnt = length;
    while (cnt--) {
	buffer[dstoff++] = (*this)[offset++];
    }
#else
    sample_t *_samples = m_storage;
    Q_ASSERT(_samples);
    if (!_samples) return 0;

    MEMCPY(&(buffer[dstoff]), &(_samples[offset]),
	   length * sizeof(sample_t));

#endif
    return length;
}

//***************************************************************************
//***************************************************************************
Stripe::StripeStorage::StripeStorage()
    :QSharedData(), m_start(0), m_length(0), m_storage(0), m_lock(),
     m_map_count(0), m_mapped_storage(0)
{
}

//***************************************************************************
Stripe::StripeStorage::StripeStorage(const StripeStorage &other)
    :QSharedData(other), m_start(other.m_start), m_length(other.m_length),
     m_storage(0), m_lock(), m_map_count(0), m_mapped_storage(0)
{
    qDebug("StripeStorage(%p) - DEEP COPY %u,%u from %p",
	static_cast<void *>(this),
	m_start, m_length,
	static_cast<const void *>(&other)
    );

    Q_ASSERT(other.m_storage);
    if (other.m_storage) {
	// allocate memory for the copy
	MemoryManager &mem = MemoryManager::instance();
	m_storage = mem.allocate(m_length * sizeof(sample_t));
	if (!m_storage) {
	    // allocation failed
	    qWarning("StripeStorage: DEEP COPY (%u) FAILED!", m_length);
	}

	// copy the data from the original
	void *original = mem.map(other.m_storage);
	mem.writeTo(m_storage, 0, original, m_length * sizeof(sample_t));
	mem.unmap(original);
    }
}

//***************************************************************************
sample_t *Stripe::StripeStorage::map()
{
    QMutexLocker lock(&m_lock);

    Q_ASSERT(m_storage);
    if (!m_storage) return 0;

    if (!m_map_count) {
	MemoryManager &mem = MemoryManager::instance();
	m_mapped_storage = reinterpret_cast<sample_t*>(
	    mem.map(m_storage));
    }
    m_map_count++;
    return m_mapped_storage;
}

//***************************************************************************
void Stripe::StripeStorage::unmap()
{
    QMutexLocker lock(&m_lock);

    Q_ASSERT(m_map_count);
    if (!m_map_count) return;

    m_map_count--;
    if (!m_map_count) {
	MemoryManager &mem = MemoryManager::instance();
	mem.unmap(m_storage);
	m_mapped_storage = 0;
    }
}

//***************************************************************************
Stripe::StripeStorage::~StripeStorage()
{
    Q_ASSERT(!m_map_count);
    Q_ASSERT(!m_mapped_storage);
    if (m_storage) {
	MemoryManager &mem = MemoryManager::instance();
	mem.free(m_storage);
    }
}

//***************************************************************************
//***************************************************************************
Stripe::Stripe()
    :m_lock(), m_data(new StripeStorage)
{
}

//***************************************************************************
Stripe::Stripe(const Stripe &other)
    :m_lock(), m_data(0)
{
    m_data = other.m_data;
}

//***************************************************************************
Stripe::Stripe(unsigned int start)
    :m_lock(), m_data(new StripeStorage)
{
    if (m_data) m_data->m_start = start;
}

//***************************************************************************
Stripe::Stripe(unsigned int start, const Kwave::SampleArray &samples)
    :m_lock(), m_data(new StripeStorage)
{
    if (m_data) m_data->m_start = start;
    if (samples.size()) append(samples, 0, samples.size());
}

//***************************************************************************
Stripe::Stripe(unsigned int start, Stripe &stripe, unsigned int offset)
    :m_lock(), m_data(new StripeStorage)
{
    if (!m_data) return;

    m_data->m_start = start;

    Q_ASSERT(offset < stripe.length());
    if (offset >= stripe.length()) return;

    unsigned int length = stripe.length() - offset;
    if (resizeStorage(length) != length) return; // out of memory

    if (length) {
	MappedArray _samples(*this);
	if (!stripe.read(_samples, 0, offset, length)) resize(0);
    }
}

//***************************************************************************
Stripe::~Stripe()
{
    QMutexLocker lock(&m_lock);
}

//***************************************************************************
unsigned int Stripe::start() const
{
    return (m_data) ? m_data->m_start : 0;
}

//***************************************************************************
void Stripe::setStart(unsigned int start)
{
    QMutexLocker lock(&m_lock);
    m_data.detach();
    if (m_data) m_data->m_start = start;
}

//***************************************************************************
unsigned int Stripe::length() const
{
    return (m_data) ? m_data->m_length : 0;
}

//***************************************************************************
unsigned int Stripe::end() const
{
    return (m_data) ? (m_data->m_start +
	((m_data->m_length) ? (m_data->m_length - 1) : 0)) : 0;
}

//***************************************************************************
unsigned int Stripe::resizeStorage(unsigned int length)
{
    if (!m_data) return 0;
    m_data.detach();

    if (m_data->m_length == length) return length; // nothing to do
//     qDebug("Stripe::resizeStorage(%u)", length);

    MemoryManager &mem = MemoryManager::instance();
    Q_ASSERT(!m_data->mapCount());

    if (length == 0) {
	// delete the array
	mem.free(m_data->m_storage);
	m_data->m_storage = 0;
	m_data->m_length  = 0;
	return 0;
    }

    if (!m_data->m_length || !m_data->m_storage) {
	// allocate new storage
	void *new_storage = mem.allocate(length * sizeof(sample_t));
	if (!new_storage) {
	    // allocation failed
	    qWarning("Stripe::resizeStorage(%u) failed! (1)", length);
	    return m_data->m_length;
        }

	m_data->m_storage = new_storage;
	m_data->m_length  = length;
	return length;
    }

    // resize the array to another size
    void *new_storage = mem.resize(m_data->m_storage,
	length * sizeof(sample_t));
    Q_ASSERT(new_storage);
    if (!new_storage) {
	// resize failed
	qWarning("Stripe::resizeStorage(%u) failed! (2)", length);
	return m_data->m_length;
    }
    m_data->m_storage = new_storage;
    m_data->m_length  = length;

    return length;
}

//***************************************************************************
unsigned int Stripe::resize(unsigned int length, bool initialize)
{
    if (!m_data) return 0;
    m_data.detach();

    unsigned int old_length = 0;
    {
	QMutexLocker lock(&m_lock);

	old_length = m_data->m_length;
	if (m_data->m_length == length) return old_length; // nothing to do

// 	qDebug("Stripe::resize() from %d to %d samples", old_length, length);
	Q_ASSERT(!m_data->mapCount());
	if (resizeStorage(length) != length) {
	    qWarning("Stripe::resize(%u) failed, out of memory ?", length);
	    return m_data->m_length;
	}

	// fill new samples with zero
	if (initialize && length) {
	    Q_ASSERT(!m_data->mapCount());
	    unsigned int pos = old_length;

#ifdef STRICTLY_QT
	    MappedArray _samples(*this);
	    if (_samples.size() != m_data->m_length) return 0;

	    while (pos < length) {
		_samples[pos++] = 0;
	    }
#else
	    MappedArray _map(*this);
	    sample_t *samples = _map.data();
	    Q_ASSERT(samples);
	    if (!samples) return 0;
	    if (pos < length) {
		memset(&(samples[pos]), 0, (length-pos)*sizeof(sample_t));
	    }
#endif
	}
    }

    return length;
}

//***************************************************************************
unsigned int Stripe::append(const Kwave::SampleArray &samples,
	unsigned int offset,
	unsigned int count)
{
    unsigned int old_length;
    unsigned int appended = 0;

    if (!count || !m_data) return 0; // nothing to do
    m_data.detach();

    {
	QMutexLocker lock(&m_lock);

	Q_ASSERT(offset + count <= samples.size());
	if (offset + count > samples.size()) return 0;

// 	qDebug("Stripe::append: adding %d samples", count);

	old_length = m_data->m_length;
	unsigned int new_length = old_length + count;
	Q_ASSERT(!m_data->mapCount());
	if (resizeStorage(new_length) != new_length)
	    return 0; // out of memory

	// append to the end of the area
	unsigned int cnt = new_length - old_length;
	Q_ASSERT(!m_data->mapCount());
	appended = MemoryManager::instance().writeTo(m_data->m_storage,
	    old_length * sizeof(sample_t),
	    &(samples[offset]), cnt * sizeof(sample_t))
	    / sizeof(sample_t);
    }
//     qDebug("Stripe::append(): resized to %d", m_length);
    return appended;
}

//***************************************************************************
void Stripe::deleteRange(unsigned int offset, unsigned int length)
{
//     qDebug("    Stripe::deleteRange(offset=%u, length=%u)", offset, length);
    if (!length || !m_data) return; // nothing to do
    m_data.detach();

    {
	QMutexLocker lock(&m_lock);

	const unsigned int size = m_data->m_length;
	if (!size) return;

	unsigned int first = offset;
	unsigned int last  = offset + length - 1;
// 	qDebug("    Stripe::deleteRange, me=[%u ... %u] del=[%u ... %u]",
// 	       m_start, m_start+size-1, m_start + first, m_start + last);

	Q_ASSERT(first < size);
	if (first >= size) return;

	// put first/last into our area
	if (last >= size) last = size - 1;
	Q_ASSERT(last >= first);
	if (last < first) return;

	// move all samples after the deleted area to the left
	unsigned int dst = first;
	unsigned int src = last+1;
	unsigned int len = size - src;
// 	qDebug("    Stripe: deleting %u ... %u", dst, src-1);
	if (len) {
	    MappedArray _samples(*this);

	    Q_ASSERT(src + len <= size);
	    Q_ASSERT(dst + len <= size);
	    if (!_samples.copy(dst, src, len)) return;
	}

	// resize the buffer to it's new size
	resizeStorage(size - length);
    }
}

//***************************************************************************
void Stripe::overwrite(unsigned int offset,
	const Kwave::SampleArray &source,
	unsigned int srcoff, unsigned int srclen)
{
    QMutexLocker lock(&m_lock);
    if (!m_data) return;
    m_data.detach();

    Q_ASSERT(!m_data->mapCount());
    MemoryManager::instance().writeTo(m_data->m_storage,
	offset * sizeof(sample_t),
	&(source[srcoff]), srclen * sizeof(sample_t));
}

//***************************************************************************
unsigned int Stripe::read(Kwave::SampleArray &buffer, unsigned int dstoff,
	unsigned int offset, unsigned int length)
{
    Q_ASSERT(length);
    if (!length || !m_data) return 0; // nothing to do !?

    QMutexLocker lock(&m_lock);

//  for (unsigned int x=dstoff; (dstoff+x < length) && (x < buffer.size()); x++)
//      buffer[x] = -(SAMPLE_MAX >> 2);

//     qDebug("Stripe::read(), me=[%u ... %u] (size=%u), offset=%u, length=%u",
//            m_start, m_start+m_length-1, m_length, offset, length);

    Q_ASSERT(offset < m_data->m_length);
    if (offset >= m_data->m_length) return 0;
    if (offset+length > m_data->m_length)
	length = m_data->m_length - offset;
    Q_ASSERT(length);
//     if (!length) qDebug("--- [%u ... %u] (%u), offset=%u",
//                         m_start, m_start+m_length-1, m_length, offset);
    if (!length) return 0;

    // read directly through the memory manager, fastest path
    Q_ASSERT(!m_data->mapCount());
    length = MemoryManager::instance().readFrom(m_data->m_storage,
        offset * sizeof(sample_t),
        &buffer[dstoff], length * sizeof(sample_t)) / sizeof(sample_t);

//  qDebug("read done, length=%u", length);
    return length;
}

//***************************************************************************
void Stripe::minMax(unsigned int first, unsigned int last,
                    sample_t &min, sample_t &max)
{
    QMutexLocker lock(&m_lock);

    min = 0;
    max = 0;

    MappedArray _samples(*this);
    const sample_t *buffer = _samples.data();
    Q_ASSERT(buffer);
    if (!buffer || !m_data) return;

    // loop over the mapped storage to get min/max
    register sample_t lo = SAMPLE_MAX;
    register sample_t hi = SAMPLE_MIN;
    Q_ASSERT(first < m_data->m_length);
    Q_ASSERT(first <= last);
    Q_ASSERT(last < m_data->m_length);
    buffer += first;
    while (first++ <= last) {
	register sample_t s = *(buffer++);
	if (s < lo) lo = s;
	if (s > hi) hi = s;
    }
    min = lo;
    max = hi;
}

//***************************************************************************
Stripe &Stripe::operator << (const Kwave::SampleArray &samples)
{
    unsigned int appended = append(samples, 0, samples.size());
    Q_ASSERT(appended == samples.size());
    Q_UNUSED(appended);
    return *this;
}

//***************************************************************************
bool Stripe::operator == (const Stripe &other) const
{
    return ((start() == other.start()) &&
            (end()   == other.end()));
}

//***************************************************************************
Stripe &Stripe::operator = (const Stripe &other)
{
    m_data = other.m_data;
    return *this;
}

//***************************************************************************
sample_t *Stripe::mapStorage()
{
    return (m_data) ? m_data->map() : 0;
}

//***************************************************************************
void Stripe::unmapStorage()
{
    if (m_data) m_data->unmap();
}

//***************************************************************************
//***************************************************************************
