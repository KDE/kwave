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

#ifndef MULTI_PLAYBACK_SINK_H
#define MULTI_PLAYBACK_SINK_H

#include "config.h"
#include "libkwave_export.h"

#include <QtGlobal>
#include <QBitArray>
#include <QMutex>
#include <QVector>

#include "libkwave/MultiTrackSink.h"
#include "libkwave/PlayBackDevice.h"
#include "libkwave/PlaybackSink.h"
#include "libkwave/SampleArray.h"

namespace Kwave
{

    class LIBKWAVE_EXPORT MultiPlaybackSink
        :public Kwave::MultiTrackSink<Kwave::PlaybackSink, false>
    {
        Q_OBJECT
    public:
        /**
         * Constructor
         * @param tracks number of tracks for playback
         * @param device a PlayBackDevice
         */
        MultiPlaybackSink(unsigned int tracks, Kwave::PlayBackDevice *device);

        /** Destructor */
        virtual ~MultiPlaybackSink() override;

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
        Kwave::PlayBackDevice *m_device;

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

#endif /* MULTI_PLAYBACK_SINK_H */

//***************************************************************************
//***************************************************************************
