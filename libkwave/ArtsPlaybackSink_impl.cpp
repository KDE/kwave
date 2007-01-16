/***************************************************************************
    ArtsPlaybackSink_impl.cpp  -  aRts compatible sink for playback
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

#include "config.h"

#include <arts/connect.h>
#include <arts/objectmanager.h>
#include <arts/flowsystem.h>
#include <arts/stdsynthmodule.h>
#include <qglobal.h>

#include "libkwave/ArtsPlaybackSink_impl.h"
#include "libkwave/ArtsMultiPlaybackSink.h"

//***************************************************************************
ArtsPlaybackSink_impl::ArtsPlaybackSink_impl()
    :ArtsPlaybackSink_skel(), Arts::StdSynthModule(),
     m_multi_sink(0), m_track(0)
{
}

//***************************************************************************
ArtsPlaybackSink_impl::ArtsPlaybackSink_impl(ArtsMultiPlaybackSink *pb_sink,
                                             unsigned int track)
    :ArtsPlaybackSink_skel(), Arts::StdSynthModule(),
     m_multi_sink(pb_sink), m_track(track)
{
}

//***************************************************************************
ArtsPlaybackSink_impl::~ArtsPlaybackSink_impl()
{
}

//***************************************************************************
void ArtsPlaybackSink_impl::calculateBlock(unsigned long samples)
{
    m_multi_sink->playback(m_track, sink, samples);
}

//***************************************************************************
void ArtsPlaybackSink_impl::goOn()
{
    _node()->requireFlow();
}

//***************************************************************************
bool ArtsPlaybackSink_impl::done()
{
    return (m_multi_sink);
}

//***************************************************************************
REGISTER_IMPLEMENTATION(ArtsPlaybackSink_impl);

//***************************************************************************
//***************************************************************************
