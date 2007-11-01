/*************************************************************************
    KwaveStreamObject.h  -  base class with a generic sample source/sink
                             -------------------
    begin                : Thu Nov 01 2007
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

#ifndef _KWAVE_STREAM_OBJECT_H_
#define _KWAVE_STREAM_OBJECT_H_

#include "config.h"
#include <qobject.h>

namespace Kwave {
    class StreamObject: public QObject
    {
    public:
	/**
	 * Constructor
	 *
	 * @param parent a parent object, passed to QObject (optional)
	 * @param name a free name, for identifying this object,
	 *             will be passed to the QObject (optional)
	 */
	StreamObject(QObject *parent=0, const char *name=0)
	    :QObject(parent, name)
	{
	};

	/** Destructor */
	virtual ~StreamObject()
	{
	};

	/**
	 * Returns the number of tracks that the source provides
	 * @return number of tracks, default is 1
	 */
	virtual unsigned int tracks() const { return 1; };

	/**
	 * Returns the source that corresponds to one specific track
	 * if the object has multiple tracks. For single-track objects
	 * it returns "this" for the first index and 0 for all others
	 */
	virtual Kwave::StreamObject * operator [] (unsigned int track)
	{
	    return (track == 0) ? this : 0;
	};

    };
}

#endif /* _KWAVE_STREAM_OBJECT_H_ */
