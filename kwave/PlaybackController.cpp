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
    if (m_playing) {
	// first stop playback
	emit sigDeviceStopPlayback();
	emit sigPlaybackStopped();
    }

    // (re)start from beginning without loop mode
    m_playback_position = m_playback_start;
    emit sigPlaybackPos(m_playback_position);

    m_loop_mode = false;
    m_paused = false;
    emit sigDeviceStartPlayback();

    m_playing = true;
    emit sigPlaybackStarted();
}

//***************************************************************************
void PlaybackController::playbackLoop()
{
    if (m_playing) {
	// first stop playback
	emit sigDeviceStopPlayback();
	m_playing = false;
	emit sigPlaybackStopped();
    }

    // (re)start from beginning without loop mode
    m_playback_position = m_playback_start;
    emit sigPlaybackPos(m_playback_position);

    m_loop_mode = true;
    m_paused = false;
    emit sigDeviceStartPlayback();

    m_playing = true;
    emit sigPlaybackStarted();
}

//***************************************************************************
void PlaybackController::playbackPause()
{
    if (!m_playing) return; // no effect if not playing

    m_paused = true;

    // stop playback for now and set the paused flag
    emit sigDeviceStopPlayback();
}

//***************************************************************************
void PlaybackController::playbackContinue()
{
    // if not paused, do the same as start
    if (!m_paused) {
	playbackStart();
	return;
    }

    // else reset the paused flag and start from current position
    emit sigDeviceStartPlayback();

    m_paused = false;
    m_playing=true;

    emit sigPlaybackStarted();
}

//***************************************************************************
void PlaybackController::playbackStop()
{
    // stopped in pause state
    if (m_paused) {
	m_playing = false;
	m_paused = false;
	emit sigPlaybackStopped();
    }
    if (!m_playing) return; // already stopped
    emit sigDeviceStopPlayback();
}

//***************************************************************************
void PlaybackController::updatePlaybackPos(unsigned int pos)
{
    m_playback_position = pos;
    emit sigPlaybackPos(m_playback_position);
}

//***************************************************************************
void PlaybackController::playbackDone()
{
    m_playing = false;
    if (m_paused)
	emit sigPlaybackPaused();
    else {
	m_playback_position = m_playback_start;
	emit sigPlaybackPos(m_playback_position);
	emit sigPlaybackStopped();
    }
}

//***************************************************************************
void PlaybackController::reset()
{
    m_playback_start = 0;
    m_playback_position = 0;
    m_loop_mode = false;
    m_playing = false;
    m_paused = false;

    emit sigPlaybackPos(0);
    emit sigPlaybackStopped();
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
