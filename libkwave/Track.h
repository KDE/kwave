/***************************************************************************
                Track.h  -  collects one or more stripes in one track
			     -------------------
    begin                : Feb 09 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _TRACK_H_
#define _TRACK_H_

#include <qlist.h>

#include "mt/Mutex.h"

#include "libkwave/InsertMode.h"
#include "libkwave/Stripe.h"

class SampleInputStream;

//***************************************************************************
class Track
{
public:
    /**
     * Default constructor. Creates a new and empty track with
     * zero-length and no stripes
     */
    Track();

    /**
     * Constructor. Creates an empty track with one stripe
     * with specified length.
     */
    Track(unsigned int length);

    /**
     * Appends a new empty stripe to the end of the track.
     */
    Stripe *appendStripe(unsigned int length);

    /**
     * Opens an input stream for a track, starting at a specified sample
     * position.
     * @param mode specifies where and how to insert
     * @param left start of the input (only useful in insert and
     *             overwrite mode)
     * @param right end of the input (only useful with overwrite mode)
     * @see InsertMode
     */
    SampleInputStream *openInputStream(InsertMode mode,
	unsigned int left = 0, unsigned int right = 0);

protected:

friend class SampleIputStream;

    /** returns the mutex for access to the whole track. */
    inline Mutex &mutex() {
	return m_lock;
    };

private:

    /** mutex for access to the whole track */
    Mutex m_lock;

    /** list of stripes (a track actually is a container for stripes) */
    QList<Stripe> m_stripes;

    /** mutex for access to the stripes list */
    Mutex m_lock_stripes;

};

#endif /* _TRACK_H_ */
