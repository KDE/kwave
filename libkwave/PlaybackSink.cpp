/***************************************************************************
       PlaybackSink.cpp  -  multi-track Kwave compatible sink for playback
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

#include "libkwave/PlaybackSink.h"
#include "libkwave/SampleArray.h"

//***************************************************************************
Kwave::PlaybackSink::PlaybackSink(unsigned int track)
    :Kwave::SampleSink(), m_track(track)
{
}

//***************************************************************************
Kwave::PlaybackSink::~PlaybackSink()
{
}

//***************************************************************************
void Kwave::PlaybackSink::input(Kwave::SampleArray data)
{
    emit output(m_track, data);
}

//***************************************************************************
//***************************************************************************

#include "moc_PlaybackSink.cpp"
