/***************************************************************************
  MultiPlaybackSink.cpp  -  multi-track Kwave playback sink
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

#include <new>

#include <QMutexLocker>
#include <QtGlobal>
#include <QThread>

#include "libkwave/MultiPlaybackSink.h"
#include "libkwave/PlayBackDevice.h"
#include "libkwave/PlaybackSink.h"
#include "libkwave/memcpy.h"

//***************************************************************************
Kwave::MultiPlaybackSink::MultiPlaybackSink(unsigned int tracks,
                                            Kwave::PlayBackDevice *device)
    :Kwave::MultiTrackSink<Kwave::PlaybackSink, false>(0),
     m_tracks(tracks), m_device(device), m_in_buffer(tracks),
     m_in_buffer_filled(tracks),
     m_out_buffer(tracks), m_lock()
{
    m_in_buffer.fill(Kwave::SampleArray(0));
    m_in_buffer_filled.fill(false);

    for (unsigned int track = 0; track < m_tracks; track++) {
	// allocate a sink
	Kwave::PlaybackSink *sink =
	    new(std::nothrow) Kwave::PlaybackSink(track);
	insert(track, sink);
	connect(sink, SIGNAL(output(uint,Kwave::SampleArray)),
	        this, SLOT(input(uint,Kwave::SampleArray)),
	        Qt::DirectConnection);
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
    m_device = Q_NULLPTR;

    // discard the buffers
    while (!m_in_buffer.isEmpty())
	m_in_buffer.erase(m_in_buffer.end() - 1);
    m_in_buffer.clear();
}

//***************************************************************************
void Kwave::MultiPlaybackSink::input(unsigned int track,
                                     Kwave::SampleArray data)
{
    QMutexLocker lock(&m_lock);

    Q_ASSERT(m_device);
    Q_ASSERT(m_tracks);
    if (!m_device || !m_tracks) return;

    Q_ASSERT(!m_in_buffer_filled[track]);
    m_in_buffer_filled[track] = true;

    // copy the input data to the buffer
    unsigned int samples = data.size();
    m_in_buffer[track] = data;

    // check if all buffers are filled
    for (unsigned int t = 0; t < m_tracks; t++)
	if (!m_in_buffer_filled[t]) return;

    // all tracks have left their data, now we are ready
    // to convert the buffers into a big combined one
    Q_ASSERT(m_out_buffer.size() >= m_tracks);
    for (unsigned int sample = 0; sample < samples; sample++) {
	for (unsigned int t = 0; t < m_tracks; t++) {
	    const Kwave::SampleArray &in = m_in_buffer[t];
	    m_out_buffer[t] = in[sample];
	}

	// play the output buffer
	unsigned int retry = 5;
	while (retry--) {
	    // shortcut for more responsiveness when pressing cancel
	    if (QThread::currentThread()->isInterruptionRequested())
		break;

	    int res = m_device->write(m_out_buffer);
	    if (res != 0) {
		QThread::yieldCurrentThread();
	    } else break;
	}
    }

    m_in_buffer_filled.fill(false);
}

//***************************************************************************
//***************************************************************************
