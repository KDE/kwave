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

#include <string.h> // for some speed-ups like memmove, memcpy ...
#include "mt/MutexGuard.h"
#include "libkwave/Stripe.h"
#include "kwave/MemoryManager.h"

// define this for using only slow Qt array functions
#undef STRICTLY_QT

/** use optimized memcpy() from xine */
#define MEMCPY xine_fast_memcpy

/* forward declaration to libkwave/memcpy.c */
extern "C" void *(* xine_fast_memcpy)(void *to, const void *from, size_t len);

//***************************************************************************
//***************************************************************************
Stripe::MapStorageGuard::MapStorageGuard(Stripe &stripe)
    :m_stripe(stripe), m_storage(0)
{
    m_storage = m_stripe.mapStorage();
}

//***************************************************************************
Stripe::MapStorageGuard::~MapStorageGuard()
{
    m_stripe.unmapStorage();
}

//***************************************************************************
sample_t *Stripe::MapStorageGuard::storage()
{
    return m_storage;
}

//***************************************************************************
//***************************************************************************
Stripe::MappedArray::MappedArray(Stripe &stripe, unsigned int length)
    :m_guard(stripe), m_length(length)
{
    sample_t *samples = m_guard.storage();
    Q_ASSERT(samples);
    Q_ASSERT(m_length);
    if (!samples) m_length = 0;

    if (m_length) setRawData(samples, m_length);
}

//***************************************************************************
Stripe::MappedArray::~MappedArray()
{
    if (m_length) resetRawData(m_guard.storage(), m_length);
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
    sample_t *_samples = m_guard.storage();
    Q_ASSERT(_samples);
    if (!_samples) return 0;
    memmove(&(_samples[dst]), &(_samples[src]), cnt * sizeof(sample_t));
#endif

    return cnt;
}

//***************************************************************************
unsigned int Stripe::MappedArray::copy(unsigned int dst,
    const QMemArray<sample_t> &source, unsigned int offset,
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
    sample_t *_samples = m_guard.storage();
    Q_ASSERT(_samples);
    if (!_samples) return 0;

    MEMCPY(&(_samples[dst]), &(source[offset]),
	   cnt * sizeof(sample_t));
#endif

    return cnt;
}

//***************************************************************************
unsigned int Stripe::MappedArray::read(QMemArray<sample_t> &buffer,
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
    sample_t *_samples = m_guard.storage();
    Q_ASSERT(_samples);
    if (!_samples) return 0;

    MEMCPY(&(buffer[dstoff]), &(_samples[offset]),
	   length * sizeof(sample_t));

#endif
    return length;
}

//***************************************************************************
//***************************************************************************
Stripe::Stripe()
    :QObject(), m_start(0), m_length(0), m_storage(0), m_lock_samples()
{
}

//***************************************************************************
Stripe::Stripe(unsigned int start)
    :QObject(), m_start(start), m_length(0), m_storage(0), m_lock_samples()
{
}

//***************************************************************************
Stripe::Stripe(unsigned int start, const QMemArray<sample_t> &samples)
    :QObject(), m_start(start), m_length(0), m_storage(0), m_lock_samples()
{
    if (samples.size()) append(samples, 0, samples.size());
}

//***************************************************************************
Stripe::Stripe(unsigned int start, Stripe &stripe, unsigned int offset)
    :QObject(), m_start(start), m_length(0), m_storage(0), m_lock_samples()
{
    Q_ASSERT(offset < stripe.length());
    if (offset >= stripe.length()) return;

    unsigned int length = stripe.length() - offset;
    if (resizeStorage(length) != length) return; // out of memory

    if (length) {
	MappedArray _samples(*this, m_length);
	if (!stripe.read(_samples, 0, offset, length)) resize(0);
    }
}

//***************************************************************************
Stripe::~Stripe()
{
    MutexGuard lock(m_lock_samples);
    resizeStorage(0);
}

//***************************************************************************
unsigned int Stripe::start()
{
    MutexGuard lock(m_lock_samples);
    return m_start;
}

//***************************************************************************
void Stripe::setStart(unsigned int start)
{
    MutexGuard lock(m_lock_samples);
    m_start = start;
}

//***************************************************************************
unsigned int Stripe::length()
{
    MutexGuard lock(m_lock_samples);
    return m_length;
}

//***************************************************************************
unsigned int Stripe::end()
{
    MutexGuard lock(m_lock_samples);
    return m_start + ((m_length) ? (m_length - 1) : 0);
}

//***************************************************************************
unsigned int Stripe::resizeStorage(unsigned int length)
{
    if (m_length == length) return length; // nothing to do
//  qDebug("Stripe::resizeStorage(%u)", length);

    MemoryManager &mem = MemoryManager::instance();

    if (!m_length || !m_storage) {
	// allocate new storage
	void *new_storage = mem.allocate(length * sizeof(sample_t));
	if (!new_storage) {
	    // allocation failed
	    qWarning("Stripe::resizeStorage(%u) failed! (1)", length);
	    return m_length;
        }

	m_storage = new_storage;
	m_length  = length;
	return length;
    }

    if (length == 0) {
	// delete the array
	mem.free(m_storage);
	m_storage = 0;
	m_length  = 0;
	return 0;
    }

    // resize the array to another size
    void *new_storage = mem.resize(m_storage, length * sizeof(sample_t));
    Q_ASSERT(new_storage);
    if (!new_storage) {
	// resize failed
	qWarning("Stripe::resizeStorage(%u) failed! (2)", length);
	return m_length;
    }
    m_storage = new_storage;
    m_length  = length;

    return length;
}

//***************************************************************************
unsigned int Stripe::resize(unsigned int length, bool initialize)
{
    unsigned int old_length = 0;
    {
	MutexGuard lock(m_lock_samples);

	old_length = m_length;
	if (m_length == length) return old_length; // nothing to do

//	qDebug("Stripe::resize() from %d to %d samples", old_length, length);
	if (resizeStorage(length) != length) {
	    qWarning("Stripe::resize(%u) failed, out of memory ?", length);
	    return m_length;
	}

	// fill new samples with zero
	if (initialize) {
	    unsigned int pos = old_length;

#ifdef STRICTLY_QT
	    MappedArray _samples(*this, m_length);
	    if (_samples.size() != m_length) return 0;

	    while (pos < length) {
		_samples[pos++] = 0;
	    }
#else
	    MapStorageGuard _map(*this);
	    sample_t *samples = _map.storage();
	    Q_ASSERT(samples);
	    if (!samples) return 0;
	    if (pos < length) {
		memset(&(samples[pos]), 0, (length-pos)*sizeof(sample_t));
	    }
#endif
	}
    }

    if (length < old_length) {
	// something has been deleted from the end
	unsigned int change = old_length - length;
	emit sigSamplesDeleted(*this, length, change);
    } else if (length > old_length) {
	// something has been added to the end
	unsigned int change = length - old_length;
	emit sigSamplesInserted(*this, old_length, change);
    }

    return length;
}

//***************************************************************************
unsigned int Stripe::append(const QMemArray<sample_t> &samples,
	unsigned int offset,
	unsigned int count)
{
    unsigned int old_length;
    unsigned int appended = 0;

    if (!count) return 0; // nothing to do

    {
	MutexGuard lock(m_lock_samples);

	Q_ASSERT(offset + count <= samples.size());
	if (offset + count > samples.size()) return 0;

	qDebug("Stripe::append: adding %d samples", count);

	old_length = m_length;
	unsigned int new_length = old_length + count;
	if (resizeStorage(new_length) != new_length)
	    return 0; // out of memory

	// append to the end of the area
	{
	    MappedArray _samples(*this, m_length);

	    unsigned int cnt = new_length - old_length;
	    appended = _samples.copy(old_length, samples, offset, cnt);
	}
    }

    qDebug("Stripe::append(): resized to %d", m_length);

    // something has been added to the end
    if (appended) emit sigSamplesInserted(*this, old_length, appended);

    return appended;
}

//***************************************************************************
unsigned int Stripe::insert(const QMemArray<sample_t> &samples,
                            unsigned int offset, unsigned int count)
{
    unsigned int old_length;
    unsigned int inserted = 0;

    {
	MutexGuard lock(m_lock_samples);

	if (!count || !m_length) return 0; // nothing to do
	Q_ASSERT(offset + count <= samples.size());
	if (offset + count > samples.size()) return 0;

	qDebug("Stripe::insert: inserting %d samples", count);

	old_length = m_length;
	unsigned int new_length = old_length + count;
	if (resizeStorage(new_length) != new_length) {
	    qWarning("Stripe::insert(): m_length=%u, old=%u, wanted=%u",
		m_length, old_length, new_length);
	    return 0;
	}

	if (offset < old_length) {
	    // move old samples right
	    MappedArray _samples(*this, m_length);

	    unsigned int src = offset;
	    unsigned int dst = offset + count;
	    if (!_samples.copy(dst, src, old_length - offset)) return 0;
	}

	// insert the new samples at the given offset
	MappedArray _samples(*this, m_length);
	inserted = _samples.copy(offset, samples, 0, count);
    }

    qDebug("Stripe::insert(): resized to %d", m_length);

    // something has been inserted
    if (inserted) emit sigSamplesInserted(*this, offset, inserted);

    return inserted;
}

//***************************************************************************
void Stripe::deleteRange(unsigned int offset, unsigned int length)
{
    qDebug("    Stripe::deleteRange(offset=%u, length=%u)", offset, length);
    if (!length) return; // nothing to do

    {
	MutexGuard lock(m_lock_samples);

	const unsigned int size = m_length;
	if (!size) return;

	unsigned int first = offset;
	unsigned int last  = offset + length - 1;
	qDebug("    Stripe::deleteRange, me=[%u ... %u] del=[%u ... %u]",
	       m_start, m_start+size-1, m_start + first, m_start + last);

	Q_ASSERT(first < size);
	if (first >= size) return;

	// put first/last into our area
	if (last >= size) last = size - 1;
	Q_ASSERT(last >= first);
	if (last <= first) return;

	// move all samples after the deleted area to the left
	unsigned int dst = first;
	unsigned int src = last+1;
	unsigned int len = size - src;
	qDebug("    Stripe: deleting %u ... %u", dst, src-1);
	if (len) {
	    MappedArray _samples(*this, m_length);

	    Q_ASSERT(src + len <= size);
	    Q_ASSERT(dst + len <= size);
	    if (!_samples.copy(dst, src, len)) return;
	}

	// resize the buffer to it's new size
	resizeStorage(size - length);
    }

    // sigSamplesDeleted will be emitted in the Track, not here
}

//***************************************************************************
void Stripe::overwrite(unsigned int offset,
	const QMemArray<sample_t> &source,
	unsigned int srcoff, unsigned int srclen)
{
    unsigned int count = srclen;
    {
	MutexGuard lock(m_lock_samples);
	{
	    MappedArray _samples(*this, m_length);
	    count = _samples.copy(offset, source, srcoff, count);
	}
    }

    if (count) emit sigSamplesModified(m_start + offset, count);
}

//***************************************************************************
unsigned int Stripe::read(QMemArray<sample_t> &buffer, unsigned int dstoff,
	unsigned int offset, unsigned int length)
{
    Q_ASSERT(length);
    if (!length) return 0; // nothing to do !?

    MutexGuard lock(m_lock_samples);

//  for (unsigned int x=dstoff; (dstoff+x < length) && (x < buffer.size()); x++)
//      buffer[x] = -(SAMPLE_MAX >> 2);

//     qDebug("Stripe::read(), me=[%u ... %u] (size=%u), offset=%u, length=%u",
//            m_start, m_start+m_length-1, m_length, offset, length);

    Q_ASSERT(offset <= m_length);
    if (offset > m_length) return 0;
    if (offset+length > m_length) length = m_length - offset;
    Q_ASSERT(length);
    if (!length) qDebug("--- [%u ... %u] (%u), offset=%u",
                        m_start, m_start+m_length-1, m_length, offset);
    if (!length) return 0;

    {
	MappedArray _samples(*this, m_length);
	length = _samples.read(buffer, dstoff, offset, length);
    }

//  qDebug("read done, length=%u", length);
    return length;
}

//***************************************************************************
Stripe &Stripe::operator << (const QMemArray<sample_t> &samples)
{
    unsigned int appended = append(samples, 0, samples.size());
    Q_ASSERT(appended == samples.size());
    return *this;
}

//***************************************************************************
sample_t *Stripe::mapStorage()
{
//  qDebug("  Stripe::mapStorage()");
    MemoryManager &mem = MemoryManager::instance();
    return reinterpret_cast<sample_t*>(mem.map(m_storage));
}

//***************************************************************************
void Stripe::unmapStorage()
{
//  qDebug("  Stripe::unmapStorage()");
    MemoryManager &mem = MemoryManager::instance();
    mem.unmap(m_storage);
}

//***************************************************************************
//***************************************************************************
