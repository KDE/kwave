/***************************************************************************
         PlaybackSink.h  -  multi-track Kwave compatible sink for playback
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

#ifndef PLAYBACK_SINK_H
#define PLAYBACK_SINK_H

#include "config.h"

#include <QObject>

#include "libkwave/SampleArray.h"
#include "libkwave/SampleSink.h"

namespace Kwave
{

    class PlaybackSink: public Kwave::SampleSink
    {
        Q_OBJECT
    public:
        /**
         * Constructor
         * @param track index of this playback channel
         */
        explicit PlaybackSink(unsigned int track);

        /** Destructor */
        ~PlaybackSink() override;

    signals:
        /** emits back the sample data received through input(...) */
        void output(unsigned int track, Kwave::SampleArray data);

    public slots:
        /** receives sample data for this playback channel */
        void input(Kwave::SampleArray data);

    private:
        /** index of the track of this playback channel */
        unsigned int m_track;
    };
}

#endif /* PLAYBACK_SINK_H */

//***************************************************************************
//***************************************************************************
