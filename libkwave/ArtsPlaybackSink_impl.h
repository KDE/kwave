/***************************************************************************
    ArtsPlaybackSink_impl.h  -  aRts compatible sink for playback
                             -------------------
    begin                : Mon Apr 28 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
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

#ifndef _ARTS_PLAYBACK_SINK_IMPL_H_
#define _ARTS_PLAYBACK_SINK_IMPL_H_

#include "config.h"
#include <arts/artsflow.h>
#include <arts/stdsynthmodule.h>

#include "libkwave/ArtsPlaybackSink.h"

class ArtsPlaybackSink_impl
    :virtual public ArtsPlaybackSink_skel,
     virtual public Arts::StdSynthModule
{
public:

    /** Constructor */
    ArtsPlaybackSink_impl();

    /** Destructor */
    virtual ~ArtsPlaybackSink_impl();

    /**
     * Receiver and data processing function.
     * @see aRts/MCOP documentation
     */
    void calculateBlock(unsigned long samples);

    /**
     * This is the most tricky part here - since we will run in a context
     * where no audio hardware will play the "give me more data role",
     * we'll have to request things ourselves (requireFlow() tells the
     * flowsystem that more signal flow should happen, so that
     * calculateBlock will get called)
     */
    void goOn();

    /** Signals end of the stream */
    bool done();
    
};

#endif /* _ARTS_PLAYBACK_SINK_IMPL_H_ */
