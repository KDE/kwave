/***************************************************************************
              Track.cpp  -  collects one or more stripes in one track
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

#include "mt/Mutex.h"
#include "mt/MutexGuard.h"
#include "mt/MutexSet.h"

#include "libkwave/SampleInputStream.h"
#include "libkwave/Stripe.h"
#include "libkwave/Track.h"

//***************************************************************************
Track::Track()
    :m_stripes(), m_lock_stripes()
{
}

//***************************************************************************
Track::Track(unsigned int length)
    :m_stripes(), m_lock_stripes()
{
    appendStripe(length);
}

//***************************************************************************
Track::~Track()
{
    MutexGuard lock(m_lock_stripes);

    debug("Track::~Track()");
    m_stripes.setAutoDelete(true);
    while (m_stripes.count()) {
	m_stripes.remove(m_stripes.last());
    }
}

//***************************************************************************
Stripe *Track::appendStripe(unsigned int length)
{
    MutexGuard lock(m_lock_stripes);

    unsigned int last = unlockedLength();
    Stripe *s = new Stripe(last, 0);
    ASSERT(s);
    if (s) {
	m_stripes.append(s);
	connect(s, SIGNAL(sigSamplesDeleted(Stripe&, unsigned int,
	    unsigned int)),
	    this, SLOT(slotSamplesDeleted(Stripe&, unsigned int,
	    unsigned int)));
	connect(s, SIGNAL(sigSamplesInserted(Stripe&, unsigned int,
	    unsigned int)),
	    this, SLOT( slotSamplesInserted(Stripe&, unsigned int,
	    unsigned int)));
	connect(s, SIGNAL(sigSamplesModified(Stripe&, unsigned int,
	    unsigned int)),
	    this, SLOT(slotSamplesModified(Stripe&, unsigned int,
	    unsigned int)));

	s->resize(length);
    }
    debug("Track::appendStripe(%d): new stripe at %p", length, s);

    return s;
}

//***************************************************************************
unsigned int Track::length()
{
    MutexGuard lock(m_lock_stripes);
    return unlockedLength();
}

//***************************************************************************
unsigned int Track::unlockedLength()
{
    unsigned int len = 0;
    Stripe *s = m_stripes.last();
    if (s) len = s->start() + s->length();
    return len;
}

//***************************************************************************
SampleInputStream *Track::openInputStream(InsertMode mode,
	unsigned int left, unsigned int right)
{
    MutexGuard lock(m_lock_stripes);
    MutexSet stripe_locks;
    QList<Stripe> stripes;

    switch (mode) {
	case Append: {
	    debug("Track::openInputStream(apppend)");
	
	    // create a new stripe
	    Stripe *s = new Stripe(unlockedLength(), 0);
	    ASSERT(s);
	    if (!s) return 0;
	
	    // add to our stripes list and lock it
	    m_stripes.append(s);
	    stripe_locks.addLock(s->mutex());

	    // use new created stripe as start
            stripes.append(s);
	    break;
	}
	case Insert:
	    debug("Track::openInputStream(insert, %u)", left);
	    warning("--- NOT IMPLEMENTED YET ---"); // ###
	    break;
	case Overwrite:
	    debug("Track::openInputStream(overwrite, %u, %u)", left, right);
	    if ((right == 0) || (right == left)) right = unlockedLength()-1;
	
	    // add all stripes within the specified range to the list
	    // and lock them
	    QListIterator<Stripe> it(m_stripes);
	    for (; it.current(); ++it) {
		Stripe *s = it.current();
		unsigned int st = s->start();
		unsigned int len = s->length();
		if (!len) continue; // skip zero-length tracks

		if (st > right) break; // ok, end reached
		if (st+len-1 >= left) {
		    // overlaps -> include to our list
		    stripe_locks.addLock(s->mutex());
		    stripes.append(s);
		}
	    }
	    break;
    }

    return new SampleInputStream(*this, stripes,
	stripe_locks, mode, left, right);
}

//***************************************************************************
void Track::slotSamplesInserted(Stripe &src, unsigned int offset,
                                unsigned int length)
{
    emit sigSamplesInserted(*this, src.start()+offset, length);
}

//***************************************************************************
void Track::slotSamplesDeleted(Stripe &src, unsigned int offset,
                               unsigned int length)
{
    emit sigSamplesDeleted(*this, src.start()+offset, length);
}

//***************************************************************************
void Track::slotSamplesModified(Stripe &src, unsigned int offset,
                                unsigned int length)
{
    emit sigSamplesModified(*this, src.start()+offset, length);
}

//***************************************************************************
//***************************************************************************
