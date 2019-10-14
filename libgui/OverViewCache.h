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

#ifndef OVER_VIEW_CACHE_H
#define OVER_VIEW_CACHE_H

#include "config.h"

#include <QtGlobal>
#include <QHash>
#include <QImage>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QUuid>
#include <QVector>

#include "libkwave/Sample.h"

#include "libgui/SelectionTracker.h"

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
    class Q_DECL_EXPORT OverViewCache: public QObject
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
	              sample_index_t src_offset,
	              sample_index_t src_length,
	              const QVector<unsigned int> *src_tracks);

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
	 * Connected to the selection tracker's sigTrackInserted.
	 * @param track_id unique ID of the track
	 * @see SelectionTracker::sigTrackInserted
	 */
	void slotTrackInserted(const QUuid &track_id);

	/**
	 * Connected to the selection tracker's sigTrackInserted.
	 * @param track_id unique ID of the track
	 * @see SelectionTracker::sigTrackDeleted
	 */
	void slotTrackDeleted(const QUuid &track_id);

	/**
	 * Connected to the selection tracker's sigLengthChanged.
	 * @param new_length new length of the selection in samples
	 * @see SelectionTracker::sigLengthChanged
	 */
	void slotLengthChanged(sample_index_t new_length);

	/**
	 * Connected to the selection tracker's sigInvalidated.
	 * @param track_id UUID of the track or null for "all tracks"
	 * @param first index of the first invalidated sample
	 * @param last index of the last invalidated sample
	 */
	void slotInvalidated(const QUuid *track_id,
	                     sample_index_t first,
	                     sample_index_t last);

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
	 * @param uuid ID of the track to invalidate or null for "all tracks"
	 * @param first index of the first entry
	 * @param last index of the last entry (will be truncated to CACHE_SIZE-1)
	 */
	void invalidateCache(const QUuid *uuid,
	                     unsigned int first,
	                     unsigned int last);

    private:

	/** signal with the data to be shown */
	Kwave::SignalManager &m_signal;

	/** selection tracker */
	Kwave::SelectionTracker m_selection;

	/** list of minimum value arrays, one array per track */
	QHash<QUuid, QVector <sample_t> > m_min;

	/** list of maximum value arrays, one array per track */
	QHash<QUuid, QVector <sample_t> > m_max;

	/** bitmask for "validity" of the min/max values */
	QHash<QUuid, QVector <CacheState> > m_state;

	/** list of min/max pairs, cached internally for getOverView */
	MinMaxArray m_minmax;

	/** number of samples per cache entry */
	quint64 m_scale;

	/** mutex for threadsafe access to the cache */
	QMutex m_lock;

    };
}

#endif // _OVER_VIEW_CACHE_H_

//***************************************************************************
//***************************************************************************
