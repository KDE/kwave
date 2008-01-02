/***************************************************************************
    KwaveMultiPlaybackSink.cpp  -  multi-track Kwave playback sink
                             -------------------
    begin                : Sun Nov 04 2007
    copyright            : (C) 2007 by Thomas Eschenbacher
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

#include <QtGlobal>

#include "libkwave/memcpy.h"
#include "libkwave/KwaveMultiPlaybackSink.h"
#include "libkwave/KwavePlaybackSink.h"
#include "libkwave/PlayBackDevice.h"

//***************************************************************************
Kwave::MultiPlaybackSink::MultiPlaybackSink(unsigned int tracks,
                                            PlayBackDevice *device)
    :Kwave::MultiTrackSink<Kwave::PlaybackSink>(tracks),
     m_tracks(tracks), m_device(device), m_in_buffer(tracks),
     m_in_buffer_filled(tracks),
     m_out_buffer(tracks)
{
    m_in_buffer.fill(0);
    m_in_buffer_filled.fill(false);

    for (unsigned int track = 0; track < m_tracks; track++) {
	// allocate a input buffer
	m_in_buffer[track] = new Kwave::SampleArray(blockSize());
	Q_ASSERT(m_in_buffer[track]);

	// allocate a sink
	Kwave::PlaybackSink *sink = new Kwave::PlaybackSink(track);
	insert(track, sink);
	connect(sink, SIGNAL(output(unsigned int, Kwave::SampleArray &)),
	        this, SLOT(input(unsigned int, Kwave::SampleArray &)));
    }
}

//***************************************************************************
Kwave::MultiPlaybackSink::~MultiPlaybackSink()
{
    // close all stream objects
    clear();

    // close the device
    if (m_device) {
	m_device->close();
	delete m_device;
    }
    m_device = 0;

    // discard the buffers
    while (!m_in_buffer.isEmpty()) {
	Kwave::SampleArray *b = m_in_buffer[0];
	if (b) delete b;
	m_in_buffer.remove(0);
    }
    m_in_buffer.clear();
}

//***************************************************************************
void Kwave::MultiPlaybackSink::input(unsigned int track,
                                     Kwave::SampleArray &data)
{
    Q_ASSERT(m_device);
    Q_ASSERT(m_tracks);
    Q_ASSERT(m_in_buffer[track]);
    if (!m_device || !m_tracks || !m_in_buffer[track]) return;

    Q_ASSERT(!m_in_buffer_filled[track]);
    m_in_buffer_filled[track] = true;

    // copy the input data to the buffer
    unsigned int samples = data.size();
    Q_ASSERT(m_in_buffer[track]->size() >= data.size());
    MEMCPY(m_in_buffer[track]->data(), data.data(),
           samples * sizeof(data[0]));

    // check if all buffers are filled
    for (unsigned int track = 0; track < m_tracks; track++)
	if (!m_in_buffer_filled[track]) return;

    // all tracks have left their data, now we are ready
    // to convert the buffers into a big combined one
    Q_ASSERT(m_out_buffer.size() >= m_tracks);
    for (unsigned int sample=0; sample < samples; sample++) {
	for (unsigned int t=0; t < m_tracks; t++)
	    m_out_buffer[t] = (*m_in_buffer[t])[sample];

	// play the output buffer
	m_device->write(m_out_buffer);
    }

    m_in_buffer_filled.fill(false);
}

//***************************************************************************
#include "KwaveMultiPlaybackSink.moc"
//***************************************************************************
//***************************************************************************
