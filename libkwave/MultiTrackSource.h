/*************************************************************************
    MultiTrackSource.h  -  template for multi-track sources
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

#ifndef MULTI_TRACK_SOURCE_H
#define MULTI_TRACK_SOURCE_H

#include "config.h"

#include <new>

#include <QtCore/QFutureSynchronizer>
#include <QtCore/QtConcurrentRun>
#include <QtCore/QObject>
#include <QtCore/QList>

#include "libkwave/SampleSource.h"

namespace Kwave
{

    /**
     * Template for easier handling of Kwave::SampleSource objects
     * that consist of multiple independent tracks.
     */
    template <class SOURCE, const bool INITIALIZE>
    class KDE_EXPORT MultiTrackSource: public Kwave::SampleSource,
	                               private QList<SOURCE *>
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
	MultiTrackSource(unsigned int tracks, QObject *parent = 0)
	    :Kwave::SampleSource(parent),
	     QList<SOURCE *>()
        {
	    Q_UNUSED(tracks);
	    Q_ASSERT(INITIALIZE || (tracks == 0));
	    Q_ASSERT(QList<SOURCE *>::size() == static_cast<int>(tracks));
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
	    QFutureSynchronizer<void> synchronizer;

	    foreach (SOURCE *src, static_cast< QList<SOURCE *> >(*this)) {
		if (!src) continue;
		synchronizer.addFuture(QtConcurrent::run(
		    this,
		    &Kwave::MultiTrackSource<SOURCE, INITIALIZE>::runSource,
		    src)
		);
	    }

	    synchronizer.waitForFinished();
	}

	/** Returns true when all sources are done */
	virtual bool done() const
	{
	    foreach (SOURCE *src, static_cast< QList<SOURCE *> >(*this))
		if (src && !src->done()) return false;
	    return true;
	}

	/**
	 * Returns the number of tracks that the source provides
	 * @return number of tracks
	 */
	virtual unsigned int tracks() const
	{
	    return QList<SOURCE *>::size();
	}

	/**
	 * Returns the source that corresponds to one specific track
	 * if the object has multiple tracks. For single-track objects
	 * it returns "this" for the first index and 0 for all others
	 */
	inline virtual SOURCE *at(unsigned int track) const {
	    return QList<SOURCE *>::at(track);
	}

	/** @see the Kwave::MultiTrackSource.at()... */
	inline virtual SOURCE * operator [] (unsigned int track) {
	    return at(track);
	}

	/**
	 * Insert a new track with a source.
	 *
	 * @param track index of the track [0...N-1]
	 * @param source pointer to a Kwave::SampleSource
	 * @return true if successful, false if failed
	 */
	inline virtual bool insert(unsigned int track, SOURCE *source) {
	    QList<SOURCE *>::insert(track, source);
	    return (at(track) == source);
	}

	/** Remove all tracks / sources */
	inline virtual void clear() {
	    while (!QList<SOURCE *>::isEmpty())
		delete QList<SOURCE *>::takeLast();
	}

    private:

	/** little wrapper for calling goOn() of a source in a worker thread */
	void runSource(SOURCE *src) {
	    src->goOn();
	}

    };

    /**
     * Specialized version that internally initializes all objects
     * by generating them through their default constructor.
     */
    template <class SOURCE>
    class KDE_EXPORT MultiTrackSource<SOURCE, true>
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
	    for (unsigned int i = 0; i < tracks; i++)
		this->insert(i, new(std::nothrow) SOURCE());
	}

	/** Destructor */
	virtual ~MultiTrackSource() { }
    };

}

#endif /* MULTI_TRACK_SOURCE_H */

//***************************************************************************
//***************************************************************************
