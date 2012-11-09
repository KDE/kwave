/***************************************************************************
    MultiPlaybackSink.h  -  multi-track Kwave playback sink
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

#ifndef _MULTI_PLAYBACK_SINK_H_
#define _MULTI_PLAYBACK_SINK_H_

#include "config.h"

#include <QBitArray>
#include <QMutex>
#include <QVector>

#include <kdemacros.h>

#include "libkwave/MultiTrackSink.h"
#include "libkwave/PlaybackSink.h"
#include "libkwave/PlayBackDevice.h"
#include "libkwave/SampleArray.h"

namespace Kwave {
    class KDE_EXPORT MultiPlaybackSink
	:public Kwave::MultiTrackSink<Kwave::PlaybackSink>
    {
	Q_OBJECT
    public:
	/**
	 * Constructor
	 * @param tracks number of tracks for playback
	 * @param device a PlayBackDevice
	 */
	MultiPlaybackSink(unsigned int tracks, PlayBackDevice *device);

	/** Destructor */
	virtual ~MultiPlaybackSink();

    private slots:

	/**
	 * receives data from one of the tracks
	 * @param track index of the track [0...tracks-1]
	 * @param data sample data for the given track
	 */
	void input(unsigned int track, Kwave::SampleArray data);

    private:

	/** number of tracks */
	unsigned int m_tracks;

	/** device used for playback */
	PlayBackDevice *m_device;

	/** list of input buffers */
	QVector< Kwave::SampleArray > m_in_buffer;

	/** "filled"-flags for input buffers */
	QBitArray m_in_buffer_filled;

	/** output buffer for all samples */
	Kwave::SampleArray m_out_buffer;

	/** mutex for locking against reentrance */
	QMutex m_lock;

    };
}

#endif /* _MULTI_PLAYBACK_SINK_H_ */
