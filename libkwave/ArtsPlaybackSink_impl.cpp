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
#include "ArtsPlaybackSink_impl.h"

#include <arts/connect.h>
#include <arts/objectmanager.h>
#include <arts/flowsystem.h>
#include <arts/stdsynthmodule.h>

#include <qglobal.h>

//***************************************************************************
ArtsPlaybackSink_impl::ArtsPlaybackSink_impl()
    :ArtsPlaybackSink_skel(), Arts::StdSynthModule()
{
}

//***************************************************************************
ArtsPlaybackSink_impl::~ArtsPlaybackSink_impl()
{
}

//***************************************************************************
void ArtsPlaybackSink_impl::calculateBlock(unsigned long samples)
{
//    debug("ArtsPlaybackSink_impl::calculateBlock(%lu)", samples);
//    unsigned long i = 0;
//    sample_t sample = 0;
//
//    if (m_reader && !(m_reader->eof())) {
//	// fill the buffer with samples
//	for (i=0;i < samples;i++) {
//	    *m_reader >> sample;
//	    source[i] = sample / double(1 << 23);
//	    if (m_reader->eof()) break;
//	}
//    }
}

//***************************************************************************
void ArtsPlaybackSink_impl::goOn()
{
    _node()->requireFlow();
}

//***************************************************************************
bool ArtsPlaybackSink_impl::done()
{
    debug("ArtsPlaybackSink_impl::done()");
    return false;
}

//***************************************************************************
REGISTER_IMPLEMENTATION(ArtsPlaybackSink_impl);

//***************************************************************************
//***************************************************************************
