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

#include "mt/SharedLockGuard.h"

#include "libkwave/SampleReader.h"
#include "libkwave/SampleWriter.h"
#include "libkwave/Stripe.h"
#include "libkwave/Track.h"

/**
 * Optimal size of a stripe [samples].
 * When creating a new stripe, it should have this size
 */
#define STRIPE_LENGTH_OPTIMAL (4UL * 1024UL * 1024UL) /* 16MB */
// #define STRIPE_LENGTH_OPTIMAL (64 * 1024UL) /* 64kB */

/**
 * Maximum stripe size [samples].
 * If a stripe gets bigger as this size, it will be split
 * into two stripes with equal size. Those two new ones
 * always are smaller than the maximum and bigger than
 * the minimum size.
 */
#define STRIPE_LENGTH_MAXIMUM (STRIPE_LENGTH_OPTIMAL * 2)

/**
 * Minimum stripe size [samples]
 * If a stripe is smaller than this size and it is possible to
 * merge it with another neighbour stripe, it will be merged
 * to it. If the result is bigger than STRIPE_LENGTH_MAXIMUM,
 * it will be split again into two parts.
 */
#define STRIPE_LENGTH_MINIMUM (STRIPE_LENGTH_OPTIMAL / 2)

//***************************************************************************
Track::Track()
    :m_lock(), m_stripes(), m_selected(true)
{
    SharedLockGuard lock(m_lock, true);

    m_stripes.setAutoDelete(true);
}

//***************************************************************************
Track::Track(unsigned int length)
    :m_lock(), m_stripes(), m_selected(true)
{
//    SharedLockGuard lock(m_lock, true);

    m_stripes.setAutoDelete(true);
    appendStripe(length);
}

//***************************************************************************
Track::~Track()
{
    SharedLockGuard lock(m_lock, true);

    while (m_stripes.count()) deleteStripe(m_stripes.last());
}

//***************************************************************************
Stripe *Track::appendStripe(unsigned int length)
{
//     SharedLockGuard lock(m_lock, true);
    unsigned int start = unlockedLength();
    unsigned int len;
    Stripe *s = 0;
    qDebug("Track::appendStripe(%u)", length); // ###
    do {
	len = length;
	if (len > STRIPE_LENGTH_MAXIMUM)
	    len = STRIPE_LENGTH_MAXIMUM;

	s = newStripe(start, len);
	if (!s) break;

	length -= len;
	start  += len;
	m_stripes.append(s);
    } while (length);

    return s;
}

//***************************************************************************
void Track::connectStripe(Stripe *s)
{
    Q_ASSERT(s);
    if (!s) return;

    connect(s, SIGNAL(sigSamplesDeleted(Stripe&, unsigned int, unsigned int)),
	this, SLOT(slotSamplesDeleted(Stripe&, unsigned int, unsigned int)));
    connect(s, SIGNAL(sigSamplesInserted(Stripe&, unsigned int, unsigned int)),
	this, SLOT( slotSamplesInserted(Stripe&, unsigned int, unsigned int)));
    connect(s, SIGNAL(sigSamplesModified(unsigned int, unsigned int)),
	this, SLOT(slotSamplesModified(unsigned int, unsigned int)));
}

//***************************************************************************
Stripe *Track::newStripe(unsigned int start, unsigned int length)
{
    Stripe *s = new Stripe(start);
    Q_ASSERT(s);
//  qDebug("Track::newStripe(%u, %u): new stripe at %p", start, length, s);
    if (!s) return 0;

    connectStripe(s);

    if (length) s->resize(length);

    return s;
}

//***************************************************************************
void Track::deleteStripe(Stripe *s)
{
    if (!s) return;

    disconnect(s, SIGNAL(sigSamplesDeleted(Stripe&, unsigned int,
	    unsigned int)),
	    this, SLOT(slotSamplesDeleted(Stripe&, unsigned int,
	    unsigned int)));
    disconnect(s, SIGNAL(sigSamplesInserted(Stripe&, unsigned int,
	    unsigned int)),
	    this, SLOT( slotSamplesInserted(Stripe&, unsigned int,
	    unsigned int)));
    disconnect(s, SIGNAL(sigSamplesModified(unsigned int,
	                                    unsigned int)),
	    this, SLOT(slotSamplesModified(unsigned int,
	                                   unsigned int)));

    m_stripes.setAutoDelete(true);
    m_stripes.remove(s);
}

//***************************************************************************
void Track::splitStripe(Stripe *stripe, unsigned int offset)
{
    Q_ASSERT(stripe);
    if (!stripe) return;
    Q_ASSERT(offset < stripe->length());
    if (offset >= stripe->length()) return;

    // create a new stripe with the data that has been split off
    Stripe *s = new Stripe(stripe->start() + offset, *stripe, offset);
    Q_ASSERT(s);
    if (!s) return;

    // shrink the old stripe
    stripe->resize(offset);

    qDebug("Track::splitStripe(%p, %u): new stripe at [%u ... %u] (%u)",
           stripe, offset, s->start(), s->end(), s->length());

    connectStripe(s);

    qDebug("    inserting at list index %u", m_stripes.findRef(stripe)+1);
    m_stripes.insert(m_stripes.findRef(stripe)+1, s);
}

//***************************************************************************
unsigned int Track::length()
{
    SharedLockGuard lock(m_lock, false);
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
SampleWriter *Track::openSampleWriter(InsertMode mode,
	unsigned int left, unsigned int right)
{
    // create the input stream
    SampleWriter *stream = new SampleWriter(*this, mode, left, right);
    Q_ASSERT(stream);

    return stream;
}

//***************************************************************************
SampleReader *Track::openSampleReader(unsigned int left,
	unsigned int right)
{
    SharedLockGuard lock(m_lock, false);
    QPtrList<Stripe> stripes;

    unsigned int length = unlockedLength();
    if (right >= length) right = length-1;

//     // lock the needed range for shared writing
//     SampleLock *range_lock = new SampleLock(*this, left, right-left+1,
// 	SampleLock::ReadShared);

    // add all stripes within the specified range to the list
    QPtrListIterator<Stripe> it(m_stripes);
    for (; it.current(); ++it) {
	Stripe *s = it.current();
	unsigned int start = s->start();
	unsigned int end   = s->end();

	if (end < left) continue; // too far left
	if (start > right) break; // ok, end reached

	// overlaps -> include to our list
	stripes.append(s);
    }

//     // no lock yet, that's not good...
//     Q_ASSERT(range_lock);
//     if (!range_lock) return 0;

    // create the input stream
    SampleReader *stream = new SampleReader(*this, stripes,
	0 /* range_lock */, left, right);
    Q_ASSERT(stream);
    if (stream) return stream;

//     // stream creation failed, clean up
//     if (range_lock) delete range_lock;
    return 0;
}

//***************************************************************************
void Track::deleteRange(unsigned int offset, unsigned int length,
                        bool make_gap)
{
    if (!length) return;

    {
	SharedLockGuard lock(m_lock, false);

	// lock the needed range for exclusive writing
	SampleLock range_lock(*this, offset, length,
	    SampleLock::WriteExclusive);

	// add all stripes within the specified range to the list
	QPtrListIterator<Stripe> it(m_stripes);
	unsigned int left  = offset;
	unsigned int right = offset + length - 1;

	qDebug("Track::deleteRange() [%u ... %u] (%u)",
	       left, right, right - left + 1);

	for (it.toLast(); it.current(); --it) {
	    Stripe *s = it.current();
	    unsigned int start  = s->start();
	    unsigned int end    = s->end();

	    if (end < left) break; // done, stripe is at left

	    if ((left <= start) && (right >= end)) {
		// total overlap -> delete whole stripe
		qDebug("deleting stripe [%u ... %u]", start, end);
		deleteStripe(s);
		if (m_stripes.isEmpty()) break;
	    } else if (/* (end >= left) && */ (start <= right)) {
		//        ^^^^^^^^^^^^ already checked above
		// partial stripe overlap
		unsigned int ofs = (start < left) ? left : start;
		if (end > right) end = right;
		qDebug("deleting [%u ... %u] (start=%u, ofs-start=%u, len=%u)",
		       ofs, end, start, ofs-start, end - ofs + 1);

		if (!make_gap || (end == s->end())) {
		    // delete within the stripe
		    qDebug("    deleting within the stripe");
		    s->deleteRange(ofs - start, end - ofs + 1);
		    Q_ASSERT(s->length());
		} else {
		    // produce a gap by splitting off a new stripe
		    qDebug("    splitting off to new stripe @ %u (ofs=%u)",
		           right+1, right+1-start);
		    splitStripe(s, right+1-start);

		    // erase to the end (reduce size)
		    qDebug("ofs-start=%u, s->end()-ofs+1=%u [%u...%u] (%u)",
		           ofs-start, s->end()-ofs+1, s->start(),
			   s->end(), s->length());
		    s->deleteRange(ofs-start, s->end()-ofs+1);
		    qDebug("length now: %u [%u ... %u]", s->length(),
		           s->start(), s->end());
		}

		// if deleted from start
		if (ofs == start) {
		    if (make_gap) {
			// make gap -> move right
			qDebug("shifting [%u ... %u] to %u",
				start, s->end(), end+1);
			s->setStart(end+1);
		    } else {
			// no gap -> move left
			qDebug("shifting [%u ... %u] to %u (len=%u)",
				start, s->end(),
				start-length+end-ofs+1,
				length+end-ofs+1);
			s->setStart(start-length+end-ofs+1);
		    }
		}

		Q_ASSERT(s->length());
	    }
	}

	// loop over all remaining stripes and move them left
	// (maybe we start the search one stripe too left,
	// but this doesn't matter, we don't care...)
	if (!make_gap) {
	    if (!it.current()) it.toFirst();

	    for (; it.current(); ++it) {
		Stripe *s = it.current();
// 		qDebug("checking for shift [%u ... %u]",
// 		       s->start(), s->end());
		if (s->start() > right) {
		    // move left
// 		    qDebug("moving stripe %p [%u...%u] %u samples left",
// 		           s, s->start(), s->end(), length);
		    Q_ASSERT(s->start() >= length);
		    s->setStart(s->start() - length);
		}
	    }
	}
    }

    emit sigSamplesDeleted(*this, offset, length);
}

//***************************************************************************
void Track::select(bool selected)
{
    if (m_selected == selected) return;
    m_selected = selected;
    emit sigSelectionChanged();
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
void Track::slotSamplesModified(unsigned int offset,
                                unsigned int length)
{
    emit sigSamplesModified(*this, offset, length);
}

//***************************************************************************
void Track::appendAfter(Stripe *stripe,  unsigned int offset,
                        const QMemArray<sample_t> &buffer,
                        unsigned int buf_offset, unsigned int length)
{
    Q_ASSERT(buf_offset + length <= buffer.size());
    if (buf_offset + length > buffer.size()) return;

    // append to the last stripe if one exists and it's not full
    // and the offset is immediately after the last stripe
    if ((stripe) && (stripe->end()+1 == offset) &&
        (stripe->length() < STRIPE_LENGTH_MAXIMUM))
    {
	unsigned int len = length;
	if (len + stripe->length() > STRIPE_LENGTH_MAXIMUM)
	    len = STRIPE_LENGTH_MAXIMUM - stripe->length();

// 	qDebug("Track::appendAfter(): appending %u samples to %p",
// 	       len, stripe);
	stripe->append(buffer, buf_offset, len);

	offset     += len;
	length     -= len;
	buf_offset += len;
    }

    int index_before = m_stripes.findRef(stripe);

    // append new stripes as long as there is something remaining
    while (length) {
	unsigned int len = length;
	if (len > STRIPE_LENGTH_MAXIMUM)
	    len = STRIPE_LENGTH_MAXIMUM;

	qDebug("Track::appendAfter: new stripe, ofs=%u, len=%u",
	       offset, len); // ###
	Stripe *new_stripe = newStripe(offset, 0);
	Q_ASSERT(new_stripe);
	if (!new_stripe) break;

	// append to the new stripe
	new_stripe->append(buffer, buf_offset, len);
	qDebug("new stripe: [%u ... %u]", new_stripe->start(),
	       new_stripe->end());

	if (index_before >= 0) {
	    // insert after the last one
	    index_before++;
	    qDebug("Track::appendAfter: insert @ #%d", index_before);
	    m_stripes.insert(index_before, new_stripe);
	} else {
	    // the one and only or insert before all others
	    qDebug("Track::appendAfter: prepending");
	    m_stripes.prepend(new_stripe);
	    index_before = 0;
	}
	offset     += len;
	length     -= len;
	buf_offset += len;
    }
}

//***************************************************************************
void Track::moveRight(unsigned int offset, unsigned int shift)
{
    QPtrListIterator<Stripe> it(m_stripes);
    for (it.toLast(); it.current(); --it) {
	Stripe *s = it.current();
	unsigned int start = s->start();
	if (start < offset) break;

	s->setStart(start + shift);
    }
}

//***************************************************************************
void Track::writeSamples(InsertMode mode,
                         unsigned int offset,
                         const QMemArray<sample_t> &buffer,
                         unsigned int buf_offset,
                         unsigned int length)
{
    Q_ASSERT(length);
    if (!length) return; // nothing to do !?

    switch (mode) {
	case Append: {
// 	    qDebug("writeSamples() - Append");
	    appendAfter(m_stripes.last(), offset, buffer, buf_offset, length);
	    break;
	}
	case Insert: {
// 	    qDebug("Track::writeSamples() - Insert @ %u, length=%u",
// 		   offset, length);

	    // find the stripe into which we insert
	    Stripe *target_stripe = 0;
	    Stripe *stripe_before = 0;
	    QPtrListIterator<Stripe> it(m_stripes);
	    for (; it.current(); ++it) {
		Stripe *s = it.current();
		unsigned int st = s->start();
		unsigned int len = s->length();
		if (!len) continue; // skip zero-length tracks

		if (offset >= st+len) stripe_before = s;

		if ((offset >= st) && (offset < st+len)) {
		    // match found
		    target_stripe = s;
		    break;
		}
	    }

// 	    qDebug("stripe_before = %p [%u...%u]", stripe_before,
// 		   stripe_before ? stripe_before->start() : 0,
// 		   stripe_before ? (stripe_before->start() +
// 				   stripe_before->length() - 1) : 0);
// 	    qDebug("target_stripe = %p [%u...%u]", target_stripe,
// 		   target_stripe ? target_stripe->start() : 0,
// 		   target_stripe ? (target_stripe->start() +
// 				   target_stripe->length() - 1) : 0);

	    // if insert is requested immediately after the last
	    // sample of the stripe before
	    if (stripe_before && (offset == stripe_before->start()+
				  stripe_before->length()))
	    {
		// append to the existing stripe
		moveRight(offset, length);
		appendAfter(stripe_before, offset, buffer,
		            buf_offset, length);
		break;
	    }

	    if (!target_stripe) {
		// insert somewhere before, between or after stripes
		moveRight(offset, length);
		appendAfter(stripe_before, offset, buffer,
		            buf_offset, length);
		break;
	    }

	    // if no stripe was found, create a new one and
	    // insert it between the existing ones
	    if (!target_stripe) {
		target_stripe = newStripe(offset, 0);
		Q_ASSERT(target_stripe);
		if (!target_stripe) return;

		// insert into our stripes, if the stripe before
		// is null, the new one will be prepended
		moveRight(offset, length);
		int index = m_stripes.findRef(stripe_before) + 1;
		m_stripes.insert(index, target_stripe);
	    } else {
	        // split the target stripe and insert the samples
		// between the two new ones
		splitStripe(target_stripe, offset-target_stripe->start());
		moveRight(offset, length);
		appendAfter(target_stripe, offset, buffer,
		            buf_offset, length);
	    }

	    break;
	}
	case Overwrite: {
// 	    qDebug("writeSamples() - Overwrite");
	    Stripe *stripe_before = 0;

	    // special case: no stripes present
	    if (m_stripes.isEmpty()) {
		// -> append mode
		qDebug("- no stripes -> appending at zero");
		appendAfter(0, offset, buffer, buf_offset, length);
		break;
	    }

	    unsigned int left  = offset;
	    unsigned int right = offset + length - 1;
//	    qDebug("left=%u, right=%u (offset=%u, length=%u)",
//		   left, right, offset, length);

	    // handle the overlap from left, until we
	    // reach the first gap or nothing remains
	    QPtrListIterator<Stripe> it(m_stripes);
//	    qDebug("number of stripes = %u", m_stripes.count());
	    it.toFirst();
	    while (it.current() && length) {
		Stripe *s = it.current();
		unsigned int start = s->start();
		unsigned int end   = s->end();
//		qDebug("L: checking stripe (#1) [%u...%u] (left=%u, right=%u)",
//		       start, end, left, right);
		if (end < left) {
		    ++it;
		    continue; // ends before offset -> next one
		}
		if (start > left) break;   // gone too far -> done

		// if we get this far, we have some kind of overlap
		if (left < start) break; // overlap at the end -> handle later

		// overlap within the stipe, good.
		Q_ASSERT(left >= start);
		Q_ASSERT(left <= end);

		// overlap at [ left ... min(end,right) ]
		if (end > right) end = right;
		unsigned int len = end - left + 1;
//		qDebug("L: overwrite(left=%u - start=%u, buf_offset=%u, len=%u",
//		       left, start, buf_offset, len);
//		qDebug("L: [%u ....... %u]",start, end);
//		qDebug("    [%u ... %u]", left, left+len-1);
		s->overwrite(left-start, buffer, buf_offset, len);
//		qDebug("L: overwrite done.");

		buf_offset += len;
		left       += len;
		length     -= len;

		if (left == end+1) stripe_before = s;
		++it;
	    }
	    if (!length) break; // nothing more to do

	    // handle the overlap from right, until we reach
	    // the first gap or nothing remains
	    it.toLast();
	    while (it.current() && length) {
		Stripe *s = it.current();
		unsigned int start = s->start();
		unsigned int end   = s->end();
// 		qDebug("R: checking stripe (#2) [%u...%u] (left=%u, right=%u)",
// 		       start, end, left, right);
		if (start > right) {
		    --it;
		    continue; // starts before offset -> previous one
		}
		if (end < right) break; // gone too far -> done

		// if get this far, we have some kind of overlap
		if (left > start) break; // no overlap at the end
		// overlap within the stipe, good.
		Q_ASSERT(left <= start);

		// overlap at [ start ... min(end,right) ]
		if (end > right) end = right;

		unsigned int len = end - start + 1;
// 		qDebug("R: overlap at [%u ... %u]", start, end);
		// ###
// 		qDebug("R: overwrite(left=%u - start=%u, buf_offset=%u, len=%u",
// 		       left, start, buf_offset, len);
// 		qDebug("R: [%u ....... %u]",start, end);
// 		qDebug("R:     [%u ... %u]", end-len+1, end);
		s->overwrite(0, buffer, buf_offset + (start-left), len); // ###
// 		qDebug("R: overwrite done.");

		buf_offset += len;
		right      -= len;
		length     -= len;
		--it;
	    }

	    // erase everything from left to the right, because
	    // it contains gaps and would lead to fragmentation
// 	    qDebug("left=%u ... right=%u, length=%u", left, right, length);
	    if (!length) break; // nothing more to do

	    Q_ASSERT(length == (right-left+1));
	    for (it.toLast(); it.current(); --it) {
		Stripe *s = it.current();
		unsigned int start = s->start();
		unsigned int end   = s->end();

		if (start > right) continue; // too much right
		if (end < left) continue;    // too much left

		// complete overlap
		if ((start >= left) && (end <= right)) {
		    // delete the whole stripe
// 		    qDebug("deleting [%u ... %u]", start, end);
		    deleteStripe(s);
		    continue;
		}

		// partial overlap ? should not happen !!!
		ASSERT(!s);
// 		qDebug("partial overlap: [%u ... %u]", start, end);
	    }

	    // add the rest between the left and right border
// 	    if (stripe_before) qDebug("before: [%u ... %u]",
//	        stripe_before->start(), stripe_before->end()); // ###

	    if (stripe_before && (stripe_before->end()+1 != left))
		stripe_before = 0;

	    appendAfter(stripe_before, left, buffer, buf_offset, length);

	    // look for possible fragmentation at the left side

	    // look for possible fragmentation at the right side

	    break;
	}
    }

}

//***************************************************************************
void Track::dump()
{
    QPtrListIterator<Stripe> it(m_stripes);
    qDebug("------------------------------------");
    for (it.toFirst(); it.current(); ++it) {
	Stripe *s = it.current();
	qDebug("%p - [%10u - %10u] (%10u)",
	       s, s->start(), s->end(), s->length());
    }
    qDebug("------------------------------------");
}

//***************************************************************************
//***************************************************************************
