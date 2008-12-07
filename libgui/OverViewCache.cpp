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
	signed char *smin = m_min[t].data();
	signed char *smax = m_max[t].data();
	CacheState *sstate = m_state[t].data();

	// destination pointers
	signed char *dmin = smin;
	signed char *dmax = smax;
	CacheState *dstate = sstate;

	// loop over all entries to be shrinked
	while (dst < count) {
	    signed char min = +127;
	    signed char max = -127;
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
    const unsigned int len = sourceLength();
    unsigned int new_scale = static_cast<unsigned int>(
	rint(ceil(len/CACHE_SIZE)));
    if (!new_scale) new_scale = 1;
    if (m_scale == new_scale) return;

    m_scale = new_scale;
    for (int track=0; track < m_state.count(); ++track) {
	invalidateCache(track, 0, len / m_scale);
    }
}

//***************************************************************************
int OverViewCache::trackIndex(unsigned int track_nr)
{
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
    QVector<signed char> min(CACHE_SIZE);
    QVector<signed char> max(CACHE_SIZE);

    min.fill(+127);
    max.fill(-127);
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
	unsigned int overlap = (offset+length) - m_src_offset;
	m_src_offset -= length-overlap;
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
    unsigned int first = (offset-m_src_offset) / m_scale;
    unsigned int last  = sourceLength() / m_scale;
    if (last != first) last--;
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
QBitmap OverViewCache::getOverView(int width, int height)
{
    QBitmap bitmap(width, height);
    bitmap.fill(Qt::color0);

    const unsigned int length = sourceLength();
    if (!length) return bitmap; // stay empty if no data available

    QList<unsigned int> track_list;
    if (!m_src_tracks.isEmpty() || !m_src_deleted.isEmpty()) {
	for (int i=0; i < m_src_tracks.count(); ++i)
	    track_list.append(m_src_tracks[i]);
    } else {
	track_list = m_signal.allTracks();
    }
    MultiTrackReader src(m_signal, track_list, m_src_offset,
	m_src_offset+length-1);
    Kwave::SampleArray buf;

    // loop over all min/max buffers and make their content valid
    Q_ASSERT(m_state.count() == static_cast<int>(src.tracks()));
    for (int t=0; (t < m_state.count()) && !src.isEmpty(); ++t) {
	unsigned int count = length / m_scale;
	if (count > CACHE_SIZE) count = 0;

	signed char *min = m_min[t].data();
	signed char *max = m_max[t].data();
	CacheState *state = m_state[t].data();
	SampleReader *reader = src[t];

	for (unsigned int ofs=0; ofs < count; ++ofs) {
	    if (state[ofs] == Valid)  continue;
	    if (state[ofs] == Unused) continue;

	    sample_t min_sample = SAMPLE_MAX;
	    sample_t max_sample = SAMPLE_MIN;
	    unsigned int first = ofs*m_scale;
	    buf.resize(m_scale);
	    unsigned int count = buf.size();
	    Q_ASSERT(count >= m_scale);
	    if (count < m_scale) break; // out of memory

	    reader->seek(m_src_offset+first);
	    count = reader->read(buf, 0, m_scale);
	    while (count--) {
		sample_t sample = buf[count];
		if (sample > max_sample) max_sample = sample;
		if (sample < min_sample) min_sample = sample;
	    }

	    min[ofs] = min_sample >> (SAMPLE_BITS - 8);
	    max[ofs] = max_sample >> (SAMPLE_BITS - 8);
	    state[ofs] = Valid;
	}
    }
    if ((width < 2) || (height < 2) || (length/m_scale < 2))
	return bitmap; // empty ?

    QPainter p;
    p.begin(&bitmap);
    p.setPen(Qt::color1);

    // loop over all min/max buffers
    for (int x=0; (x < width) && (m_state.count()) && !src.isEmpty(); ++x) {
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
	int minimum = +127;
	int maximum = -127;
	for (; index <= last_index; ++index) {
	    // loop over all tracks
	    for (int t=0; t < m_state.count(); ++t) {
		signed char *min = m_min[t].data();
		signed char *max = m_max[t].data();
		CacheState *state = m_state[t].data();
		Q_ASSERT(state);
		if (!state) continue;
		if (state[index] != Valid) continue;

		if (min[index] < minimum) minimum = min[index];
		if (max[index] > maximum) maximum = max[index];
	    }
	}

	// update the bitmap
	const int middle = (height>>1);
	p.drawLine(x, middle - (minimum * height)/254,
	           x, middle - (maximum * height)/254);
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
