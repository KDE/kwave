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
	     QPtrVector<SOURCE>(tracks)
        {
	    for (unsigned int i=0; i < tracks; i++)
	    {
		SOURCE *src = new SOURCE();
		Q_ASSERT(src);
		if (!src) break;
		insert(i, src);
	    }
	};

	/** Destructor */
	virtual ~MultiTrackSource()
	{
	    QPtrVector<SOURCE>::clear();
	};

	/**
	 * Calls goOn() for each track.
	 * @see Kwave::SampleSource::goOn()
	 */
	virtual void goOn()
	{
	    const unsigned int tracks = QPtrVector<SOURCE>::count();
	    for (unsigned int i=0; i < tracks; i++)
	    {
		SOURCE *src = QPtrVector<SOURCE>::at(i);
		if (src) src->goOn();
	    }
	};

	/**
	 * Returns the number of tracks that the source provides
	 * @return number of tracks
	 */
	virtual unsigned int tracks() const
	{
	    return QPtrVector<SOURCE>::count();
	};

	/**
	 * Returns the source that corresponds to one specific track
	 * if the object has multiple tracks. For single-track objects
	 * it returns "this" for the first index and 0 for all others
	 */
	virtual Kwave::SampleSource * operator [] (unsigned int track)
	{
	    return QPtrVector<SOURCE>::at(track);
	};
    };
}

#endif /* _KWAVE_MULTI_TRACK_SOURCE_H_ */
