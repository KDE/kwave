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
#include <QObject>
#include <QVector>

#include "libkwave/KwaveSampleSink.h"

namespace Kwave {
    template <class SINK>
    class MultiTrackSink: public Kwave::SampleSink,
                          private QVector<SINK *>
    {
    public:
	/**
	 * Constructor
	 * @param tracks number of tracks
	 * @param parent a parent object, passed to QObject (optional)
	 */
	MultiTrackSink(unsigned int tracks, QObject *parent = 0)
	    :Kwave::SampleSink(parent),
	    QVector<SINK *>(tracks)
	{
	    QVector<SINK *>::fill(static_cast<SINK *>(0));
	    Q_ASSERT(QVector<SINK *>::size() == static_cast<int>(tracks));
	}

	/** Destructor */
	virtual ~MultiTrackSink()
	{
	    clear();
	}

	/** Returns true when all sinks are done */
	virtual bool done() const
	{
	    foreach (Kwave::SampleSink *s,
		     static_cast< QVector<SINK *> >(*this))
		if (s && !s->done()) return false;
	    return true;
	}

	/**
	 * Returns the number of tracks that the sink provides
	 * @return number of tracks
	 */
	virtual unsigned int tracks() const
	{
	    return QVector<SINK *>::size();
	}

	/**
	 * Returns the sink that corresponds to one specific track
	 * if the object has multiple tracks. For single-track objects
	 * it returns "this" for the first index and 0 for all others
	 */
	inline virtual SINK *at(unsigned int track) const {
	    return QVector<SINK *>::at(track);
	}

	/** @see the Kwave::MultiTrackSink.at()... */
	inline virtual SINK * operator [] (unsigned int track) {
	    return at(track);
	}

	/**
	 * Insert a new track with a sink.
	 *
	 * @param track index of the track [0...N-1]
	 * @param sink pointer to a Kwave::SampleSink
	 * @return true if successful, false if failed
	 */
	virtual bool insert(unsigned int track, SINK *sink) {
	    QVector<SINK *>::insert(track, sink);
	    return (at(track) == sink);
	}

	/** Remove all tracks / sinks */
	virtual void clear() {
	    while (!QVector<SINK *>::isEmpty()) {
		Kwave::SampleSink *s = at(0);
		if (s) delete s;
		QVector<SINK *>::remove(0);
	    }
	    QVector<SINK *>::clear();
	}
    };
}

#endif /* _KWAVE_MULTI_TRACK_SINK_H_ */
