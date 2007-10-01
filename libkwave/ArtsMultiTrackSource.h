/***************************************************************************
 ArtsMultiTrackSource.h  -  aRts compatible source from a MultiTrackReader
                             -------------------
    begin                : Mon Dec 10 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#ifndef _ARTS_MULTI_TRACK_SOURCE_H_
#define _ARTS_MULTI_TRACK_SOURCE_H_

#include "config.h"
#ifdef HAVE_ARTS_SUPPORT

#include "libkwave/ArtsMultiSource.h"
#include "libkwave/ArtsMultiIO.h"
#include "libkwave/ArtsSampleSource.h"
#include "libkwave/ArtsSampleSource_impl.h"
#include "libkwave/MultiTrackReader.h"

//***************************************************************************
typedef ArtsMultiIO< ArtsMultiSource, ArtsSampleSource, \
    ArtsSampleSource_impl, MultiTrackReader > ArtsMultiTrackSource_base;

//***************************************************************************
class ArtsMultiTrackSource
    :public ArtsMultiTrackSource_base
{
public:
    /**
     * Constructor.
     * @see ArtsMultiTrackSource
     */
    ArtsMultiTrackSource(MultiTrackReader &reader)
	:ArtsMultiTrackSource_base(reader) {};

    /** Destructor. */
    virtual ~ArtsMultiTrackSource() {};

    /** @see ArtsMultiSource::done() */
    virtual bool done() {
	for (unsigned int t=0; t < m_count; ++t)
	    if (!m_ios[t]->done()) return false;
	return true;
    };
};

#else /* HAVE_ARTS_SUPPORT */
#warning aRts support is disabled
#endif /* HAVE_ARTS_SUPPORT */

#endif /* _ARTS_MULTI_TRACK_SOURCE_H_ */
