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

#include <QtGui/QImage>
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QMutex>
#include <QtCore/QObject>
#include <QtCore/QUuid>
#include <QtCore/QVector>

#include <kdemacros.h>

#include "libkwave/Sample.h"

class QColor;

namespace Kwave
{

    class SignalManager;
    class Track;

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

	typedef struct {
	    sample_t min;
	    sample_t max;
	} MinMax;

	typedef QVector<MinMax> MinMaxArray;

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
	OverViewCache(Kwave::SignalManager &signal,
	              sample_index_t src_offset = 0,
	              sample_index_t src_length = 0,
	              const QList<unsigned int> *src_tracks = 0);

	/** Destructor */
	virtual ~OverViewCache();

	/**
	 * Get an array with min/max sample values
	 * @param width number of min/max entries to fill
	 * @param minmax array that receives the minmax values
	 * @return number of successfully filled min/max entries
	 */
	int getMinMax(int width, MinMaxArray &minmax);

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
	 * @param track pointer to the track instance
	 * @see Signal::sigTrackInserted
	 * @internal
	 */
	void slotTrackInserted(unsigned int index, Kwave::Track *track);

	/**
	 * Connected to the signal's sigTrackInserted.
	 * @param index the index of the inserted track
	 * @param track pointer to the track instance
	 * @see Signal::sigTrackDeleted
	 * @internal
	 */
	void slotTrackDeleted(unsigned int index, Kwave::Track *track);

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

	/** Returns the number of selected samples of the source */
	sample_index_t sourceLength();

	/**
	 * Returns true when there was a list of selected tracks
	 */
	bool haveTrackSelection() const {
	    return (!m_src_tracks.isEmpty() || !m_src_deleted.isEmpty());
	}

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
	 * @param uuid ID of the track to invalidate
	 * @param first index of the first entry
	 * @param last index of the last entry (will be truncated to CACHE_SIZE-1)
	 */
	void invalidateCache(const QUuid &uuid,
	                     unsigned int first,
	                     unsigned int last);

    private:

	/** signal with the data to be shown */
	Kwave::SignalManager &m_signal;

	/** list of minimum value arrays, one array per track */
	QHash<QUuid, QVector <sample_t> > m_min;

	/** list of maximum value arrays, one array per track */
	QHash<QUuid, QVector <sample_t> > m_max;

	/** bitmask for "validity" of the min/max values */
	QHash<QUuid, QVector <CacheState> > m_state;

	/** list of min/max pairs, cached internally for getOverView */
	MinMaxArray m_minmax;

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
	QList<QUuid> m_src_tracks;

	/** list of selected and deleted source tracks */
	QList<QUuid> m_src_deleted;

    };
}

#endif // _OVER_VIEW_CACHE_H_

//***************************************************************************
//***************************************************************************
