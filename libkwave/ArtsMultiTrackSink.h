/***************************************************************************
   ArtsMultiTrackSink.h  -  aRts compatible sink to a MultiTrackWriter
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

#ifndef _ARTS_MULTI_TRACK_SINK_H_
#define _ARTS_MULTI_TRACK_SINK_H_

#include "libkwave/ArtsMultiSink.h"
#include "libkwave/ArtsMultiIO.h"
#include "libkwave/ArtsSampleSink.h"
#include "libkwave/ArtsSampleSink_impl.h"
#include "libkwave/MultiTrackWriter.h"

//***************************************************************************
typedef class ArtsMultiIO<ArtsMultiSink, ArtsSampleSink, \
    ArtsSampleSink_impl, MultiTrackWriter> ArtsMultiTrackSink_base;

//***************************************************************************
class ArtsMultiTrackSink
    :public ArtsMultiTrackSink_base
{
public:
    /**
     * Constructor.
     * @see ArtsMultiTrackSink
     */
    ArtsMultiTrackSink(MultiTrackWriter &writer)
	:ArtsMultiTrackSink_base(writer) {};

    /** Destructor */
    virtual ~ArtsMultiTrackSink() {};

    /** @see ArtsMultiSink::goOn() */
    virtual void goOn() {
	unsigned int t;
	for (t=0; t < m_count; ++t) {
	    m_ios[t]->goOn();
	}
    }
};

#endif /* _ARTS_MULTI_TRACK_SINK_H_ */
