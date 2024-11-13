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

#include "config.h"

#include <algorithm>
#include <new>

#include <QMutexLocker>
#include <QReadLocker>
#include <QWriteLocker>

#include "libkwave/SampleReader.h"
#include "libkwave/Stripe.h"
#include "libkwave/Track.h"
#include "libkwave/TrackWriter.h"
#include "libkwave/Utils.h"
#include "libkwave/Writer.h"

/**
 * Optimal size of a stripe [samples].
 * When creating a new stripe, it should have this size
 */
#define STRIPE_LENGTH_OPTIMAL (4UL * 1024UL * 1024UL) /* 16MB */

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
Kwave::Track::Track()
    :m_lock(), m_lock_usage(), m_stripes(), m_selected(true),
     m_uuid(QUuid::createUuid())
{
}

//***************************************************************************
Kwave::Track::Track(sample_index_t length, QUuid *uuid)
    :m_lock(), m_lock_usage(), m_stripes(), m_selected(true),
     m_uuid((uuid) ? *uuid : QUuid::createUuid())
{
    if (length <= STRIPE_LENGTH_MAXIMUM) {
        if (length) appendStripe(length);
    } else {
        Stripe s(length - STRIPE_LENGTH_OPTIMAL);
        s.resize(STRIPE_LENGTH_OPTIMAL);
        if (s.length()) m_stripes.push_back(s);
    }
}

//***************************************************************************
Kwave::Track::~Track()
{
    // wait until all readers are finished
    QWriteLocker lock_usage(&m_lock_usage);

    // don't allow any further operation
    QMutexLocker lock(&m_lock);

    // delete all stripes
    m_stripes.clear();
}

//***************************************************************************
void Kwave::Track::appendStripe(sample_index_t length)
{
    sample_index_t start = unlockedLength();
    do {
        unsigned int len = Kwave::toUint(
            qMin<sample_index_t>(STRIPE_LENGTH_MAXIMUM, length));

        Stripe s(start);
        s.resize(len);
        if (len) emit sigSamplesInserted(this, start, len);

        length -= len;
        start  += len;
        m_stripes.push_back(s);
    } while (length);

}

//***************************************************************************
Kwave::Stripe Kwave::Track::splitStripe(Kwave::Stripe &stripe,
                                        unsigned int offset)
{
    Q_ASSERT(offset < stripe.length());
    Q_ASSERT(offset);
    if (offset >= stripe.length()) return Stripe();
    if (!offset) return Stripe();

    // create a new stripe with the data that has been split off
    Stripe s(stripe.start() + offset, stripe, offset);
    if (!s.length()) return Stripe();

    // shrink the old stripe
    stripe.resize(offset);

//  qDebug("Kwave::Track::splitStripe(%p, %u): new stripe at "
//      "[%llu ... %llu] (%u)", static_cast<void *>(&stripe),
//      offset, s.start(), s.end(), s.length());

    return s;
}

//***************************************************************************
bool Kwave::Track::mergeStripe(Kwave::Stripe &stripe)
{
    sample_index_t left  = stripe.start();
    sample_index_t right = stripe.end();

//  qDebug("Track::mergeStripe() [%llu - %llu]", left, right);
//  dump();

    // remove all stripes that are overlapped completely by
    // this stripe and crop stripes that overlap partially
    unlockedDelete(left, right - left + 1, true);

//  qDebug("Track::mergeStripe() [%llu - %llu] - after delete", left, right);
//  dump();

    // find the stripe before which we have to insert
    std::vector<Stripe>::iterator where = std::find_if(
        m_stripes.begin(), m_stripes.end(),
        [right] (const Stripe &s) -> bool
        { return (s.start() > right); }
    );

    if (where != m_stripes.end()) {
        // insert before some existing stripe
//      qDebug("insert before %p [%llu - %llu]",
//          static_cast<void *>(&(*where)), where->start(), where->end());
        m_stripes.insert(where, stripe);
    } else {
        // the one and only or insert after all others
//      qDebug("append %p", static_cast<void *>(&(stripe)));
        m_stripes.push_back(stripe);
    }

//  qDebug("Track::mergeStripe() - done");
//  dump();
    return true;
}

//***************************************************************************
sample_index_t Kwave::Track::length()
{
    QMutexLocker lock(&m_lock);
    return unlockedLength();
}

//***************************************************************************
sample_index_t Kwave::Track::unlockedLength()
{
    if (m_stripes.empty()) return 0;
    const Stripe &s = m_stripes.back();
    return s.start() + s.length();
}

//***************************************************************************
Kwave::Writer *Kwave::Track::openWriter(Kwave::InsertMode mode,
                                        sample_index_t left,
                                        sample_index_t right)
{
    // create the input stream
    Kwave::Writer *stream =
        new(std::nothrow) Kwave::TrackWriter(*this, mode, left, right);
    Q_ASSERT(stream);

    if (stream) {
        // at the point of time when the writer gets closed we can take
        // care of fragmentation
        connect(stream, SIGNAL(destroyed(QObject*)),
                this,   SLOT(defragment()),
                Qt::DirectConnection);
    }

    return stream;
}

//***************************************************************************
Kwave::Stripe::List Kwave::Track::stripes(sample_index_t left,
                                          sample_index_t right)
{
    QMutexLocker lock(&m_lock);

    // collect all stripes that are in the requested range
    Kwave::Stripe::List stripes(left, right);
    for (const Stripe &stripe : m_stripes) {
        if (!stripe.length()) continue;
        sample_index_t start = stripe.start();
        sample_index_t end   = stripe.end();

        if (end < left) continue; // not yet in range
        if (start > right) break; // done

        if ((end <= right) && (start >= left)) {
            // append the stripe as it is, unmodified
            stripes.append(stripe);
            continue;
        }

        // there is only some overlap, no 100% match
        // -> make a cropped copy
        Stripe cropped(stripe);

        // remove data after the end of the selection
        if (end > right)
            cropped.resize(Kwave::toUint(cropped.length() - (end - right)));

        // remove data before the start of the selection
        if (start < left) {
            cropped.deleteRange(0, Kwave::toUint(left - start));
            cropped.setStart(left);
        }

        stripes.append(cropped);
    }

    return stripes;
}

//***************************************************************************
bool Kwave::Track::mergeStripes(const Kwave::Stripe::List &stripes)
{
    bool succeeded = true;
    {
        QMutexLocker lock(&m_lock);
        for (const Stripe &stripe : stripes) {
            Stripe s(stripe);
            if (!mergeStripe(s)) {
                succeeded = false;
                break;
            }
        }
    }

    // do some defragmentation, to combine the ends of the inserted stripes
    defragment();

    const sample_index_t left  = stripes.left();
    const sample_index_t right = stripes.right();
    emit sigSamplesModified(this, left, right - left + 1);
    return succeeded;
}

//***************************************************************************
Kwave::SampleReader *Kwave::Track::openReader(Kwave::ReaderMode mode,
        sample_index_t left, sample_index_t right)
{
    QMutexLocker lock(&m_lock);

    const sample_index_t length = unlockedLength();
    if (right >= length) right = (length) ? (length - 1) : 0;

    // collect all stripes that are in the requested range
    Kwave::Stripe::List stripe_list = stripes(left, right);

    // create the input stream
    Kwave::SampleReader *stream =
        new(std::nothrow) Kwave::SampleReader(mode, stripe_list);
    Q_ASSERT(stream);
    return stream;
}

//***************************************************************************
void Kwave::Track::deleteRange(sample_index_t offset, sample_index_t length,
                               bool make_gap)
{
    if (!length) return;

//  qDebug("Kwave::Track::deleteRange() [%llu ... %llu] (%llu)",
//      offset, offset + length - 1, length);

    {
        QMutexLocker lock(&m_lock);
        unlockedDelete(offset, length, make_gap);
    }

    // deletion without gap might have left some fragments
    if (!make_gap)
        defragment();

    emit sigSamplesDeleted(this, offset, length);
}

//***************************************************************************
bool Kwave::Track::insertSpace(sample_index_t offset, sample_index_t shift)
{
//     qDebug("Kwave::Track::insertSpace(offset=%u,shift=%u)",offset, shift);
    if (!shift) return true;

    {
        QMutexLocker lock(&m_lock);
        sample_index_t len = unlockedLength();
        if (offset < len) {
            // find out whether the offset is within a stripe and
            // split that one if necessary
            std::vector<Stripe>::iterator it(m_stripes.begin());
            while (it != m_stripes.end()) {
                Stripe &s = *it;
                sample_index_t end    = s.end();
                if (end < offset) {
                    ++it;
                    continue; // skip, stripe is at left
                }

                sample_index_t start  = s.start();
                if (start >= offset) break; // not "within" the stripe

//             qDebug("Kwave::Track::insertSpace => splitting [%llu...%llu]",
//                      start,end);
                Stripe new_stripe = splitStripe(s,
                    Kwave::toUint(offset - start)
                );
                if (!new_stripe.length()) return false; // OOM ?
                m_stripes.insert(++it, new_stripe);
                break;
            }

            // move all stripes that are after the offset right
//          qDebug("Kwave::Track::insertSpace => moving right");
            moveRight(offset, shift);
        } else {
//          qDebug("Kwave::Track::insertSpace => appending stripe at %llu",
//                  offset + shift - 1);
            Stripe s(offset + shift - 1);
            s.resize(1);
            if (s.length()) m_stripes.push_back(s);
        }
    }

//  dump();
    emit sigSamplesInserted(this, offset, shift);
    return true;
}

//***************************************************************************
void Kwave::Track::unlockedDelete(sample_index_t offset, sample_index_t length,
                                  bool make_gap)
{
    if (!length) return;

    // add all stripes within the specified range to the list
    sample_index_t left  = offset;
    sample_index_t right = offset + length - 1;

    std::vector<Stripe>::reverse_iterator it_r(m_stripes.rbegin());
    while (it_r != m_stripes.rend()) {
        Stripe        &s      = *it_r;
        sample_index_t start  = s.start();
        sample_index_t end    = s.end();

        if (end   < left)  break;                // done, stripe is at left
        if (start > right) { ++it_r; continue; } // skip, stripe is at right

        if ((left <= start) && (right >= end)) {
            // case #1: total overlap -> delete whole stripe
//          qDebug("deleting stripe [%llu ... %llu]", start, end);
            m_stripes.erase((++it_r).base());
            if (m_stripes.empty()) break;
            continue;
        } else /* if ((end >= left) && (start <= right)) */ {
            //        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
            //        already checked above
            // partial stripe overlap
            sample_index_t ofs = (start < left) ? left : start;
            if (end > right) end = right;
//          qDebug("deleting [%llu ... %llu] (start=%llu, ofs-start=%llu, len=%llu)",
//              ofs, end, start, ofs-start, end - ofs + 1);

            if (!make_gap ||
                ((left <= s.start()) && (right  < s.end())) ||
                ((left  > s.start()) && (right >= s.end())))
            {
                // case #2: delete without creating a gap
                // case #3: delete from the left only
                // case #4: delete from the right only
//              qDebug("    deleting within the stripe");
                s.deleteRange(
                    Kwave::toUint(ofs - start),
                    Kwave::toUint(end - ofs + 1)
                );
                if (!s.length()) break; // OOM ?

                // if deleted from start
                if (left <= s.start()) {
                    // move right, producing a (temporary) gap
//                  qDebug("shifting [%llu ... %llu] to %llu",
//                          start, s.end(), end + 1);
                    s.setStart(end + 1);
                }
            } else {
                // case #5: delete from the middle and produce a gap
                //          by splitting off a new stripe
//              qDebug("    splitting off to new stripe @ %llu (ofs=%llu)",
//                  right + 1, right + 1 - start);
                Stripe new_stripe = splitStripe(s,
                    Kwave::toUint(right + 1 - start));
                if (!new_stripe.length()) break; // OOM ?
                m_stripes.insert(it_r.base(), new_stripe);

                // erase to the end (reduce size)
                const unsigned int todel = Kwave::toUint(s.end() - ofs + 1);
//              qDebug("ofs-start=%llu, s->end()-ofs+1=%u [%llu...%llu] (%u)",
//                  ofs-start, todel, s.start(), s.end(), s.length());
                s.deleteRange(Kwave::toUint(ofs - start), todel);
//              qDebug("length now: %u [%llu ... %llu]", s.length(),
//                  s.start(), s.end());
//              Q_ASSERT(s.length());
            }
            ++it_r;
//          Q_ASSERT(s.length());
        }
    }

    // loop over all remaining stripes and move them left
    // (maybe we start the search one stripe too left,
    // but this doesn't matter, we don't care...)
    if (!make_gap) {
        std::vector<Stripe>::iterator it(it_r.base());
        for (; it != m_stripes.end(); ++it) {
            Stripe &s = *it;
//          qDebug("checking for shift [%llu ... %llu]",
//                  s.start(), s.end());
            Q_ASSERT(s.start() != right);
            if (s.start() > right) {
                // move left
//              qDebug("moving stripe %p [%llu.%llu] %llu samples left",
//                     static_cast<void *>(&s), s.start(), s.end(), length);
                Q_ASSERT(s.start() >= length);
                s.setStart(s.start() - length);
            }
        }
    }
}

//***************************************************************************
void Kwave::Track::select(bool selected)
{
    if (m_selected == selected) return;
    m_selected = selected;
    emit sigSelectionChanged(m_selected);
}

//***************************************************************************
void Kwave::Track::toggleSelection()
{
    select(!selected());
}

//***************************************************************************
bool Kwave::Track::appendAfter(Stripe *stripe,  sample_index_t offset,
                               const Kwave::SampleArray &buffer,
                               unsigned int buf_offset, unsigned int length)
{
    Q_ASSERT(buf_offset + length <= buffer.size());
    if (buf_offset + length > buffer.size()) return false;

    // append to the last stripe if one exists and it's not full
    // and the offset is immediately after the last stripe
    if ((stripe) && (stripe->end() + 1 == offset) &&
        (stripe->length() < STRIPE_LENGTH_MAXIMUM))
    {
        unsigned int len = length;
        if (len + stripe->length() > STRIPE_LENGTH_MAXIMUM)
            len = STRIPE_LENGTH_MAXIMUM - stripe->length();

//      qDebug("Kwave::Track::appendAfter(): appending %u samples to %p",
//              len, static_cast<void *>(stripe));
        if (!stripe->append(buffer, buf_offset, len))
            return false; // out of memory

        offset     += len;
        length     -= len;
        buf_offset += len;
    }

    std::vector<Stripe>::iterator where(m_stripes.begin());
    if (stripe != nullptr) {
        where = std::find(m_stripes.begin(), m_stripes.end(), *stripe);
        if (where != m_stripes.end()) ++where;
    }

    // append new stripes as long as there is something remaining
    while (length) {
        unsigned int len = Kwave::toUint(qMin<sample_index_t>(
            length, STRIPE_LENGTH_MAXIMUM));

//      qDebug("Kwave::Track::appendAfter: new stripe, ofs=%llu, len=%u",
//          offset, len);
        Stripe new_stripe(offset);

        // append to the new stripe
        if (!new_stripe.append(buffer, buf_offset, len)) {
            return false; /* out of memory */
        }
        Q_ASSERT(new_stripe.length() == len);

//      qDebug("new stripe: [%llu ... %llu] (%u)", new_stripe.start(),
//              new_stripe.end(), new_stripe.length());
        if (where == m_stripes.begin()) {
//          qDebug("Kwave::Track::appendAfter: prepending");
            where = std::next(m_stripes.insert(where, new_stripe));
        } else if (where != m_stripes.end()) {
            // insert after the previous one
//          qDebug("Kwave::Track::appendAfter: insert after [%10llu - %10llu]",
//              stripe->start(), stripe->end());
            where = std::next(m_stripes.insert(where, new_stripe));
        } else {
            // append at the end
//          qDebug("Kwave::Track::appendAfter: appending");
            m_stripes.push_back(new_stripe);
        }
        offset     += len;
        length     -= len;
        buf_offset += len;
    }

    return true;
}

//***************************************************************************
void Kwave::Track::moveRight(sample_index_t offset, sample_index_t shift)
{
    if (m_stripes.empty()) return;

    for (std::vector<Stripe>::reverse_iterator it = m_stripes.rbegin();
         it != m_stripes.rend(); ++it) {
        Stripe &s = *it;
        sample_index_t start = s.start();
        if (start < offset) break;

        s.setStart(start + shift);
    }
}

//***************************************************************************
bool Kwave::Track::writeSamples(Kwave::InsertMode mode,
                                sample_index_t offset,
                                const Kwave::SampleArray &buffer,
                                unsigned int buf_offset,
                                unsigned int length)
{
    Q_ASSERT(length);
    if (!length) return true; // nothing to do !?

    switch (mode) {
        case Kwave::Append: {
//          qDebug("writeSamples() - Append");
            bool appended;
            {
                QMutexLocker _lock(&m_lock);
                appended = appendAfter(
                    m_stripes.empty() ? nullptr : &(m_stripes.back()),
                    offset, buffer,
                    buf_offset, length);
            }
            if (appended)
                emit sigSamplesInserted(this, offset, length);
            else
                return false; /* out of memory */
            break;
        }
        case Kwave::Insert: {
            m_lock.lock();

//          qDebug("Kwave::Track::writeSamples() - Insert @ %llu, length=%u",
//                 offset, length);

            // find the stripe into which we insert
            Stripe *target_stripe = nullptr;
            Stripe *stripe_before = nullptr;
            for (Stripe &s : m_stripes) {
                sample_index_t st  = s.start();
                sample_index_t len = s.length();
                if (!len) continue; // skip zero-length tracks

                if (offset >= st + len) stripe_before = &s;

                if ((offset >= st) && (offset < st+len)) {
                    // match found
                    target_stripe = &s;
                    break;
                }
            }

//          qDebug("stripe_before = %p [%llu...%llu]",
//                  static_cast<void *>(stripe_before),
//                  stripe_before ? stripe_before->start() : 0,
//                  stripe_before ? (stripe_before->start() +
//                                  stripe_before->length() - 1) : 0);
//          qDebug("target_stripe = %p [%llu...%llu]",
//                  static_cast<void *>(target_stripe),
//                  target_stripe ? target_stripe->start() : 0,
//                  target_stripe ? (target_stripe->start() +
//                              target_stripe->length() - 1) : 0);

            // if insert is requested immediately after the last
            // sample of the stripe before
            if (stripe_before && (offset == stripe_before->start() +
                                  stripe_before->length()))
            {
                // append to the existing stripe
                moveRight(offset, length);
                appendAfter(stripe_before, offset, buffer,
                            buf_offset, length);
                m_lock.unlock();
                emit sigSamplesInserted(this, offset, length);
                break;
            }

            // if no stripe was found, create a new one and
            // insert it between the existing ones
            if (!target_stripe || (offset == target_stripe->start())) {
                // insert somewhere before, between or after stripes
                moveRight(offset, length);
                appendAfter(stripe_before, offset, buffer,
                            buf_offset, length);
            } else {
                // split the target stripe and insert the samples
                // between the two new ones
                Stripe new_stripe = splitStripe(*target_stripe,
                    Kwave::toUint(offset - target_stripe->start())
                );
                if (!new_stripe.length()) {
                    m_lock.unlock();
                    break;
                }
                std::vector<Stripe>::iterator it = std::find(
                    m_stripes.begin(), m_stripes.end(), *target_stripe);
                m_stripes.insert(++it, new_stripe);

                moveRight(offset, length);
                appendAfter(target_stripe, offset, buffer,
                            buf_offset, length);
            }

            m_lock.unlock();
            emit sigSamplesInserted(this, offset, length);

            break;
        }
        case Kwave::Overwrite: {
//          const sample_index_t left  = offset;
//          const sample_index_t right = offset + length - 1;
//          qDebug("writeSamples() - Overwrite [%llu - %llu]", left, right);

            {
                QMutexLocker _lock(&m_lock);

                // delete old content, producing a gap
                unlockedDelete(offset, length, true);

                // fill in the content of the buffer, append to the stripe
                // before the gap if possible
                Stripe *stripe_before = nullptr;
                for (Stripe &s : m_stripes) {
                    if (s.start() >= offset) break;
                    if (s.end() < offset) stripe_before = &s;
                }
                appendAfter(stripe_before, offset, buffer, buf_offset, length);
            }
            emit sigSamplesModified(this, offset, length);
            break;
        }
        DEFAULT_IMPOSSIBLE;
    }

    return true;
}

//***************************************************************************
void Kwave::Track::defragment()
{

    if (!m_lock.tryLock()) {
        // this could happen when there are two or more writers
//      qDebug("Track::defragment() - busy");
        return;
    }

    if (!m_stripes.empty())
    {
//      qDebug("Track::defragment(), state before:");
//      dump();

//      unsigned int   index  = 0;
        Kwave::Stripe *before = nullptr;
        Kwave::Stripe *stripe = nullptr;

        // use a quick and simple algorithm:
        // iterate over all stripes and analyze pairwise
        std::vector<Stripe>::iterator it(m_stripes.begin());
        while (it != m_stripes.end()) {
            before = stripe;
            stripe = &(*it);
            if (!before) {
                ++it;
                continue; // skip the first entry
            }
//          index++;

//          qDebug("Track::defragment(), checking #%u [%llu..%llu] (%u)",
//                  index, stripe->start(), stripe->end(), stripe->length());

            const sample_index_t before_start = before->start();
            const sample_index_t stripe_end   = stripe->end();
            const sample_index_t combined_len = stripe_end - before_start + 1;

            if (combined_len > STRIPE_LENGTH_MAXIMUM) {
                ++it;
                continue; // would be too large
            }

            if ((before->length() < STRIPE_LENGTH_MINIMUM) ||
                (stripe->length() < STRIPE_LENGTH_MINIMUM)) {
//              qDebug("Track::defragment(), combine  #%u [%llu..%llu] & "
//                     "#%u [%llu..%llu] => [%llu..%llu] (%llu)",
//                     index - 1, before->start(), before->end(),
//                     index,     stripe->start(), stripe->end(),
//                     before->start(), stripe->end(), combined_len);

                // try to resize the stripe before to contain the
                // combined length
                const unsigned int offset = Kwave::toUint(
                    stripe->start() - before_start);
                if (!before->combine(offset, *stripe)) {
                    ++it;
                    continue; // not possible, maybe OOM ?
                }

                // remove the current stripe, to avoid an overlap
                it = m_stripes.erase(it);
                stripe = before;
//              index--;
            }
            else ++it;
        }

        qDebug("Track::defragment(), state after:");
        dump();
    }

    m_lock.unlock();
}

//***************************************************************************
void Kwave::Track::use()
{
    m_lock_usage.lockForRead();
}

//***************************************************************************
void Kwave::Track::release()
{
    m_lock_usage.unlock();
}

//***************************************************************************
void Kwave::Track::dump()
{
    qDebug("------------------------------------");
    unsigned int   index    = 0;
    sample_index_t last_end = 0;
    for (const Stripe &s : m_stripes) {
        sample_index_t start = s.start();
        if (index && (start <= last_end))
            qDebug("--- OVERLAP ---");
        if (start > last_end+1)
            qDebug("       : GAP              [%10lu - %10lu] (%10lu)",
            static_cast<unsigned long int>(last_end + ((index) ? 1 : 0)),
            static_cast<unsigned long int>(start - 1),
            static_cast<unsigned long int>(start - last_end -
                ((index) ? 1 : 0)));
        qDebug("#%6d: %p - [%10lu - %10lu] (%10lu)",
               index++, static_cast<const void *>(&s),
               static_cast<unsigned long int>(s.start()),
               static_cast<unsigned long int>(s.end()),
               static_cast<unsigned long int>(s.length()));
        last_end = s.end();
    }
    qDebug("------------------------------------");
}

//***************************************************************************
//***************************************************************************

#include "moc_Track.cpp"
