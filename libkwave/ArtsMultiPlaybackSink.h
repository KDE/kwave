/***************************************************************************
    ArtsMultiPlaybackSink.h  -  multi-track aRts compatible sink for playback
                             -------------------
    begin                : Mon Apr 28 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
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

#ifndef _ARTS_MULTI_PLAYBACK_SINK_H_
#define _ARTS_MULTI_PLAYBACK_SINK_H_

#include "config.h"
#ifdef HAVE_ARTS_SUPPORT

#include <math.h>
#include <qbitarray.h>
#include <qmemarray.h>
#include <qptrvector.h>
#include <arts/artsflow.h>

#include "libkwave/ArtsMultiSink.h"
#include "libkwave/ArtsPlaybackSink.h"
#include "libkwave/ArtsPlaybackSink_impl.h"
#include "libkwave/Sample.h"

class Arts::Object;
class PlayBackDevice;

class ArtsMultiPlaybackSink: public ArtsMultiSink
{
public:

    /**
     * Constructor. Creates all i/o objects and holds them in
     * an internal vector. If the creation of an object failed,
     * the initialization will be aborted and the count of
     * objects will be reduced.
     */
    ArtsMultiPlaybackSink(unsigned int tracks, PlayBackDevice *device);

    /**
     * Destructor.
     * Deletes all i/o objects in reverse order
     */
    virtual ~ArtsMultiPlaybackSink();

    /**
     * Returns one of the aRts i/o objects.
     * @param i index of the track [0..count-1]
     * @return pointer to the object or 0 if index is out of range
     */
    virtual Arts::Object *operator[](unsigned int i);

    /** Calls start() for each aRts i/o object. */
    virtual void start() {
	for (unsigned int t=0; t < m_tracks; ++t) m_sinks[t]->start();
    };

    /** Calls stop() for each aRts i/o object. */
    virtual void stop() {
	for (unsigned int t=0; t < m_tracks; ++t) m_sinks[t]->stop();
    };

    /**
     * does the data processing
     * @see ArtsMultiSink::goOn()
     */
    virtual void goOn();

    /**
     * Normally returns false, as playback streams have no end. However,
     * if there was an error while opening the stream or someone wants
     * to cancel the playback, this will return true to signal the end
     * of playback.
     */
    virtual bool done();

protected:
    friend class ArtsPlaybackSink_impl;

    /**
     * Collects data from a track and does the playback when enough
     * data is available.
     */
    void playback(int track, float *buffer, unsigned long samples);

protected:

    /** device used for playback */
    PlayBackDevice *m_device;

    /**
     * number of tracks, should be the same as the number of elements
     * in m_sinks
     */
    unsigned int m_tracks;

    /**
     * List of ArtsPlayback sinks
     */
    QPtrVector<ArtsPlaybackSink> m_sinks;

    /** used for signalling a cancel or abort, can be queried with done() */
    bool m_done;

    /** list of input buffers */
    QPtrVector< QMemArray<float> > m_in_buffer;

    /** "filled"-flags for input buffers */
    QBitArray m_in_buffer_filled;

    /** output buffer for all samples */
    QMemArray<sample_t> m_out_buffer;

};

#else /* HAVE_ARTS_SUPPORT */
#warning aRts support is disabled
#endif /* HAVE_ARTS_SUPPORT */

#endif /* _ARTS_MULTI_PLAYBACK_SINK_H_ */
