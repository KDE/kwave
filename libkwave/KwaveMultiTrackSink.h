/*************************************************************************
    KwaveMultiTrackSink.h  -  template for multi-track sinks
			    -------------------
    begin                : Sat Oct 20 2007
    copyright            : (C) 2007 by Thomas Eschenbacher
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

#ifndef _KWAVE_MULTI_TRACK_SINK_H_
#define _KWAVE_MULTI_TRACK_SINK_H_

#include "config.h"
#include <qobject.h>
#include <qptrvector.h>

#include "libkwave/KwaveSampleSink.h"

namespace Kwave {
    template <class SINK>
    class MultiTrackSink: public Kwave::SampleSink,
                          private QPtrVector<SINK>
    {
    public:
	/**
	 * Constructor
	 *
	 * @param parent a parent object, passed to QObject (optional)
	 * @param name a free name, for identifying this object,
	 *             will be passed to the QObject (optional)
	 */
	MultiTrackSink(unsigned int tracks,
		       QObject *parent = 0,
		       const char *name = 0)
	    :Kwave::SampleSink(parent, name),
	    QPtrVector<SINK>()
	{
	    QPtrVector<SINK>::setAutoDelete(true);
	    QPtrVector<SINK>::resize(tracks);
	    Q_ASSERT(QPtrVector<SINK>::size() == tracks);
	};

	/** Destructor */
	virtual ~MultiTrackSink()
	{
	    QPtrVector<SINK>::setAutoDelete(true);
	    QPtrVector<SINK>::clear();
	};

	/** Returns true when all sinks are done */
	virtual bool done()
	{
	    for (unsigned int track=0; track < tracks(); ++track) {
		Kwave::SampleSink *s = (*this)[track];
		if (s && !s->done()) return false;
	    }
	    return true;
	};

	/**
	 * Returns the number of tracks that the sink provides
	 * @return number of tracks
	 */
	virtual unsigned int tracks() const
	{
	    return QPtrVector<SINK>::size();
	};

	/**
	 * Returns the sink that corresponds to one specific track
	 * if the object has multiple tracks. For single-track objects
	 * it returns "this" for the first index and 0 for all others
	 */
	inline virtual SINK *at(unsigned int track) const {
	    return QPtrVector<SINK>::at(track);
	};

	/** @see the Kwave::MultiTrackSink.at()... */
	inline virtual SINK * operator [] (unsigned int track) {
	    return at(track);
	};

	/**
	 * Insert a new track with a sink.
	 *
	 * @param track index of the track [0...N-1]
	 * @param sink pointer to a Kwave::SampleSink
	 * @return true if successful, false if failed
	 */
	inline virtual bool insert(unsigned int track, SINK *sink) {
	    return QPtrVector<SINK>::insert(track, sink);
	};

	/** Remove all tracks / sinks */
	inline virtual void clear() {
	    QPtrVector<SINK>::clear();
	};
    };
}

#endif /* _KWAVE_MULTI_TRACK_SINK_H_ */
