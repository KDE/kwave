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

#include "config.h"
#include "math.h"

#include <QColor>
#include <QPainter>

#include "libkwave/MultiTrackReader.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SignalManager.h"
#include "libkwave/String.h"
#include "libkwave/Track.h"
#include "libkwave/Utils.h"

#include "libgui/OverViewCache.h"

#define CACHE_SIZE 8192           /**< number of cache entries */

//***************************************************************************
Kwave::OverViewCache::OverViewCache(Kwave::SignalManager &signal,
                                    sample_index_t src_offset,
                                    sample_index_t src_length,
                                    const QList<unsigned int> *src_tracks)
    :m_signal(signal),
     m_selection(&signal, src_offset, src_length, src_tracks),
     m_min(), m_max(), m_state(), m_minmax(),
     m_scale(1),
     m_lock(QMutex::Recursive)
{

    connect(&m_selection, SIGNAL(sigTrackInserted(QUuid)),
            this,         SLOT(slotTrackInserted(QUuid)));
    connect(&m_selection, SIGNAL(sigTrackDeleted(QUuid)),
            this,         SLOT(slotTrackDeleted(QUuid)));
    connect(&m_selection, SIGNAL(sigLengthChanged(sample_index_t)),
            this,         SLOT(slotLengthChanged(sample_index_t)));
    connect(
	&m_selection,
	SIGNAL(sigInvalidated(const QUuid*,sample_index_t,sample_index_t)),
	this,
	SLOT(slotInvalidated(const QUuid*,sample_index_t,sample_index_t))
    );

    // take over the initial list of tracks
    foreach (const QUuid &uuid, m_selection.allTracks())
	slotTrackInserted(uuid);
}

//***************************************************************************
Kwave::OverViewCache::~OverViewCache()
{
    QMutexLocker lock(&m_lock);

    m_state.clear();
    m_min.clear();
    m_max.clear();
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
    foreach (const QUuid &uuid, m_state.keys()) {
	unsigned int dst = 0;
	unsigned int count = CACHE_SIZE / shrink;
        Q_ASSERT(count <= CACHE_SIZE);

	// source pointers
	sample_t *smin = m_min[uuid].data();
	sample_t *smax = m_max[uuid].data();
	CacheState *sstate = m_state[uuid].data();

	// destination pointers
	sample_t *dmin = smin;
	sample_t *dmax = smax;
	CacheState *dstate = sstate;

	// loop over all entries to be shrinked
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
    invalidateCache(Q_NULLPTR, 0, CACHE_SIZE - 1);
}

//***************************************************************************
void Kwave::OverViewCache::invalidateCache(const QUuid *track_id,
                                           unsigned int first,
                                           unsigned int last)
{
    if (track_id) {
	// invalidate a single track
	Q_ASSERT(m_state.contains(*track_id));
	if (!m_state.contains(*track_id)) return;

	if (last >= CACHE_SIZE) last = CACHE_SIZE - 1;

// 	qDebug("OverViewCache[%p]::invalidateCache(%s, %u, %u)",
// 	       static_cast<void *>(this), DBG(track_id->toString()),
// 	       first, last);

	for (unsigned int pos = first; pos <= last; ++pos)
	    m_state[*track_id][pos] = Invalid;
    } else {
	// invalidate all tracks
	foreach (const QUuid &uuid, m_state.keys())
	    invalidateCache(&uuid, first, last);
    }
}

//***************************************************************************
void Kwave::OverViewCache::slotTrackInserted(const QUuid &track_id)
{
    QMutexLocker lock(&m_lock);

    // just to be sure: check scale again, maybe it was the first track
    if ((m_selection.length() / m_scale) > CACHE_SIZE)
	scaleUp();
    if ((m_selection.length() / m_scale) < (CACHE_SIZE / 4))
	scaleDown();

    QVector<CacheState> state(CACHE_SIZE);
    QVector<sample_t> min(CACHE_SIZE);
    QVector<sample_t> max(CACHE_SIZE);

    min.fill(SAMPLE_MAX);
    max.fill(SAMPLE_MIN);
    state.fill(Unused);

    m_min.insert(track_id, min);
    m_max.insert(track_id, max);
    m_state.insert(track_id, state);

    // mark the new cache content as invalid
    invalidateCache(&track_id, 0, CACHE_SIZE - 1);

    emit changed();
}

//***************************************************************************
void Kwave::OverViewCache::slotTrackDeleted(const QUuid &track_id)
{
    QMutexLocker lock(&m_lock);

    m_min.remove(track_id);
    m_max.remove(track_id);
    m_state.remove(track_id);

    emit changed();
}

//***************************************************************************
void Kwave::OverViewCache::slotInvalidated(const QUuid *track_id,
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
	width = minmax.count();

    QList<unsigned int> track_list;
    const QList<QUuid> selected_tracks = m_selection.allTracks();
    foreach (unsigned int track, m_signal.allTracks())
	if (selected_tracks.contains(m_signal.uuidOfTrack(track)))
	    track_list.append(track);
    if (track_list.isEmpty())
	return 0;

    Kwave::MultiTrackReader src(
	Kwave::SinglePassForward,
	m_signal, track_list, first, last
    );
    Q_ASSERT(m_min.count() == m_max.count());
    Q_ASSERT(m_min.count() == m_state.count());

    if ((length / m_scale < 2) || src.isEmpty() || !m_state.count())
	return 0; // empty ?

    // loop over all min/max buffers and make their content valid
    for (int index = 0; index < track_list.count(); ++index) {
	unsigned int count = qBound<unsigned int>(
	    1, Kwave::toUint(length / m_scale), CACHE_SIZE);

	QUuid uuid = m_signal.uuidOfTrack(track_list[index]);
	if (uuid.isNull()) continue; // track has just been deleted

	// check: maybe slotTrackInserted has not yet been called
	//        or slotTrackDeleted has just been called
	if (!m_state.keys().contains(uuid))
	    continue;

	sample_t *min = m_min[uuid].data();
	sample_t *max = m_max[uuid].data();
	CacheState *state = m_state[uuid].data();
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
	    foreach (const QUuid &uuid, m_state.keys()) {
		sample_t *min = m_min[uuid].data();
		sample_t *max = m_max[uuid].data();
		const CacheState *state = m_state[uuid].constData();
		Q_ASSERT(state);
		if (!state) continue;
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
