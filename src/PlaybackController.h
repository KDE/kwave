/***************************************************************************
       PlaybackController.h  -  Interface for generic playback control
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

#ifndef _PLAYBACK_CONTROLLER_H_
#define _PLAYBACK_CONTROLLER_H_

#include <qobject.h>

/**
 * \class PlaybackController
 * Provides a generic interface for classes that can contol playback
 * with start, stop, pause and continue. This class is intended to be used
 * or derived in a class that is able to control a playback device by
 * simply startig or stopping playback and can be easily used by some
 * other part of the program that has nothing to do directly with
 * playback. The playback control functions all start with "playback"
 * and are slots that are intended to be connected to some simple
 * gui elements like toolbar buttons or menu entries.
 *
 * This class internally manages the logic and handling of the
 * playback position.
 */
class PlaybackController: public QObject
{
Q_OBJECT

public:

    /** Default constructor */
    PlaybackController();

    /** Destructor */
    virtual ~PlaybackController();

public:

    /** returns the loop mode flag */
    bool loop();

    /** returns true if the playback is running */
    bool running();

    /** returns true if the playback is paused */
    bool paused();

    /** sets a new start position */
    void setStartPos(unsigned long int pos);

    /** returns the position where the playback starts */
    unsigned long int startPos();

    /** returns the current position of the playback pointer */
    unsigned long int currentPos();

public slots:

    /**
     * (Re-)starts the playback. If playback has successfully been
     * started, the signal sigPlaybackStarted() will be emitted.
     */
    void playbackStart();

    /**
     * (Re-)starts the playback in loop mode (like with playbackStart().
     * Also emitts sigPlaybackStarted() if playback has successfully
     * been started.
     */
    void playbackLoop();

    /**
     * Pauses the playback. Causes sigPlaybackDone() to be emitted if
     * the current buffer has played out. The current playback pointer
     * will stay at it's current position.
     */
    void playbackPause();

    /**
     * Continues the playback at the position where it has been stopped
     * by the playbackPause() command. If the last playback pointer
     * has become invalid or is not available (less 0), this function
     * will do the same as playbackStart(). This also emits the
     * signal sigPlaybackStarted().
     */
    void playbackContinue();

    /**
     * Stopps playback / loop. Like playbackPause(), but resets the
     * playback pointer back to the start.
     */
    void playbackStop();

signals:

    /**
     * Signals that playback should be started.
     */
    void sigStartPlayback();

    /**
     * Signals that playback should be stopped.
     */
    void sigStopPlayback();

    /**
     * Signals that playback has started.
     */
    void sigPlaybackStarted();

    /**
     * Signals that playback has been paused.
     */
    void sigPlaybackPaused();

    /**
     * Signals that playback has stopped.
     */
    void sigPlaybackStopped();

private:
    /** if set to true, the playback will be done in loop mode */
    bool m_loop_mode;

    /** if true, playback is only paused and can be continued */
    bool m_paused;

    /** is set to true if the playback has been started */
    bool m_playing;

    /** the current play position */
    unsigned long int m_playback_position;

    /** the start position for playback */
    unsigned long int m_playback_start;

};

#endif /* _PLAYBACK_CONTROLLER_H_ */
