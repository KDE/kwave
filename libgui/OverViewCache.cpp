/***************************************************************************
      OverViewCache.cpp  -  fast cache for sample data overview
                             -------------------
    begin                : Mon May 20 2002
    copyright            : (C) 2000 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <math.h>

#include <QColor>
#include <QPainter>

#include "libkwave/MultiTrackReader.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SignalManager.h"
#include "libkwave/Track.h"
#include "libkwave/Utils.h"

#include "libgui/OverViewCache.h"

#define CACHE_SIZE 8192           /**< number of cache entries */

//***************************************************************************
Kwave::OverViewCache::OverViewCache(Kwave::SignalManager &signal,
                                    sample_index_t src_offset,
                                    sample_index_t src_length,
                                    const QVector<unsigned int> *src_tracks)
    :m_signal(signal),
     m_selection(&signal, src_offset, src_length, src_tracks),
     m_track_state(),
     m_minmax(),
     m_scale(1),
     m_lock()
{

    connect(&m_selection, SIGNAL(sigTrackInserted(quint64)),
            this,         SLOT(slotTrackInserted(quint64)));
    connect(&m_selection, SIGNAL(sigTrackDeleted(quint64)),
            this,         SLOT(slotTrackDeleted(quint64)));
    connect(&m_selection, SIGNAL(sigLengthChanged(sample_index_t)),
            this,         SLOT(slotLengthChanged(sample_index_t)));
    connect(
        &m_selection,
        SIGNAL(sigInvalidated(quint64,sample_index_t,sample_index_t)),
        this,
        SLOT(slotInvalidated(quint64,sample_index_t,sample_index_t))
    );

    // take over the initial list of tracks
    for (const quint64 uid : m_selection.allTracks())
        slotTrackInserted(uid);
}

//***************************************************************************
Kwave::OverViewCache::~OverViewCache()
{
    QMutexLocker lock(&m_lock);
    m_track_state.clear();
}

//***************************************************************************
void Kwave::OverViewCache::scaleUp()
{
    Q_ASSERT(m_scale);
    if (!m_scale) return;

    // calculate the new scale
    const sample_index_t len = m_selection.length();
    unsigned int shrink = Kwave::toUint(len / (m_scale * CACHE_SIZE));
    while (len > CACHE_SIZE * m_scale * shrink) {
        shrink++;
    }
    if (shrink <= 1) return; // nothing to shrink, just ignore new scale

    // loop over all tracks
    for (auto ts : m_track_state)
    {
        unsigned int dst = 0;
        unsigned int count = CACHE_SIZE / shrink;
        Q_ASSERT(count <= CACHE_SIZE);

        // source pointers
        CacheState *sstate = ts.m_state.data();
        sample_t   *smin   = ts.m_min.data();
        sample_t   *smax   = ts.m_max.data();

        // destination pointers
        CacheState *dstate = sstate;
        sample_t   *dmin   = smin;
        sample_t   *dmax   = smax;

        // loop over all entries to be shrunk
        while (dst < count) {
            sample_t min = SAMPLE_MAX;
            sample_t max = SAMPLE_MIN;
            CacheState state = Unused;
            for (unsigned int i = 0; i < shrink; ++i) {
                if (*smin < min) min = *smin;
                if (*smax > max) max = *smax;
                if (*sstate < state) state = *sstate;
                ++smin;
                ++smax;
                ++sstate;
            }
            *dmin = min;
            *dmax = max;
            *dstate = state;
            ++dmin;
            ++dmax;
            ++dstate;
            ++dst;
        }

        // the rest will be unused
        while (dst++ < CACHE_SIZE) {
            *dstate = Unused;
            dstate++;
        }
    }

    m_scale *= shrink;
}

//***************************************************************************
void Kwave::OverViewCache::scaleDown()
{
    const sample_index_t len = m_selection.length();
    quint64 new_scale = static_cast<quint64>(rint(ceil(len / CACHE_SIZE)));
    if (!new_scale) new_scale = 1;
    if (m_scale == new_scale) return;

    m_scale = new_scale;
    invalidateCache(0, 0, CACHE_SIZE - 1);
}

//***************************************************************************
void Kwave::OverViewCache::invalidateCache(quint64 track_id,
                                           unsigned int first,
                                           unsigned int last)
{
    if (track_id) {
        // invalidate a single track
        QHash<quint64, TrackState>::iterator it = m_track_state.find(track_id);
        Q_ASSERT(it != m_track_state.end());
        if (it == m_track_state.end()) return;

        if (last >= CACHE_SIZE) last = CACHE_SIZE - 1;

//      qDebug("OverViewCache[%p]::invalidateCache(%s, %u, %u)",
//             static_cast<void *>(this), DBG(track_id->toString()),
//             first, last);

        for (unsigned int pos = first; pos <= last; ++pos)
            (*it).m_state[pos] = Invalid;
    } else {
        // invalidate all tracks
        for (QHash<quint64, TrackState>::key_iterator
            it(m_track_state.keyBegin()); it != m_track_state.keyEnd(); ++it)
        {
            invalidateCache(*it, first, last);
        }
    }
}

//***************************************************************************
void Kwave::OverViewCache::slotTrackInserted(quint64 track_id)
{
    QMutexLocker lock(&m_lock);

    // just to be sure: check scale again, maybe it was the first track
    if ((m_selection.length() / m_scale) > CACHE_SIZE)
        scaleUp();
    if ((m_selection.length() / m_scale) < (CACHE_SIZE / 4))
        scaleDown();

    m_track_state.insert(track_id, {
        QVector<CacheState>(CACHE_SIZE, Unused),
        QVector<sample_t>(CACHE_SIZE, SAMPLE_MAX),
        QVector<sample_t>(CACHE_SIZE, SAMPLE_MIN)
    });

    // mark the new cache content as invalid
    invalidateCache(track_id, 0, CACHE_SIZE - 1);

    emit changed();
}

//***************************************************************************
void Kwave::OverViewCache::slotTrackDeleted(quint64 track_id)
{
    QMutexLocker lock(&m_lock);
    m_track_state.remove(track_id);
    emit changed();
}

//***************************************************************************
void Kwave::OverViewCache::slotInvalidated(quint64 track_id,
                                           sample_index_t first,
                                           sample_index_t last)
{
    QMutexLocker lock(&m_lock);

    // adjust offsets, absolute -> relative
    sample_index_t offset = m_selection.offset();
    Q_ASSERT(first >= offset);
    Q_ASSERT(last  >= offset);
    Q_ASSERT(last  >= first);
    first -= offset;
    last  -= offset;

    unsigned int first_idx = Kwave::toUint(first / m_scale);
    unsigned int last_idx;
    if (last >= (SAMPLE_INDEX_MAX - (m_scale - 1)))
        last_idx = CACHE_SIZE - 1;
    else
        last_idx = Kwave::toUint(
            qMin(Kwave::round_up(last, m_scale) / m_scale,
            quint64(CACHE_SIZE - 1))
        );

    invalidateCache(track_id, first_idx, last_idx);
    emit changed();
}

//***************************************************************************
void Kwave::OverViewCache::slotLengthChanged(sample_index_t new_length)
{
    QMutexLocker lock(&m_lock);

    // just to be sure: check scale again, maybe it was the first track
    if ((new_length / m_scale) > CACHE_SIZE)
        scaleUp();
    if ((new_length / m_scale) < (CACHE_SIZE / 4))
        scaleDown();
}

//***************************************************************************
int Kwave::OverViewCache::getMinMax(int width, MinMaxArray &minmax)
{
    QMutexLocker lock(&m_lock);
    int retval = 0;

    const sample_index_t first  = m_selection.offset();
    const sample_index_t last   = m_selection.last();
    const sample_index_t length = m_selection.length();
    if (!length)
        return 0;

    // resize the target buffer if necessary
    if (minmax.count() < width)
        minmax.resize(width);
    if (minmax.count() < width) // truncated, OOM
        width = static_cast<int>(minmax.count());

    QVector<unsigned int> track_list;
    const QList<quint64> selected_tracks = m_selection.allTracks();
    for (unsigned int track : m_signal.allTracks())
        if (selected_tracks.contains(m_signal.uidOfTrack(track)))
            track_list.append(track);
    if (track_list.isEmpty())
        return 0;

    Kwave::MultiTrackReader src(
        Kwave::SinglePassForward,
        m_signal, track_list, first, last
    );

    if ((length / m_scale < 2) || src.isEmpty() || !m_track_state.count())
        return 0; // empty ?

    // loop over all min/max buffers and make their content valid
    for (int index = 0; index < track_list.count(); ++index) {
        unsigned int count = qBound<unsigned int>(
            1, Kwave::toUint(length / m_scale), CACHE_SIZE);

        quint64 uid = m_signal.uidOfTrack(track_list[index]);
        if (uid == 0) continue; // track has just been deleted

        QHash<quint64, TrackState>::iterator it = m_track_state.find(uid);

        // check: maybe slotTrackInserted has not yet been called
        //        or slotTrackDeleted has just been called
        if (it == m_track_state.end())
            continue;

        CacheState *state = (*it).m_state.data();
        sample_t   *min   = (*it).m_min.data();
        sample_t   *max   = (*it).m_max.data();
        Q_ASSERT(min && max && state);
        Kwave::SampleReader *reader = src[index];
        Q_ASSERT(reader);

        if (!reader || !min || !max || !state) continue;

        for (unsigned int ofs = 0; ofs < count; ++ofs) {
            if (state[ofs] == Valid)  continue;
            if (state[ofs] == Unused) continue;

            // get min/max
            sample_index_t first_idx = m_selection.offset() + (ofs * m_scale);
            sample_index_t last_idx  = first_idx + m_scale - 1;
            reader->minMax(first_idx, last_idx, min[ofs], max[ofs]);
            state[ofs] = Valid;
        }
    }

    // loop over all min/max buffers
    for (int x = 0; x < width; ++x) {
        unsigned int count = qBound<unsigned int>(
            1, Kwave::toUint(length / m_scale), CACHE_SIZE);

        // get the corresponding cache index
        unsigned int index = ((count - 1) * x) / (width - 1);
        unsigned int last_index  = ((count - 1) * (x + 1)) / (width - 1);
        Q_ASSERT(index < CACHE_SIZE);
        if (index >= CACHE_SIZE) index = CACHE_SIZE - 1;
        if (last_index > index) last_index--;
        if (last_index >= CACHE_SIZE) last_index = CACHE_SIZE - 1;

        // loop over all cache indices
        sample_t minimum = SAMPLE_MAX;
        sample_t maximum = SAMPLE_MIN;
        for (; index <= last_index; ++index) {
            // loop over all tracks
            for (const auto &it : m_track_state) {
                const CacheState *state = it.m_state.constData();
                const sample_t   *min   = it.m_min.constData();
                const sample_t   *max   = it.m_max.constData();
                Q_ASSERT(state && min && max);
                if (!state || !min || !max) continue;
                if (state[index] != Valid) {
                    if (minimum > 0) minimum = 0;
                    if (maximum < 0) maximum = 0;
                    continue;
                }

                if (min[index] < minimum) minimum = min[index];
                if (max[index] > maximum) maximum = max[index];
            }
        }

        minmax[x].min = minimum;
        minmax[x].max = maximum;
        retval++;
    }

    return retval;
}

//***************************************************************************
QImage Kwave::OverViewCache::getOverView(int width, int height,
                                         const QColor &fg, const QColor &bg,
                                         double gain)
{
    QMutexLocker lock(&m_lock);

    QImage bitmap(width, height, QImage::Format_ARGB32_Premultiplied);
    if ((width < 2) || (height < 3) || bitmap.isNull()) return bitmap;

    QPainter p;
    p.begin(&bitmap);
    p.fillRect(bitmap.rect(), bg);
    p.setPen(fg);

    int count = getMinMax(width, m_minmax);
    if (count < 1) {
        p.end();
        return bitmap; // empty ?
    }

    // draw the bitmap
    for (int x = 0; x < count; ++x) {
        const int middle = (height >> 1);
        const double scale = static_cast<double>(middle) /
                             static_cast<double>(SAMPLE_MAX);
        double min = m_minmax[x].min * scale;
        double max = m_minmax[x].max * scale;

        if (gain != 1.0) {
            min *= gain;
            max *= gain;
            if (min < -middle) min = -middle;
            if (min > +middle) min = +middle;
            if (max < -middle) max = -middle;
            if (max > +middle) max = +middle;
        }
        p.drawLine(x, middle - Kwave::toInt(max),
                   x, middle - Kwave::toInt(min));
    }

    p.end();
    return bitmap;
}

//***************************************************************************
//***************************************************************************

#include "moc_OverViewCache.cpp"
