/***************************************************************************
    ArtsSampleSink_impl.h  -  adapter for converting from aRts to Kwave
			     -------------------
    begin                : Nov 13 2001
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

#ifndef _ARTS_SAMPLE_SINK_H_
#define _ARTS_SAMPLE_SINK_H_

#include <arts/artsflow.h>
#include <arts/stdsynthmodule.h>

#include "libkwave/SampleWriter.h"
#include "libkwave/ArtsSampleSink.h"

//***************************************************************************
class ArtsSampleSink_impl
    :virtual public ArtsSampleSink_skel,
     virtual public Arts::StdSynthModule
{
public:

    /** Default constructor. Should never be used */
    ArtsSampleSink_impl();

    /**
     * Constructor.
     * @param writer a SampleWriter to receive the aRts output
     */
    ArtsSampleSink_impl(SampleWriter *writer);

    /**
     * Receiver and data processing function.
     * @see aRts/MCOP documentation
     */
    void calculateBlock(unsigned long samples);

    /*
     * This is the most tricky part here - since we will run in a context
     * where no audio hardware will play the "give me more data role",
     * we'll have to request things ourselves (requireFlow() tells the
     * flowsystem that more signal flow should happen, so that
     * calculateBlock will get called
     */
    void goOn();

protected:

    /** receiver for the converted aRts sample stream */
    SampleWriter *m_writer;

    /** true if end of stream reached */
    bool m_done;

};

#endif /* _ARTS_SAMPLE_SINK_H_ */
