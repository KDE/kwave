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
Stripe::Stripe(unsigned int start, unsigned int length)
    :m_lock(), m_start(start), m_samples(), m_lock_samples()
{
    m_samples.resize(length);
    ASSERT(m_samples.size() == length);
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
    MutexGuard lock(m_lock_samples);

    unsigned int old_size = m_samples.size();
    if (old_size == length) return old_size; // nothing to do

    debug("resizing stripe from %d to %d samples", old_size, length);
    m_samples.resize(length);
    ASSERT(length = m_samples.size());
    if (length < m_samples.size()) {
	warning("resize failed, out of memory ?");
    }

    length = m_samples.size();

    // fill new samples with zero
    while (old_size < length) {
	m_samples[old_size++] = 0;
    }

    return length;
}

//***************************************************************************
unsigned int Stripe::append(const QArray<sample_t> &samples,
	unsigned int count)
{
    if (!count || !samples.size()) return 0; // nothing to do
    ASSERT(count <= samples.size());
    if (count > samples.size()) count = samples.size();

    MutexGuard lock(m_lock_samples);
//    debug("Stripe::append: adding %d samples", samples.count());

    unsigned int old_size = m_samples.size();
    unsigned int newlength = old_size + count;
    m_samples.resize(newlength);
    ASSERT(m_samples.size() == newlength);
    newlength = m_samples.size();

    // append to the end of the area
    unsigned int appended = 0;
    while (old_size < newlength) {
	m_samples[old_size++] = samples[appended++];
    }

    debug("Stripe::append(): resized to %d", m_samples.size());
    return appended;
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
