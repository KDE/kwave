/***************************************************************************
    ArtsMultiPlaybackSink.cpp  -  multi-track aRts compatible sink for playback
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
#include <arts/artsflow.h>

#include "ArtsPlaybackSink.h"
#include "ArtsMultiPlaybackSink.h"

////***************************************************************************
ArtsMultiPlaybackSink::ArtsMultiPlaybackSink(unsigned int tracks,
                                             PlaybackDevice *device)
    :m_device(device), m_tracks(tracks), m_sinks(tracks), m_done(false)
{
    unsigned int t;

    for (t=0; t < m_tracks; t++) {
	m_sinks.insert(t, 0);

	ArtsPlaybackSink_impl *r = new ArtsPlaybackSink_impl();
	ASSERT(r);
	if (r) m_sinks.insert(t, new ArtsPlaybackSink(
		ArtsPlaybackSink::_from_base(r)));

	ASSERT(m_sinks[t]);
	if (!m_sinks[t]) {
	    warning("ArtsMultiPlaybackSink: creation of adapter failed!!!");
	    m_tracks = t;
	    break;
	}
    }
}

//***************************************************************************
ArtsMultiPlaybackSink::~ArtsMultiPlaybackSink()
{
    m_sinks.setAutoDelete(true);
    while (m_tracks--) {
	m_sinks.remove(m_tracks);
    }
}

//***************************************************************************
Arts::Object *ArtsMultiPlaybackSink::operator [] (unsigned int i)
{
    ASSERT(m_sinks.size() == m_tracks);
    if (m_sinks.size() != m_tracks) return 0;
    return m_sinks[i];
}

//***************************************************************************
void ArtsMultiPlaybackSink::goOn()
{
    for (unsigned int t=0; t < m_tracks; ++t) {
	m_sinks[t]->goOn();
    }
}

//***************************************************************************
bool ArtsMultiPlaybackSink::done()
{
    return (m_done || !m_tracks);
}

//***************************************************************************
//***************************************************************************
