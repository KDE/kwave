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
#include <qbitmap.h>
#include <qcstring.h>
#include <qlist.h>
#include <qobject.h>

#include "mt/Mutex.h"

class QBitmap;
class SignalManager;
class Track;

/**
 * @class OverViewCache
 * Fixed-size cache for multi-track sample data. Automatically updates
 * itself if data has been changed, inserted or deleted.
 * Optimized for speed!
 */
class OverViewCache: public QObject
{
    Q_OBJECT
public:

    OverViewCache(SignalManager &signal);

    /** Destructor */
    virtual ~OverViewCache();

    /**
     * Renders an overview into a bitmap. The bitmap will be black & white
     * only and can be used as a brush.
     * @param width the width of the bitmap in pixels
     * @param height the width of the bitmap in pixels
     * @return the created bitmap
     */
    virtual QBitmap getOverView(int width, int height);

signals:

    /** emitted whenever the cache content has changed */
    void changed();

protected slots:

    /**
     * Connected to the signal's sigTrackInserted.
     * @param track index of the inserted track
     * @see Signal::sigTrackInserted
     * @internal
     */
    void slotTrackInserted(unsigned int index, Track &track);

    /**
     * Connected to the signal's sigTrackInserted.
     * @param track index of the inserted track
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
    void slotSamplesInserted(unsigned int track, unsigned int offset,
                             unsigned int length);

    /**
     * Connected to the signal's sigSamplesDeleted.
     * @param track index of the source track [0...tracks-1]
     * @param offset position from which the data was removed
     * @param length number of samples deleted
     * @see Signal::sigSamplesDeleted
     * @internal
     */
    void slotSamplesDeleted(unsigned int track, unsigned int offset,
                            unsigned int length);

    /**
     * Connected to the signal's sigSamplesModified
     * @param track index of the source track [0...tracks-1]
     * @param offset position from which the data was modified
     * @param length number of samples modified
     * @see Signal::sigSamplesModified
     * @internal
     */
    void slotSamplesModified(unsigned int track, unsigned int offset,
                             unsigned int length);

protected:

    /** State of a cache entry */
    typedef enum {Invalid, Fuzzy, Valid, Unused} CacheState;

private:

    /**
     * Compresses the cache to hold more samples per entry.
     */
    void scaleUp();

    /**
     * Expands the cache to hold less samples per entry. As this
     * process looses accuracy, the cache must be "polished" in
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
    QList<QByteArray> m_min;

    /** list of maximum value arrays, one array per track */
    QList<QByteArray> m_max;

    /** bitmask for "validity" of the min/max values */
    QList< QArray <CacheState> > m_state;

    /** number of min/max values */
    unsigned int m_count;

    /** number of samples per cache entry */
    unsigned int m_scale;

    /** mutex for threadsafe access to the cache */
    Mutex m_lock;

};

#endif // _OVER_VIEW_CACHE_H_
