/***************************************************************************
       PlayBackPlugin.h  -  plugin for playback and playback configuration
			     -------------------
    begin                : Sun May 13 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#ifndef _PLAY_BACK_PLUGIN_H_
#define _PLAY_BACK_PLUGIN_H_

#include "config.h"

#include <QMutex>
#include <QString>

#include "libkwave/KwavePlugin.h"
#include "libkwave/PlaybackDeviceFactory.h"
#include "libkwave/Sample.h"
#include "PlayBackParam.h"

class QStringList;
class PlaybackController;
class PluginContext;
class PlayBackDevice;
class PlayBackDialog;

class PlayBackPlugin: public Kwave::Plugin, public PlaybackDeviceFactory
{
    Q_OBJECT
public:

    /** Constructor */
    PlayBackPlugin(const PluginContext &c);

    /** Destructor */
    virtual ~PlayBackPlugin();

    /**
     * Gets called when the plugin is first loaded and connects itself
     * to the playback controller and the current signal.
     */
    virtual void load(QStringList &params);

    /** @see Kwave::Plugin::setup() */
    virtual QStringList *setup(QStringList &previous_params);

    /**
     * This plugin needs to be persistent!
     * @see Kwave::Plugin::isPersistent()
     */
    virtual bool isPersistent() { return true; };

    virtual bool isUnique() { return false; };

    /**
     * Does playback in a thread.
     */
    virtual void run(QStringList);

signals:

    /**
     * Signals that playback has stopped.
     */
    void sigPlaybackDone();

    /** Emits the current playback position */
    void sigPlaybackPos(sample_index_t pos);

    /** Emitted after a successful seek operation */
    void sigSeekDone(sample_index_t pos);

public slots:

    /**
     * Starts playback.
     */
    void startDevicePlayBack();

    /**
     * Stops playback.
     */
    void stopDevicePlayBack();

    /**
     * Seek to a new playback position
     * @param pos the new position in samples, absolute
     */
    void seekTo(sample_index_t pos);

    /**
     * Plays a sample sound for testing the playback
     */
    void testPlayBack();

private slots:

    /**
     * Change the playback method
     * @param method the new playback method
     */
    void setMethod(playback_method_t method);

    /**
     * Change the playback device (file) name
     * @param device the filename of the device
     */
    void setDevice(const QString &device);

    /**
     * Closes the playback device, deletes the instance of the
     * PlayBackDevice and sets m_device to 0.
     * @see m_device
     * @see PlayBackDevice
     */
    void closeDevice();

protected:

    /**
     * Interpretes a given parameter list and sets up internal
     * parameters accordingly.
     * @param params reference to a QStringList with parameters
     * @return 0 if ok, or an error code if failed
     */
    int interpreteParameters(QStringList &params);

    /**
     * Create a playback device matching the given playback method.
     * @param method a playback_method_t (aRts, ALSA, OSS...), will
     *               be modified and set to a possible method if the
     *               passed one is not possible
     * @return a new PlayBackDevice or 0 if failed
     */
    PlayBackDevice *createDevice(playback_method_t &method);

    /**
     * Opens and initializes the playback device. If the initialization
     * worked, it returns a valid pointer. On any errors m_device
     * will be 0. If a device was open before, it will be closed.
     * @param name the name of the logical playback device or the name
     *             of the lowlevel device. If null or zero-length, the
     *             default device will be used.
     * @param tracks number of tracks,
     *               if negative use the setting of playback_params
     * @param playback_params points to a class that holds all playback
     *                        parameters. If null, the default parameters
     *                        of the current signal will be used
     * @return a pointer to an opened PlayBackDevice or null if something
     *         failed
     * @see PlayBackDevice
     */
    virtual PlayBackDevice *openDevice(const QString &name, int tracks,
	const PlayBackParam *playback_params);

    /**
     * Returns true if the given device name is supported
     * and can be used for openDevice.
     * @param name the name of a playback device
     * @return true if supported
     * @see openDevice
     */
    virtual bool supportsDevice(const QString &name);

private:

    /** dialog for the playback setup */
    PlayBackDialog *m_dialog;

    /** The playback device used for playback */
    PlayBackDevice *m_device;

    /** Mutex for locking access to the playback device */
    QMutex m_lock_device;

    /** the parameters used for playback */
    PlayBackParam m_playback_params;

    /** reference to the playback controller */
    PlaybackController &m_playback_controller;

    /** Start of the selection when playback started */
    unsigned int m_old_first;

    /** End of the selection when playback started */
    unsigned int m_old_last;

    /** Mutex for locking access to m_should_seek and m_seek_pos */
    QMutex m_lock_seek;

    /** if true, m_seek_pos is valid and a seek has been requested */
    bool m_should_seek;

    /** position to seek to */
    sample_index_t m_seek_pos;

};

//*****************************************************************************
#endif /* _PLAY_BACK_PLUGIN_H_ */
