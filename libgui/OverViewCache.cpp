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

#include <QMutableListIterator>
#include <QColor>
#include <QPainter>

#include "libkwave/MultiTrackReader.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SignalManager.h"
#include "libkwave/Track.h"

#include "OverViewCache.h"

#define CACHE_SIZE 8192           /* number of cache entries */

//***************************************************************************
OverViewCache::OverViewCache(SignalManager &signal, unsigned int src_offset,
                             unsigned int src_length,
                             const QList<unsigned int> *src_tracks)
    :m_signal(signal), m_min(), m_max(), m_state(), m_count(0), m_scale(1),
     m_lock(QMutex::Recursive), m_src_offset(src_offset),
     m_src_length(src_length), m_src_tracks(), m_src_deleted()
{
    // connect to the signal manager
    SignalManager *sig = &signal;
    Q_ASSERT(sig);
    connect(sig, SIGNAL(sigTrackInserted(unsigned int, Track *)),
            this, SLOT(slotTrackInserted(unsigned int, Track *)));
    connect(sig, SIGNAL(sigTrackDeleted(unsigned int)),
            this, SLOT(slotTrackDeleted(unsigned int)));
    connect(sig, SIGNAL(sigSamplesDeleted(unsigned int, unsigned int,
	unsigned int)),
	this, SLOT(slotSamplesDeleted(unsigned int, unsigned int,
	unsigned int)));
    connect(sig, SIGNAL(sigSamplesInserted(unsigned int, unsigned int,
	unsigned int)),
	this, SLOT(slotSamplesInserted(unsigned int, unsigned int,
	unsigned int)));
    connect(sig, SIGNAL(sigSamplesModified(unsigned int, unsigned int,
	unsigned int)),
	this, SLOT(slotSamplesModified(unsigned int, unsigned int,
	unsigned int)));

    if (src_tracks && !src_tracks->isEmpty()) {
	// already having a list of selected tracks
	foreach (unsigned int track, *src_tracks) {
	    m_src_deleted.append(track);
	    slotTrackInserted(track, 0);
	}
    } else {
	// take over all tracks from the signal manager
	QList<unsigned int> tracks = signal.allTracks();
	foreach (unsigned int track, tracks) {
	    slotTrackInserted(track, 0);
	}
    }
}

//***************************************************************************
OverViewCache::~OverViewCache()
{
    QMutexLocker lock(&m_lock);

    m_state.clear();
    m_min.clear();
    m_max.clear();
}

//***************************************************************************
unsigned int OverViewCache::sourceLength()
{
    return (m_src_length) ? m_src_length : m_signal.length();
}

//***************************************************************************
void OverViewCache::scaleUp()
{
    QMutexLocker lock(&m_lock);

    Q_ASSERT(m_scale);
    if (!m_scale) return;

    // calculate the new scale
    const unsigned int len = sourceLength();
    unsigned int shrink = len / (m_scale * CACHE_SIZE);
    Q_ASSERT(shrink);
    while (len > CACHE_SIZE * m_scale * shrink) {
	shrink++;
    }
    Q_ASSERT(shrink > 1);
    if (shrink <= 1) return; // nothing to shrink, just ignore new scale

    // loop over all tracks
    for (int t=0; t < m_state.count(); ++t) {
	unsigned int dst = 0;
	unsigned int count = CACHE_SIZE / shrink;
        Q_ASSERT(count <= CACHE_SIZE);

	// source pointers
	sample_t *smin = m_min[t].data();
	sample_t *smax = m_max[t].data();
	CacheState *sstate = m_state[t].data();

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
void OverViewCache::scaleDown()
{
    QMutexLocker lock(&m_lock);

    const unsigned int len = sourceLength();
    unsigned int new_scale = static_cast<unsigned int>(
	rint(ceil(len/CACHE_SIZE)));
    if (!new_scale) new_scale = 1;
    if (m_scale == new_scale) return;

    m_scale = new_scale;
    for (int track = 0; track < m_state.count(); ++track) {
	invalidateCache(track, 0, len / m_scale);
    }
}

//***************************************************************************
int OverViewCache::trackIndex(unsigned int track_nr)
{
    QMutexLocker lock(&m_lock);

    if (!m_src_tracks.isEmpty() || !m_src_deleted.isEmpty()) {
	return m_src_tracks.indexOf(track_nr);
    } else {
	return track_nr;
    }
}

//***************************************************************************
void OverViewCache::invalidateCache(unsigned int track, unsigned int first,
                                    unsigned int last)
{
    QMutexLocker lock(&m_lock);

//     qDebug("OverViewCache::invalidateCache(%u, %u, %u)",track,first,last);
    if (m_state.isEmpty()) return;
    int cache_track = trackIndex(track);
    if (cache_track < 0) return;
    if (cache_track >= m_state.count()) return;

    QVector<CacheState> &state = m_state[cache_track];

    if (last >= CACHE_SIZE) last = CACHE_SIZE-1;
    unsigned int pos;
    for (pos = first; pos <= last; ++pos) {
	state[pos] = Invalid;
    }
}

//***************************************************************************
void OverViewCache::slotTrackInserted(unsigned int index, Track *)
{
    QMutexLocker lock(&m_lock);

    // just to be sure: check scale again, maybe it was the first track
    if ((sourceLength() / m_scale) > CACHE_SIZE)
	scaleUp();
    if ((sourceLength() / m_scale) < (CACHE_SIZE/4))
	scaleDown();

    // "selected tracks" mode -> adjust indices in track lists
    if (!m_src_tracks.isEmpty() || !m_src_deleted.isEmpty()) {
	QMutableListIterator<unsigned int> it_s(m_src_tracks);
	QMutableListIterator<unsigned int> it_d(m_src_deleted);
	unsigned int track;

	if (it_d.findNext(index)) {
	    // deleted selected track came back again
	    it_d.remove();
	    it_d.toFront();
	    while (it_d.hasNext())
		if ((track = it_d.next()) >= index) it_d.setValue(--track);
	    while (it_s.hasNext())
		if ((track = it_s.next()) >= index) it_s.setValue(++track);

	    Q_ASSERT(!m_src_tracks.contains(index));
	    m_src_tracks.append(index);
	    dumpTracks();
	} else {
	    // inserted new/unknown track
	    while (it_d.hasNext())
		if ((track = it_d.next()) >= index) it_d.setValue(--track);
	    while (it_s.hasNext())
		if ((track = it_s.next()) >= index) it_s.setValue(++track);
	    dumpTracks();
	    return;
	}
    }

    QVector<CacheState> state(CACHE_SIZE);
    QVector<sample_t> min(CACHE_SIZE);
    QVector<sample_t> max(CACHE_SIZE);

    min.fill(SAMPLE_MAX);
    max.fill(SAMPLE_MIN);
    state.fill(Unused);

    int cache_index = trackIndex(index);
    m_min.insert(cache_index, min);
    m_max.insert(cache_index, max);
    m_state.insert(cache_index, state);

    // mark the new cache content as invalid
    if (sourceLength()) {
	invalidateCache(index, 0, (sourceLength() / m_scale) - 1);
    } else {
	invalidateCache(index, 0, CACHE_SIZE-1);
    }

    emit changed();
}

//***************************************************************************
void OverViewCache::slotTrackDeleted(unsigned int index)
{
    QMutexLocker lock(&m_lock);

    int cache_track = trackIndex(index);
    if (cache_track >= 0) {
	m_min.removeAt(cache_track);
	m_max.removeAt(cache_track);
	m_state.removeAt(cache_track);
    }

    if (!m_src_tracks.isEmpty() || !m_src_deleted.isEmpty()) {
	// "selected tracks" mode -> adjust indices in track lists
	QMutableListIterator<unsigned int> it_s(m_src_tracks);
	QMutableListIterator<unsigned int> it_d(m_src_deleted);
	unsigned int track;

	dumpTracks();
	if (it_s.findNext(index)) {
	    // remove selected track
	    it_s.remove();
	    it_s.toFront();
	    m_src_deleted.append(index);
	}
	while (it_d.hasNext())
	    if ((track = it_d.next()) >= index) it_d.setValue(++track);
	while (it_s.hasNext())
	    if ((track = it_s.next()) > index) it_s.setValue(--track);
	dumpTracks();
    }

    emit changed();
}

//***************************************************************************
void OverViewCache::slotSamplesInserted(unsigned int track,
    unsigned int offset, unsigned int length)
{
    QMutexLocker lock(&m_lock);

    if ((sourceLength() / m_scale) > CACHE_SIZE)
        scaleUp();

    // not in our selection
    if (!m_src_tracks.isEmpty() && !(m_src_tracks.contains(track))) return;

    // right out of our range -> out of interest
    const unsigned int len = sourceLength();
    if (offset >= (m_src_offset + ((len) ? (len - 1) : 1))) return;

    // left from us -> just move our own offset right
    if (offset < m_src_offset) {
	m_src_offset += length;
	return;
    }

    // in our range -> increase length and invalidate all
    // samples from offset to end of file
    if (m_src_length) m_src_length += length;
    if ((sourceLength() / m_scale) > CACHE_SIZE) scaleUp();

    unsigned int first = (offset - m_src_offset) / m_scale;
    unsigned int last  = sourceLength() / m_scale;
    if (last != first) last--;

//    static unsigned int last_length = 0;
//    if (sourceLength() != last_length) {
//	last_length = sourceLength();
//	first = 0;
//    }
//
    invalidateCache(track, first, last);
    emit changed();
}

//***************************************************************************
void OverViewCache::slotSamplesDeleted(unsigned int track,
    unsigned int offset, unsigned int length)
{
    QMutexLocker lock(&m_lock);

    if ((sourceLength() / m_scale) < (CACHE_SIZE/4))
        scaleDown();

    if (!length) return; // nothing to do

    // not in our selection
    if (!m_src_tracks.isEmpty() && !(m_src_tracks.contains(track))) return;

    // right out of our range -> out of interest
    if (offset > (m_src_offset + sourceLength())) return;

    // completely left from us -> just move our own offset left
    if (offset+length <= m_src_offset) {
	m_src_offset -= length;
	return;
    }

    // overlapping
    if (offset < m_src_offset) {
	unsigned int overlap = (offset + length) - m_src_offset;
	m_src_offset -= length - overlap;
	length -= overlap;
	offset = m_src_offset;
    }
    if (!length) return; // nothing more to do ?

    // in our range -> invalidate all samples from offset to end of file
    if (m_src_length) {
	if (m_src_length > length)
	    m_src_length -= length;
	else
	    m_src_length = 1;
    }

    unsigned int first = offset;
    unsigned int last  = offset + length - 1;
    if (first < m_src_offset) first = m_src_offset;
    first -= m_src_offset;
    last  -= m_src_offset;
    first = static_cast<unsigned int>(floor(first / m_scale));
    last  = static_cast<unsigned int>(ceil(last / m_scale));
    invalidateCache(track, first, last);
    emit changed();
}

//***************************************************************************
void OverViewCache::slotSamplesModified(unsigned int track,
    unsigned int offset, unsigned int length)
{
    QMutexLocker lock(&m_lock);

    if (!length) return; // nothing to do

    // not in our selection
    if (!m_src_tracks.isEmpty() && !(m_src_tracks.contains(track))) return;

    // right out of our range -> out of interest
    if (offset > (m_src_offset + sourceLength())) return;

    // completely left from us -> out of interest
    if (offset+length < m_src_offset) return;

    // overlapping
    unsigned int first = offset;
    unsigned int last  = offset + length - 1;
    if (first < m_src_offset) first = m_src_offset;
    if (last > m_src_offset+sourceLength()-1)
        last = m_src_offset+sourceLength()-1;
    first -= m_src_offset;
    last  -= m_src_offset;
    first = static_cast<unsigned int>(floor(first / m_scale));
    last  = static_cast<unsigned int>(ceil(last / m_scale));
    invalidateCache(track, first, last);
    emit changed();
}

//***************************************************************************
QImage OverViewCache::getOverView(int width, int height,
                                  const QColor &fg, const QColor &bg,
                                  double gain)
{
    QMutexLocker lock(&m_lock);

    QImage bitmap(width, height, QImage::Format_ARGB32_Premultiplied);
    if (!width || !height || bitmap.isNull()) return bitmap;

    QPainter p;
    p.begin(&bitmap);
    p.fillRect(bitmap.rect(), bg);
    p.setPen(fg);

    const unsigned int length = sourceLength();
    if (!length) {
	p.end();
	return bitmap; // stay empty if no data available
    }

    QList<unsigned int> track_list;
    if (!m_src_tracks.isEmpty() || !m_src_deleted.isEmpty()) {
	for (int i=0; i < m_src_tracks.count(); ++i)
	    track_list.append(m_src_tracks[i]);
    } else {
	track_list = m_signal.allTracks();
    }

    MultiTrackReader src(Kwave::SinglePassForward,
	m_signal, track_list, m_src_offset,
	m_src_offset+length-1);
    Q_ASSERT(m_min.count() == m_max.count());
    Q_ASSERT(m_min.count() == m_state.count());

    // abort if the track count has recently changed
    if (m_state.count() != static_cast<int>(src.tracks())) {
	qWarning("OverViewCache::getOverView(): track count has changed");
	p.end();
	return bitmap;
    }

    // loop over all min/max buffers and make their content valid
    for (int t = 0; (t < m_state.count()) && !src.isEmpty(); ++t) {
	unsigned int count = length / m_scale;
	if (count > CACHE_SIZE) count = 0;

	sample_t *min = m_min[t].data();
	sample_t *max = m_max[t].data();
	CacheState *state = m_state[t].data();
	SampleReader *reader = src[t];
	Q_ASSERT(reader);
	if (!reader) continue;

	for (unsigned int ofs = 0; ofs < count; ++ofs) {
	    if (state[ofs] == Valid)  continue;
	    if (state[ofs] == Unused) continue;

	    // get min/max
	    const unsigned int first = m_src_offset + (ofs * m_scale);
	    const unsigned int last  = first + m_scale - 1;
	    reader->minMax(first, last, min[ofs], max[ofs]);
	    state[ofs] = Valid;
	}
    }

    if ((width < 2) || (height < 2) || (length/m_scale < 2)) {
	p.end();
	return bitmap; // empty ?
    }

    // loop over all min/max buffers
    for (int x = 0; (x < width) && (m_state.count()) && !src.isEmpty(); ++x) {
	unsigned int count = length / m_scale;
	if (count > CACHE_SIZE) count = 1;

	// get the corresponding cache index
	unsigned int index = ((count-1) * x) / (width-1);
	unsigned int last_index  = ((count-1) * (x+1)) / (width-1);
	Q_ASSERT(index < CACHE_SIZE);
	if (index >= CACHE_SIZE) index = CACHE_SIZE-1;
	if (last_index > index) last_index--;
	if (last_index >= CACHE_SIZE) last_index = CACHE_SIZE-1;

	// loop over all cache indices
	sample_t minimum = SAMPLE_MAX;
	sample_t maximum = SAMPLE_MIN;
	for (; index <= last_index; ++index) {
	    // loop over all tracks
	    Q_ASSERT(m_min.count() == m_state.count());
	    Q_ASSERT(m_max.count() == m_state.count());
	    for (int t = 0; t < m_state.count(); ++t) {
		Q_ASSERT(t < m_min.count());
		Q_ASSERT(t < m_max.count());
		sample_t *min = m_min[t].data();
		sample_t *max = m_max[t].data();
		CacheState *state = m_state[t].data();
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

	// update the bitmap
	const int middle = (height >> 1);
	const double scale = static_cast<double>(middle) /
	                     static_cast<double>(SAMPLE_MAX);
	double min = minimum * scale;
	double max = maximum * scale;
	if (gain != 1.0) {
	    min *= gain;
	    max *= gain;
	    if (min < -middle) min = -middle;
	    if (min > +middle) min = +middle;
	    if (max < -middle) max = -middle;
	    if (max > +middle) max = +middle;
	}
	p.drawLine(x, middle - static_cast<int>(min),
	           x, middle - static_cast<int>(max));
    }

    p.end();
    return bitmap;
}

//***************************************************************************
void OverViewCache::dumpTracks()
{
    QString list = "OverViewCache - selected:";
    foreach (unsigned int track, m_src_tracks)
	list += " " + list.number(track);
    list += " --- deleted:";
    foreach (unsigned int track, m_src_deleted)
	list += " " + list.number(track);
    qDebug("%s", list.toLocal8Bit().data());
}

//***************************************************************************
#include "OverViewCache.moc"
//***************************************************************************
//***************************************************************************
