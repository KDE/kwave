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

#include <math.h>
#include <string.h>
#include <arts/artsflow.h>

#include "libkwave/ArtsPlaybackSink.h"
#include "libkwave/ArtsMultiPlaybackSink.h"
#include "libkwave/PlayBackDevice.h"

//***************************************************************************
ArtsMultiPlaybackSink::ArtsMultiPlaybackSink(unsigned int tracks,
                                             PlayBackDevice *device)
    :m_device(device), m_tracks(tracks), m_sinks(tracks), m_done(false),
     m_out_buffer()
{
    unsigned int t;

    // allocate (empty) buffers for the tracks
    m_in_buffer.resize(m_tracks);
    m_in_buffer.fill(0);
    m_in_buffer_filled.resize(m_tracks);
    m_in_buffer_filled.fill(false);
    
    for (t=0; t < m_tracks; t++) {
	// allocate a buffer
        m_in_buffer.insert(t, new QMemArray<float>);
        Q_ASSERT(m_in_buffer[t]);
	
        // allocate a sink
	m_sinks.insert(t, 0);

	ArtsPlaybackSink_impl *r = new ArtsPlaybackSink_impl(this, t);
	Q_ASSERT(r);
	if (r) m_sinks.insert(t, new ArtsPlaybackSink(
		ArtsPlaybackSink::_from_base(r)));

	Q_ASSERT(m_sinks[t]);
	if (!m_sinks[t]) {
	    qWarning("ArtsMultiPlaybackSink: creation of adapter failed!!!");
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

    if (m_device) m_device->close();
}

//***************************************************************************
Arts::Object *ArtsMultiPlaybackSink::operator [] (unsigned int i)
{
    Q_ASSERT(m_sinks.size() == m_tracks);
    if (m_sinks.size() != m_tracks) return 0;
    Q_ASSERT(m_sinks[i]);
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
void ArtsMultiPlaybackSink::playback(int track, float *buffer,
                                     unsigned long samples)
{
    unsigned int t;
    
    Q_ASSERT(m_in_buffer[track]);
    Q_ASSERT(m_device);
    Q_ASSERT(m_tracks);
    if (!m_in_buffer[track] || !m_device || !m_tracks) return;
    
    // check if the input buffer of the track is usable
    if (m_in_buffer[track]->count() <= samples) {
	// increase the buffer sizes
	m_in_buffer[track]->resize(samples);
	m_in_buffer[track]->fill((float)0.0);
	
	Q_ASSERT(m_in_buffer[track]->count() >= samples);
	if (m_in_buffer[track]->count() < samples) return;
    }

    // create the output buffer if necessary
    if (m_out_buffer.count() < m_tracks) {
	m_out_buffer.resize(m_tracks);
	m_out_buffer.fill((sample_t)0);
    }

    // copy the input data to the buffer
//    float *pf = m_in_buffer[track]->data();
//    for (unsigned int i=0; i < samples; ++i) {
//	pf[i] = buffer[i];
//    }
    memcpy(m_in_buffer[track]->data(), buffer, samples*sizeof(buffer[0]));

    m_in_buffer_filled[track] = true;
    
    // check if all buffers are filled
    for (t=0; t < m_tracks; t++)
	if (!m_in_buffer_filled[t]) return; // not ready

    // now all tracks have left their data, now we are ready
    // to convert the buffers into a big combined one
    unsigned int sample;
    for (sample=0; sample < samples; ++sample) {
	for (t=0; t < m_tracks; t++)
	    m_out_buffer[t] = float2sample((*m_in_buffer[t])[sample]);

	// play the output buffer
	m_device->write(m_out_buffer);
    }
   
    m_in_buffer_filled.fill(false);
}                                     

//***************************************************************************
//***************************************************************************
