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

#include <qpainter.h>

#include "mt/Mutex.h"
#include "mt/MutexGuard.h"

#include "libkwave/MultiTrackReader.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/Track.h"

#include "kwave/SignalManager.h"

#include "OverViewCache.h"

#define CACHE_SIZE 8192           /* number of cache entries */

//*****************************************************************************
OverViewCache::OverViewCache(SignalManager &signal)
    :m_signal(signal), m_min(), m_max(), m_state(),
     m_count(0), m_scale(1), m_lock()
{
    // connect to the signal manager
    SignalManager *sig = &signal;
    ASSERT(sig);
    connect(sig, SIGNAL(sigTrackInserted(unsigned int, Track &)),
            this, SLOT(slotTrackInserted(unsigned int, Track &)));
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

}

//*****************************************************************************
OverViewCache::~OverViewCache()
{
}

//*****************************************************************************
void OverViewCache::scaleUp()
{
    // calculate the new scale
    const unsigned int len = m_signal.length();
    unsigned int new_scale = static_cast<unsigned int>(
	rint(ceil((double)len/(double)CACHE_SIZE)));
    if (!new_scale) new_scale = 1;

    // get the shrink factor: "shrink" entries -> "1" entry
    const unsigned int shrink = static_cast<unsigned int>(
	rint(ceil(new_scale / m_scale)));

    // loop over all tracks
    for (unsigned int t=0; t < m_state.count(); ++t) {
	unsigned int dst = 0;
	unsigned int count = len / m_scale;
	if (count > CACHE_SIZE) count = 0;
	
	// source pointers
	char *smin = m_min.at(t)->data();
	char *smax = m_max.at(t)->data();
	CacheState *sstate = m_state.at(t)->data();
	
	// destination pointers
	char *dmin = smin;
	char *dmax = smax;
	CacheState *dstate = sstate;
	
	// loop over all entries to be shrinked
	while (dst < count) {
	    char min = +127;
	    char max = -127;
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

    m_scale = new_scale;
}

//*****************************************************************************
void OverViewCache::scaleDown()
{
    const unsigned int len = m_signal.length();
    unsigned int new_scale = static_cast<unsigned int>(
	rint(ceil(len/CACHE_SIZE)));
    if (!new_scale) new_scale = 1;
    if (m_scale == new_scale) return;

    m_scale = new_scale;
    for (unsigned int track=0; track < m_state.count(); ++track) {
	invalidateCache(track, 0, len / m_scale);
    }
}

//*****************************************************************************
void OverViewCache::slotTrackInserted(unsigned int index, Track &track)
{
    MutexGuard lock(m_lock);

    // just to be sure: check scale again, maybe it was the first track
    if ((m_signal.length() / m_scale) > CACHE_SIZE)
	scaleUp();
    if ((m_signal.length() / m_scale) < (CACHE_SIZE/4))
	scaleDown();

    QArray<CacheState> *state = new QArray<CacheState>(CACHE_SIZE);
    QByteArray *min = new QByteArray(CACHE_SIZE);
    QByteArray *max = new QByteArray(CACHE_SIZE);

    if (!state || !min || !max) {
	ASSERT(state);
	ASSERT(min);
	ASSERT(max);
	if (state) delete state;
	if (min) delete min;
	if (max) delete max;
	return;
    }

    min->fill(+127);
    max->fill(-127);
    state->fill(Unused);

    m_min.insert(index, min);
    m_max.insert(index, max);
    m_state.insert(index, state);

    // mark the new cache content as invalid
    invalidateCache(index, 0, (track.length() / m_scale) - 1);

    emit changed();
}

//*****************************************************************************
void OverViewCache::invalidateCache(unsigned int track, unsigned int first,
                                    unsigned int last)
{
    QArray<CacheState> *state = m_state.at(track);
    ASSERT(state);
    if (!state) return;

    ASSERT(last < CACHE_SIZE);
    if (last >= CACHE_SIZE) last = CACHE_SIZE-1;

    unsigned int pos;
    for (pos = first; pos <= last; ++pos) {
	(*state)[pos] = Invalid;
    }
}

//*****************************************************************************
void OverViewCache::slotTrackDeleted(unsigned int index)
{
    MutexGuard lock(m_lock);

    m_min.remove(index);
    m_max.remove(index);
    m_state.remove(index);

    if (m_state.isEmpty()) m_scale = 1;
    emit changed();
}

//*****************************************************************************
void OverViewCache::slotSamplesInserted(unsigned int track,
    unsigned int offset, unsigned int /*length*/)
{
    MutexGuard lock(m_lock);

    if ((m_signal.length() / m_scale) > CACHE_SIZE)
        scaleUp();

    // invalidate all samples from offset to end of file
    unsigned int first = offset / m_scale;
    unsigned int last  = m_signal.length() / m_scale;
    invalidateCache(track, first, last);
    emit changed();
}

//*****************************************************************************
void OverViewCache::slotSamplesDeleted(unsigned int track,
    unsigned int offset, unsigned int /* length */)
{
    MutexGuard lock(m_lock);

    if ((m_signal.length() / m_scale) < (CACHE_SIZE/4))
        scaleDown();

    // invalidate all samples from offset to end of file
    unsigned int first = offset / m_scale;
    unsigned int last  = m_signal.length() / m_scale;
    invalidateCache(track, first, last);
    emit changed();
}

//*****************************************************************************
void OverViewCache::slotSamplesModified(unsigned int track,
    unsigned int offset, unsigned int length)
{
    MutexGuard lock(m_lock);

    unsigned int first = offset / m_scale;
    unsigned int last  = ((offset+length-1) / m_scale) + 1;
    invalidateCache(track, first, last);
    emit changed();
}

//*****************************************************************************
QBitmap OverViewCache::getOverView(int width, int height)
{
    QBitmap bitmap(width, height);

    const unsigned int length = m_signal.length();
    MultiTrackReader src;
    m_signal.openMultiTrackReader(src, m_signal.allTracks(),
                                  0, length-1);

    // loop over all min/max buffers and make their content valid
    for (unsigned int t=0; t < m_state.count(); ++t) {
	unsigned int count = length / m_scale;
	if (count > CACHE_SIZE) count = 0;
	
	char *min = m_min.at(t)->data();
	char *max = m_max.at(t)->data();
	CacheState *state = m_state.at(t)->data();
	SampleReader *reader = src[t];
	
	for (unsigned int ofs=0; ofs < count; ++ofs) {
	    if (state[ofs] == Valid)  continue;
	    if (state[ofs] == Unused) continue;
	
	    sample_t min_sample = SAMPLE_MAX;
	    sample_t max_sample = SAMPLE_MIN;
	    unsigned int first = ofs*m_scale;
	    unsigned int count = m_scale;
	
	    reader->seek(first);
	    while (count--) {
		sample_t sample;
		(*reader) >> sample;
		if (sample > max_sample) max_sample = sample;
		if (sample < min_sample) min_sample = sample;
	    }
	
	    min[ofs] = min_sample >> (SAMPLE_BITS - 8);
	    max[ofs] = max_sample >> (SAMPLE_BITS - 8);
	    state[ofs] = Valid;
	}
    }
    if (!width || !height) return bitmap; // empty ?

    bitmap.fill(color0);
    QPainter p;
    p.begin(&bitmap);

    // loop over all min/max buffers
    for (int x=0; (x < width) && (m_state.count()); ++x) {
	unsigned int count = length / m_scale;
	if (count > CACHE_SIZE) count = 1;
	
	// get the corresponding cache index
	unsigned int index = ((count-1) * x) / (width-1);
	unsigned int last_index  = ((count-1) * (x+1)) / (width-1);
	if (last_index > index) last_index--;
	
	// loop over all cache indices
	int minimum = +127;
	int maximum = -127;
	for (; index <= last_index; ++index) {
	    // loop over all tracks
	    for (unsigned int t=0; t < m_state.count(); ++t) {
		char *min = m_min.at(t)->data();
		char *max = m_max.at(t)->data();
		CacheState *state = m_state.at(t)->data();
		if (state[index] != Valid) continue;
		
		if (min[index] < minimum) minimum = min[index];
		if (max[index] > maximum) maximum = max[index];
	    }
	}
	
	// update the bitmap
	p.setPen(color0);
	p.drawLine(x, 0, x, height-1);
	p.setPen(color1);
	
	const int middle = (height>>1);
	p.drawLine(x, middle + (minimum * height)/254,
	           x, middle + (maximum * height)/254);
    }

    p.end();
    return bitmap;
}

//*****************************************************************************
//*****************************************************************************
