/***************************************************************************
        OverViewCache.h  -  fast cache for sample data overview
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

#ifndef _OVER_VIEW_CACHE_H_
#define _OVER_VIEW_CACHE_H_

#include "config.h"

#include <QImage>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QVector>

#include <kdemacros.h>

#include "libkwave/Sample.h"

class QColor;
class SignalManager;
namespace Kwave { class Track; }

/**
 * @class OverViewCache
 * Fixed-size cache for multi-track sample data. Automatically updates
 * itself if data has been changed, inserted or deleted.
 * Optimized for speed!
 */
class KDE_EXPORT OverViewCache: public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor
     * @param signal reference to a SignalManager with the source
     * @param src_offset first sample index in the source.
     *                   optional, default=0
     * @param src_length number of samples in the source.
     *                   optional, default=0 (whole signal)
     * @param src_tracks list of selected source tracks
     *                   optional, default=0 (whole signal)
     */
    OverViewCache(SignalManager &signal, sample_index_t src_offset = 0,
                  sample_index_t src_length = 0,
                  const QList<unsigned int> *src_tracks = 0);

    /** Destructor */
    virtual ~OverViewCache();

    /**
     * Renders an overview into a QImage.
     * @param width the width of the bitmap in pixels
     * @param height the width of the bitmap in pixels
     * @param fg foreground color
     * @param bg background color
     * @param gain additional y scaling factor (optional, default = 1.0)
     * @return the created bitmap
     */
    virtual QImage getOverView(int width, int height,
                               const QColor &fg, const QColor &bg,
                               double gain = 1.0);

signals:

    /** emitted whenever the cache content has changed */
    void changed();

protected slots:

    /**
     * Connected to the signal's sigTrackInserted.
     * @param index the index [0...tracks()-1] of the inserted track
     * @see Signal::sigTrackInserted
     * @internal
     */
    void slotTrackInserted(unsigned int index, Kwave::Track *);

    /**
     * Connected to the signal's sigTrackInserted.
     * @param index the index of the inserted track
     * @see Signal::sigTrackDeleted
     * @internal
     */
    void slotTrackDeleted(unsigned int index);

    /**
     * Connected to the signal's sigSamplesInserted.
     * @param track index of the source track [0...tracks-1]
     * @param offset position from which the data was inserted
     * @param length number of samples inserted
     * @see Signal::sigSamplesInserted
     * @internal
     */
    void slotSamplesInserted(unsigned int track, sample_index_t offset,
                             sample_index_t length);

    /**
     * Connected to the signal's sigSamplesDeleted.
     * @param track index of the source track [0...tracks-1]
     * @param offset position from which the data was removed
     * @param length number of samples deleted
     * @see Signal::sigSamplesDeleted
     * @internal
     */
    void slotSamplesDeleted(unsigned int track, sample_index_t offset,
                            sample_index_t length);

    /**
     * Connected to the signal's sigSamplesModified
     * @param track index of the source track [0...tracks-1]
     * @param offset position from which the data was modified
     * @param length number of samples modified
     * @see Signal::sigSamplesModified
     * @internal
     */
    void slotSamplesModified(unsigned int track, sample_index_t offset,
                             sample_index_t length);

protected:

    /** State of a cache entry */
    typedef enum {
	Invalid = 0,
	Fuzzy,
	Valid,
	Unused
    } CacheState;

private:

    /**
     * Shows a short list of selected / deleted selected tracks.
     * @internal for debugging only
     */
    void dumpTracks();

    /**
     * Translates a track number from the original signal into an
     * internal track index. If all tracks are selected, this is
     * equal to the signal's track number, in "selected-tracks" mode
     * it is the internal index within the list of selected tracks.
     * @param track_nr index of a track of the signal
     * @return internal index or -1 if not in selection
     */
    int trackIndex(unsigned int track_nr);

    /** Returns the number of selected samples of the source */
    sample_index_t sourceLength();

    /**
     * Compresses the cache to hold more samples per entry.
     */
    void scaleUp();

    /**
     * Expands the cache to hold less samples per entry. As this
     * process loses accuracy, the cache must be "polished" in
     * a second step.
     */
    void scaleDown();

    /**
     * Marks a range of cache entries of a track as invalid
     * @param track index of the track to invalidate
     * @param first index of the first entry
     * @param last index of the last entry (will be truncated to CACHE_SIZE-1)
     */
    void invalidateCache(unsigned int track, unsigned int first,
                         unsigned int last);

    /**
     * Refreshes all modified parts of the bitmap
     */
    void refreshBitmap();

private:

    /** signal with the data to be shown */
    SignalManager &m_signal;

    /** list of minimum value arrays, one array per track */
    QList< QVector <sample_t> > m_min;

    /** list of maximum value arrays, one array per track */
    QList< QVector <sample_t> > m_max;

    /** bitmask for "validity" of the min/max values */
    QList< QVector <CacheState> > m_state;

    /** number of min/max values */
    unsigned int m_count;

    /** number of samples per cache entry */
    unsigned int m_scale;

    /** mutex for threadsafe access to the cache */
    QMutex m_lock;

    /** first sample index in the source */
    sample_index_t m_src_offset;

    /** length of the source in samples, or zero for "whole signal" */
    sample_index_t m_src_length;

    /** list of selected source tracks */
    QList<unsigned int> m_src_tracks;

    /** list of selected and deleted source tracks */
    QList<unsigned int> m_src_deleted;

};

#endif // _OVER_VIEW_CACHE_H_
