/*************************************************************************
    KwaveMultiTrackSource.h  -  template for multi-track sources
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

#ifndef _KWAVE_MULTI_TRACK_SOURCE_H_
#define _KWAVE_MULTI_TRACK_SOURCE_H_

#include "config.h"
#include <qobject.h>
#include <qptrvector.h>

#include "libkwave/KwaveSampleSource.h"

namespace Kwave {

    template <class SOURCE>
    class MultiTrackSource: public Kwave::SampleSource,
	                    private QPtrVector<SOURCE>
    {
    public:
	/**
	 * Constructor
	 *
	 * @param tracks number of tracks
	 * @param parent a parent object, passed to QObject (optional)
	 * @param name a free name, for identifying this object,
	 *             will be passed to the QObject (optional)
	 */
	MultiTrackSource(unsigned int tracks,
	                 QObject *parent=0,
	                 const char *name=0)
	    :Kwave::SampleSource(parent, name),
	     QPtrVector<SOURCE>()
        {
	    QPtrVector<SOURCE>::setAutoDelete(true);
	    QPtrVector<SOURCE>::resize(tracks);
	    Q_ASSERT(QPtrVector<SOURCE>::size() == tracks);
	};

	/** Destructor */
	virtual ~MultiTrackSource()
	{
	    QPtrVector<SOURCE>::setAutoDelete(true);
	    QPtrVector<SOURCE>::clear();
	};

	/**
	 * Calls goOn() for each track.
	 * @see Kwave::SampleSource::goOn()
	 */
	virtual void goOn()
	{
	    const unsigned int tracks = QPtrVector<SOURCE>::size();
	    for (unsigned int i=0; i < tracks; i++)
	    {
		SOURCE *src = QPtrVector<SOURCE>::at(i);
		Q_ASSERT(src);
		if (src) src->goOn();
	    }
	};

	/** Returns true when all sources are done */
	virtual bool done()
	{
	    for (unsigned int track=0; track < tracks(); ++track) {
		Kwave::SampleSource *s = (*this)[track];
		if (s && !s->done()) return false;
	    }
	    return true;
	};

	/**
	 * Returns the number of tracks that the source provides
	 * @return number of tracks
	 */
	virtual unsigned int tracks() const
	{
	    return QPtrVector<SOURCE>::size();
	};

	/**
	 * Returns the source that corresponds to one specific track
	 * if the object has multiple tracks. For single-track objects
	 * it returns "this" for the first index and 0 for all others
	 */
	inline virtual SOURCE *at(unsigned int track) const {
	    return QPtrVector<SOURCE>::at(track);
	};

	/** @see the Kwave::MultiTrackSource.at()... */
	inline virtual SOURCE * operator [] (unsigned int track) {
	    return at(track);
	};

	/**
	 * Insert a new track with a source.
	 *
	 * @param track index of the track [0...N-1]
	 * @param sink pointer to a Kwave::SampleSource
	 * @return true if successful, false if failed
	 */
	inline virtual bool insert(unsigned int track, SOURCE *source) {
	    return QPtrVector<SOURCE>::insert(track, source);
	};

	/** Remove all tracks / sources */
	inline virtual void clear() {
	    QPtrVector<SOURCE>::clear();
	};

    };
}

#endif /* _KWAVE_MULTI_TRACK_SOURCE_H_ */
