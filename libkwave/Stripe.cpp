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
#include "kwave/KwaveApp.h"
#include "kwave/MemoryManager.h"

// define this for using only slow Qt array functions
#undef STRICTLY_QT

//***************************************************************************
Stripe::Stripe()
    :m_start(0), m_samples(), m_lock_samples()
{
}

//***************************************************************************
Stripe::Stripe(unsigned int start)
    :m_start(start), m_samples(), m_lock_samples()
{
}

//***************************************************************************
Stripe::Stripe(unsigned int start, const QArray<sample_t> &samples)
    :m_start(start), m_samples(), m_lock_samples()
{
    if (samples.size()) append(samples, samples.size());
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
unsigned int Stripe::length()
{
    MutexGuard lock(m_lock_samples);
    return m_samples.size();
}


//***************************************************************************
unsigned int Stripe::resizeStorage(unsigned int length)
{
    if (m_samples.size() == length) return length; // nothing to do

    MemoryManager &mem = KwaveApp::memoryManager();

#ifndef STRICTLY_QT
    if (m_samples.size() == 0) {
	// allocate new storage
	sample_t *newstorage = (sample_t *)mem.allocate(
		length*sizeof(sample_t));
	if (!newstorage) return 0;
	
	m_samples.setRawData(newstorage, length);
	return length;
    }

    if (length == 0) {
	// delete the array
	sample_t *oldstorage = m_samples.data();
	m_samples.resetRawData(oldstorage, m_samples.size());
	mem.free(reinterpret_cast<void*>(oldstorage));
	return 0;
    }

    // resize the array to another size
    sample_t *storage = m_samples.data();
    unsigned int oldlength = m_samples.size();
    m_samples.resetRawData(storage, m_samples.size());

    unsigned int newsize = mem.resize(
	reinterpret_cast<void*>(storage), length*sizeof(sample_t));
    if (newsize < length*sizeof(sample_t)) {
	// resize failed
	m_samples.setRawData(storage, oldlength);
	return oldlength;
    }
    m_samples.setRawData(storage, length);
    return length;

#else
    m_samples.resize(length);
    return m_samples.size();

#endif
}

//***************************************************************************
unsigned int Stripe::resize(unsigned int length)
{
    unsigned int old_length;
    {
	MutexGuard lock(m_lock_samples);
	
	old_length = m_samples.size();
	if (old_length == length) return old_length; // nothing to do

//	debug("Stripe::resize() from %d to %d samples", old_length, length);
	resizeStorage(length);
	ASSERT(length = m_samples.size());
	if (length < m_samples.size()) {
	    warning("Stripe::resize(%u) failed, out of memory ?", length);
	}

	length = m_samples.size();

	// fill new samples with zero
	unsigned int pos = old_length;
#ifdef STRICTLY_QT
	while (pos < length) {
	    m_samples[pos++] = 0;
	}
#else
	if (pos < length) {
	    memset(&(m_samples[pos]), 0, (length-pos)*sizeof(sample_t));
	}
#endif
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
unsigned int Stripe::append(const QArray<sample_t> &samples,
	unsigned int count)
{
    unsigned int old_length;
    unsigned int appended = 0;

    {
	MutexGuard lock(m_lock_samples);

	if (!count || !samples.size()) return 0; // nothing to do
	ASSERT(count <= samples.size());
	if (count > samples.size()) count = samples.size();

//	debug("Stripe::append: adding %d samples", count);

	old_length = m_samples.size();
	unsigned int newlength = old_length + count;
	resizeStorage(newlength);
	ASSERT(m_samples.size() == newlength);
	newlength = m_samples.size();

	// append to the end of the area
	unsigned int pos = old_length;
#ifdef STRICTLY_QT
	while (pos < newlength) {
	    m_samples[pos++] = samples[appended++];
	}
#else
	if (pos < newlength) {
	    appended = newlength - pos;
	    memmove(&(m_samples[pos]), &(samples[0]),
	            appended*sizeof(sample_t));
	}
#endif
    }

    debug("Stripe::append(): resized to %d", m_samples.size());

    // something has been added to the end
    if (appended) emit sigSamplesInserted(*this, old_length, appended);

    return appended;
}

//***************************************************************************
unsigned int Stripe::insert(const QArray<sample_t> &samples,
	unsigned int offset, unsigned int count)
{
    unsigned int old_length;
    unsigned int inserted = 0;

    {
	MutexGuard lock(m_lock_samples);
	
	if (!count || !samples.size()) return 0; // nothing to do
	ASSERT(count <= samples.size());
	if (count > samples.size()) count = samples.size();
	
//	debug("Stripe::insert: inserting %d samples", count);
	
	old_length = m_samples.size();
	unsigned int new_length = old_length + count;
	resizeStorage(new_length);
	ASSERT(m_samples.size() == new_length);
	new_length = m_samples.size();
	
	unsigned int src = old_length;
	unsigned int dst = new_length;
	unsigned int cnt;
	
	if (offset < old_length) {
	    // move old samples right
	    cnt = old_length - offset;
#ifdef STRICTLY_QT
	    while (cnt--) {
		m_samples[--new_length] = m_samples[--old_length];
	    }
#else
	    unsigned int src = old_length - cnt;
	    unsigned int dst = new_length - cnt;
	    memmove(&(m_samples[dst]), &(m_samples[src]),
	            cnt*sizeof(sample_t));
#endif
	}
	
	// insert at the given offset
	src = 0;
	dst = offset;
	cnt = count;
#ifdef STRICTLY_QT
	while (cnt--) {
	    m_samples[dst++] = samples[src++];
	}
#else
	memmove(&(m_samples[dst]), &(samples[src]), cnt * sizeof(sample_t));
#endif
	
	inserted = count;
    }

//    debug("Stripe::insert(): resized to %d", m_samples.size());

    // something has been inserted
    if (inserted) emit sigSamplesInserted(*this, offset, inserted);

    return inserted;
}

//***************************************************************************
void Stripe::deleteRange(unsigned int offset, unsigned int length)
{
    {
	MutexGuard lock(m_lock_samples);
	unsigned int size = m_samples.size();
	
	if (offset >= m_start+size) return;
	if (offset < m_start) {
	    // put offset into our area
	    unsigned int shift = m_start - offset;
	    if (shift >= length) return;
	    length -= shift;
	    offset = m_start;
	}
	if (offset+length > m_start+size) {
	    // limit the size to the end of the samples
	    length = size - m_start;
	}
	if (!length) return;
	
	// move all samples after the deleted area to the left
	unsigned int dst = offset - m_start;
	unsigned int src = dst + length;
	unsigned int len = size - src;
#ifdef STRICTLY_QT
	while (len--) {
	    m_samples[dst++] = m_samples[src++];
	}
#else
	if (len) memmove(&(m_samples[dst]), &(m_samples[src]),
	                 len*sizeof(sample_t));
#endif
	
	// resize the buffer to it's new size
	resizeStorage(size - length);
    }
//
//    if (length) emit sigSamplesDeleted(*this, offset, length);
}

//***************************************************************************
void Stripe::overwrite(unsigned int offset, const QArray<sample_t> &samples,
    	unsigned int srcoff, unsigned int srclen)
{
    unsigned int count = 0;
    {
	MutexGuard lock(m_lock_samples);
	
#ifdef STRICTLY_QT
	unsigned int dst = offset;
	while (srclen--) {
	    m_samples[dst++] = samples[srcoff++];
	    count++;
	}
#else
	unsigned int len = m_samples.size();
	ASSERT(offset+count <= len);
	if (srclen) {
	    count = srclen;
	    memmove(&(m_samples[offset]), &(samples[srcoff]),
	            count * sizeof(sample_t));
	}
#endif
    }

    if (count) emit sigSamplesModified(*this, offset, count);
}

//***************************************************************************
unsigned int Stripe::read(QArray<sample_t> &buffer, unsigned int dstoff,
	unsigned int offset, unsigned int length)
{
    unsigned int count = 0;

    MutexGuard lock(m_lock_samples);
#ifdef STRICTLY_QT
    while (length--) {
	buffer[dstoff++] = m_samples[offset++];
	count++;
    }
#else
    if (length) {
	count = length;
	memmove(&(buffer[dstoff]), &(m_samples[offset]),
	        count * sizeof(sample_t));
    }
#endif

    return count;
}

//***************************************************************************
Stripe &Stripe::operator << (const QArray<sample_t> &samples)
{
    unsigned int appended = append(samples, samples.size());
    ASSERT(appended == samples.size());
    return *this;
}

//***************************************************************************
//***************************************************************************
