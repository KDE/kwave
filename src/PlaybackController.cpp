/***************************************************************************
     PlaybackController.cpp  -  Interface for generic playback control
			     -------------------
    begin                : Nov 15 2000
    copyright            : (C) 2000 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "PlaybackController.h"

//***************************************************************************
PlaybackController::PlaybackController()
{
    m_loop_mode = false;
    m_paused = false;
    m_playing = false;
    m_playback_position = 0;
    m_playback_start = 0;
}

//***************************************************************************
PlaybackController::~PlaybackController()
{
    playbackStop();
}

//***************************************************************************
void PlaybackController::playbackStart()
{
    debug("PlaybackController::playbackStart()");
    if (m_playing) {
	// first stop playback
	emit sigStopPlayback();
	emit sigPlaybackStopped();
    }

    // (re)start from beginning without loop mode
    m_playback_position = m_playback_start;
    m_loop_mode = false;
    m_paused = false;
    emit sigStartPlayback();
    emit sigPlaybackStarted();
    m_playing = true;
}

//***************************************************************************
void PlaybackController::playbackLoop()
{
    if (m_playing) {
	// first stop playback
	emit sigStopPlayback();
	emit sigPlaybackStopped();
    }

    // (re)start from beginning without loop mode
    m_playback_position = m_playback_start;
    m_loop_mode = true;
    m_paused = false;
    emit sigStartPlayback();
    emit sigPlaybackStarted();
    m_playing = true;
}

//***************************************************************************
void PlaybackController::playbackPause()
{
    if (!m_playing) return; // no effect if not playing

    // stop playback for now and set the paused flag
    emit sigStopPlayback();
    emit sigPlaybackPaused();
    m_paused = true;
    m_playing = false;
}

//***************************************************************************
void PlaybackController::playbackContinue()
{
    // if not paused, do the same as start
    if (!m_paused) playbackStart();

    // else reset the paused flag and start from current position
    m_paused = false;
    emit sigStartPlayback();
    emit sigPlaybackStarted();
    m_playing=true;
}

//***************************************************************************
void PlaybackController::playbackStop()
{
    debug("PlaybackController::playbackStop()");
    m_paused = false;
    if (!m_playing) return; // already stopped

    emit sigStopPlayback();
    emit sigPlaybackStopped();
    m_playing = false;
    m_playback_position = m_playback_start;
}

//***************************************************************************
bool PlaybackController::loop()
{
    return m_loop_mode;
}

//***************************************************************************
bool PlaybackController::running()
{
    return m_playing;
}

//***************************************************************************
bool PlaybackController::paused()
{
    return m_paused;
}

//***************************************************************************
void PlaybackController::setStartPos(unsigned long int pos)
{
    m_playback_start = pos;
}

//***************************************************************************
unsigned long int PlaybackController::startPos()
{
    return m_playback_start;
}

//***************************************************************************
unsigned long int PlaybackController::currentPos()
{
    return m_playback_position;
}

//***************************************************************************
//***************************************************************************
/* end of src/PlaybackController.h */
