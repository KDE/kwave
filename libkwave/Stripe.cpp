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

#include "mt/MutexGuard.h"

#include "libkwave/Stripe.h"

//***************************************************************************
Stripe::Stripe()
    :m_lock(), m_samples(), m_lock_samples()
{
}

//***************************************************************************
Stripe::Stripe(unsigned int start)
    :m_lock(), m_start(start), m_samples(), m_lock_samples()
{
}

//***************************************************************************
Stripe::Stripe(unsigned int start, const QArray<sample_t> &samples)
    :m_lock(), m_start(start), m_samples(samples), m_lock_samples()
{
}

//***************************************************************************
Stripe::~Stripe()
{
    MutexGuard lock(m_lock_samples);
    debug("Stripe::~Stripe()");
    m_samples.resize(0);
}

//***************************************************************************
Mutex &Stripe::mutex()
{
    return m_lock;
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
unsigned int Stripe::resize(unsigned int length)
{
    unsigned int old_length;
    {
	MutexGuard lock(m_lock_samples);

	old_length = m_samples.size();
	if (old_length == length) return old_length; // nothing to do

//	debug("Stripe::resize() from %d to %d samples", old_length, length);
	m_samples.resize(length);
	ASSERT(length = m_samples.size());
	if (length < m_samples.size()) {
	    warning("Stripe::resize(%u) failed, out of memory ?", length);
	}

	length = m_samples.size();

	// fill new samples with zero
	unsigned int pos = old_length;
	while (pos < length) {
	    m_samples[pos++] = 0;
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

//	debug("Stripe::append: adding %d samples", samples.count());

	old_length = m_samples.size();
	unsigned int newlength = old_length + count;
	m_samples.resize(newlength);
	ASSERT(m_samples.size() == newlength);
	newlength = m_samples.size();

	// append to the end of the area
	unsigned int pos = old_length;
	while (pos < newlength) {
	    m_samples[pos++] = samples[appended++];
	}
    }

    debug("Stripe::append(): resized to %d", m_samples.size());

    // something has been added to the end
    emit sigSamplesInserted(*this, old_length, appended);

    return appended;
}

//***************************************************************************
void Stripe::overwrite(unsigned int offset, const QArray<sample_t> &samples,
    	unsigned int srcoff, unsigned int srclen)
{
    unsigned int count = 0;
    {
	MutexGuard lock(m_lock_samples);
	unsigned int pos = offset;
	while (srclen--) {
	    m_samples[pos++] = samples[srcoff++];
	    count++;
	}
    }

    emit sigSamplesModified(*this, offset, count);
}

//***************************************************************************
unsigned int Stripe::read(unsigned int offset, QArray<sample_t> &samples,
	unsigned int dstoff, unsigned int dstlen)
{
    unsigned int count = 0;
    {
	MutexGuard lock(m_lock_samples);
	unsigned int pos = offset;
	while (dstlen--) {
	    samples[pos++] = m_samples[dstoff++];
	    count++;
	}
    }
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
