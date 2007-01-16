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

class ArtsMultiPlaybackSink;

class ArtsPlaybackSink_impl
    :virtual public ArtsPlaybackSink_skel,
     virtual public Arts::StdSynthModule
{
public:

    /**
     * Default constructor
     */
    ArtsPlaybackSink_impl();

    /**
     * Constructor
     * @param pb_sink a ArtsMultiPlaybackSink that receives the samples
     * @param track index of the corresponding track [0...tracks-1]
     */
    ArtsPlaybackSink_impl(ArtsMultiPlaybackSink *pb_sink,
                          unsigned int track);

    /** Destructor */
    virtual ~ArtsPlaybackSink_impl();

    /**
     * Receiver and data processing function.
     * @see aRts/MCOP documentation
     */
    virtual void calculateBlock(unsigned long samples);

    /**
     * This is the most tricky part here - since we will run in a context
     * where no audio hardware will play the "give me more data role",
     * we'll have to request things ourselves (requireFlow() tells the
     * flowsystem that more signal flow should happen, so that
     * calculateBlock will get called)
     */
    virtual void goOn();

    /** Signals end of the stream */
    virtual bool done();

    /** @see Arts::StdSynthModule::start() */
    virtual void start() {
	Arts::StdSynthModule::start();
    };

    /** @see Arts::StdSynthModule::stop() */
    virtual void stop() {
	Arts::StdSynthModule::stop();
    };

protected:

    /** ArtsMultiPlaybackSink that receives the samples */
    ArtsMultiPlaybackSink *m_multi_sink;

    /** index of the corresponding track */
    unsigned int m_track;

};

#endif /* _ARTS_PLAYBACK_SINK_IMPL_H_ */
