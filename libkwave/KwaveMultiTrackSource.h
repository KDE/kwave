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
#include <QObject>
#include <QVector>

#include "libkwave/KwaveSampleSource.h"

namespace Kwave {

    /**
     * Template for easier handling of Kwave::SampleSource objects
     * that consist of multiple independend tracks.
     */
    template <class SOURCE, bool INITIALIZE>
    class MultiTrackSource: public Kwave::SampleSource,
	                    private QVector<SOURCE *>
    {
    public:
	/**
	 * Default constructor, which does no initialization of the
	 * objects themselfes. If you want to use this, you should
	 * derive from this class, create all objects manually and
	 * "insert" them from the constructor.
	 *
	 * @param tracks number of tracks
	 * @param parent a parent object, passed to QObject (optional)
	 */
	MultiTrackSource(unsigned int tracks,
	                 QObject *parent=0)
	    :Kwave::SampleSource(parent),
	     QVector<SOURCE *>(tracks)
        {
	    Q_ASSERT(INITIALIZE || (tracks == 0));
	    Q_ASSERT(QVector<SOURCE *>::size() == static_cast<int>(tracks));
	}

	/** Destructor */
	virtual ~MultiTrackSource()
	{
	    clear();
	}

	/**
	 * Calls goOn() for each track.
	 * @see Kwave::SampleSource::goOn()
	 */
	virtual void goOn()
	{
	    foreach (SOURCE *src, static_cast< QVector<SOURCE *> >(*this))
		if (src) src->goOn();
	}

	/** Returns true when all sources are done */
	virtual bool done() const
	{
	    foreach (SOURCE *src, static_cast< QVector<SOURCE *> >(*this))
		if (src && !src->done()) return false;
	    return true;
	}

	/**
	 * Returns the number of tracks that the source provides
	 * @return number of tracks
	 */
	virtual unsigned int tracks() const
	{
	    return QVector<SOURCE *>::size();
	}

	/**
	 * Returns the source that corresponds to one specific track
	 * if the object has multiple tracks. For single-track objects
	 * it returns "this" for the first index and 0 for all others
	 */
	inline virtual SOURCE *at(unsigned int track) const {
	    return QVector<SOURCE *>::at(track);
	}

	/** @see the Kwave::MultiTrackSource.at()... */
	inline virtual SOURCE * operator [] (unsigned int track) {
	    return at(track);
	}

	/**
	 * Insert a new track with a source.
	 *
	 * @param track index of the track [0...N-1]
	 * @param sink pointer to a Kwave::SampleSource
	 * @return true if successful, false if failed
	 */
	inline virtual bool insert(unsigned int track, SOURCE *source) {
	    QVector<SOURCE *>::insert(track, source);
	    return (at(track) == source);
	}

	/** Remove all tracks / sources */
	inline virtual void clear() {
	    while (!QVector<SOURCE *>::isEmpty()) {
		SOURCE *s = at(0);
		if (s) delete s;
		QVector<SOURCE *>::remove(0);
	    }
	    QVector<SOURCE *>::clear();
	}
    };

    /**
     * Specialized version that internally initializes all objects
     * by generating them through their default constructor.
     */
    template <class SOURCE>
    class MultiTrackSource<SOURCE, true>
	:public Kwave::MultiTrackSource<SOURCE, false>
    {
    public:
	/**
	 * Constructor
	 *
	 * @param tracks number of tracks
	 * @param parent a parent object, passed to QObject (optional)
	 */
	MultiTrackSource(unsigned int tracks,
	                 QObject *parent = 0)
	    :Kwave::MultiTrackSource<SOURCE, false>(0, parent)
	{
	    for (unsigned int i=0; i < tracks; i++)
		insert(i, new SOURCE());
	}

	/** Destructor */
	virtual ~MultiTrackSource() { }
    };

}

#endif /* _KWAVE_MULTI_TRACK_SOURCE_H_ */
