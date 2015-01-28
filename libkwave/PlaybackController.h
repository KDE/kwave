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

#include "config.h"

#include <QtCore/QList>
#include <QtCore/QMutex>
#include <QtCore/QObject>

#include <kdemacros.h>

#include "libkwave/PlayBackParam.h"
#include "libkwave/Runnable.h"
#include "libkwave/Sample.h"
#include "libkwave/WorkerThread.h"

namespace Kwave
{

    class PlayBackDevice;
    class PlaybackDeviceFactory;
    class SignalManager;

    /**
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
    class KDE_EXPORT PlaybackController: public QObject, public Kwave::Runnable
    {
    Q_OBJECT

    public:

	/** Default constructor */
	explicit PlaybackController(Kwave::SignalManager &signal_manager);

	/** Destructor */
	virtual ~PlaybackController();

    public:

	/** resets start, current position, loop, pause and running flag */
	void reset();

	/** returns the loop mode flag */
	bool loop() const;

	/** returns true if the playback is running */
	bool running() const;

	/** returns true if the playback is paused */
	bool paused() const;

	/** sets a new start position */
	void setStartPos(sample_index_t pos);

	/** sets a new end position */
	void setEndPos(sample_index_t pos);

	/** returns the position where the playback starts */
	sample_index_t startPos() const;

	/** returns the position where the playback ends */
	sample_index_t endPos() const;

	/** returns the current position of the playback pointer */
	sample_index_t currentPos() const;

	/**
	 * Registers a PlaybackDeviceFactory
	 */
	void registerPlaybackDeviceFactory(
	    Kwave::PlaybackDeviceFactory *factory);

	/**
	 * Unregisters a PlaybackDeviceFactory
	 */
	void unregisterPlaybackDeviceFactory(
	    Kwave::PlaybackDeviceFactory *factory);

	/**
	 * Create a playback device matching the given playback method.
	 *
	 * @param method a playback_method_t (e.g. Pulse, Phonon, ALSA, OSS...)
	 * @return a new PlayBackDevice or 0 if failed
	 */
	virtual Kwave::PlayBackDevice *createDevice(
	    Kwave::playback_method_t method);

	/**
	 * Creates, opens and initializes a playback device.
	 *
	 * @param tracks number of tracks,
	 *               if negative use the setting of playback_params
	 * @param playback_params points to a structure with playback
	 *                        parameters. If null, the default parameters
	 *                        of the current signal will be used
	 * @return a pointer to an opened PlayBackDevice or null if failed
	 * @see PlayBackDevice
	 */
	Kwave::PlayBackDevice *openDevice(int tracks,
	    const Kwave::PlayBackParam *playback_params);

	/**
	 * Sets default playback parameters, for use next time playback
	 * is started
	 * @param params new playback parameters
	 */
	void setDefaultParams(const Kwave::PlayBackParam &params);

	/**
	 * Checks whether a playback method is supported and returns the
	 * next best match if not.
	 * @param method reference to a playback method, can be modified
	 */
	void checkMethod(Kwave::playback_method_t &method);

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

	/**
	 * If playback is currently running, it will be paused and
	 * then restarted with current track and time selection.
	 */
	void reload();

	/** Seeks to a new position */
	void seekTo(sample_index_t pos);

	/** Called when the seek has finished */
	void seekDone(sample_index_t pos);

	/** Updates the current playback position */
	void updatePlaybackPos(sample_index_t pos);

	/** Updates the status if playback is done */
	void playbackDone();

    signals:

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

	/**
	 * Emits the current position of the playback pointer
	 */
	void sigPlaybackPos(sample_index_t pos);

	/**
	 * Emits the current position after a seek operation
	 */
	void sigSeekDone(sample_index_t pos);

	/**
	 * Signals that playback has stopped (sent from worker thread).
	 */
	void sigDevicePlaybackDone();

	/** Emits the current playback position (from worker thread) */
	void sigDevicePlaybackPos(sample_index_t pos);

	/** Emitted after a successful seek operation (from worker thread)*/
	void sigDeviceSeekDone(sample_index_t pos);

    private slots:

	/**
	 * Closes the playback device, deletes the instance of the
	 * PlayBackDevice and sets m_device to 0.
	 * @see m_device
	 * @see PlayBackDevice
	 */
	void closeDevice();

	/** updates the mixer matrix if the track selection has changed */
	void trackSelectionChanged();

    protected:

	/** wrapper for our run() function, called from worker thread */
	virtual void run_wrapper(const QVariant &params);

    private:

	/** Starts playback device (and worker thread) */
	void startDevicePlayBack();

	/** Stops the playback device (and worker thread) */
	void stopDevicePlayBack();

    private:

	/** Reference to our signal manager */
	Kwave::SignalManager &m_signal_manager;

	/**
	 * Thread that executes the run() member function.
	 */
	Kwave::WorkerThread m_thread;

	/** The playback device used for playback */
	Kwave::PlayBackDevice *m_device;

	/** Mutex for locking access to the playback device */
	QMutex m_lock_device;

	/** the parameters used for playback */
	Kwave::PlayBackParam m_playback_params;

	/**
	 * Mutex for locking access to members that control the playback
	 * loop, like m_should_seek, m_seek_pos and m_mixer
	 */
	QMutex m_lock_playback;

	/** if true, m_seek_pos is valid and a seek has been requested */
	bool m_should_seek;

	/** position to seek to */
	sample_index_t m_seek_pos;

	/** notification flag, true if the track selection has changed */
	bool m_track_selection_changed;

	/**
	 * If true, we are in "reload" mode. In this mode the playback is
	 * paused and continued without emitting a sigPlaybackDone. This
	 * is useful if playback parameters or signal selection has changed
	 * during playback.
	 */
	bool m_reload_mode;

	/** if set to true, the playback will be done in loop mode */
	bool m_loop_mode;

	/** if true, playback is only paused and can be continued */
	bool m_paused;

	/** is set to true if the playback has been started */
	bool m_playing;

	/** the current play position */
	sample_index_t m_playback_position;

	/** the start position for playback */
	sample_index_t m_playback_start;

	/** the end position for playback */
	sample_index_t m_playback_end;

	/** Start of the selection when playback started */
	sample_index_t m_old_first;

	/** End of the selection when playback started */
	sample_index_t m_old_last;

	/** list of playback device factories */
	QList<Kwave::PlaybackDeviceFactory *> m_playback_factories;

    };
}

#endif /* _PLAYBACK_CONTROLLER_H_ */

//***************************************************************************
//***************************************************************************
