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
#include <qstring.h>

#include "mt/Mutex.h"
#include "mt/SignalProxy.h"
#include "libkwave/KwavePlugin.h"
#include "libkwave/PlaybackDeviceFactory.h"
#include "PlayBackParam.h"

class QStringList;
class PlaybackController;
class PluginContext;
class PlayBackDevice;

class PlayBackPlugin: public KwavePlugin, public PlaybackDeviceFactory
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

    /** @see KwavePlugin::setup() */
    virtual QStringList *setup(QStringList &previous_params);

    /**
     * This plugin needs to be persistent!
     * @see KwavePlugin::isPersistent()
     */
    virtual bool isPersistent() { return true; };

    virtual bool isUnique() { return false; };

    /**
     * Does playback in a thread.
     */
    virtual void run(QStringList);

public slots:

    /**
     * Starts playback.
     */
    void startDevicePlayBack();

    /**
     * Stops playback.
     */
    void stopDevicePlayBack();

private slots:
    /**
     * Called from the playback thread to notify about a new
     * playback pointer.
     * @internal
     */
    void updatePlaybackPos();

protected:

    /**
     * Interpretes a given parameter list and sets up internal
     * parameters accordingly.
     * @param params reference to a QStringList with parameters
     * @return 0 if ok, or an error code if failed
     */
    int interpreteParameters(QStringList &params);

    /**
     * Calls the playback controller's playbackDone() slot through a
     * threadsafe signal proxy. This is used from inside the playback
     * thread to signal that the end of playback has been reached or
     * from the start/stopPlayback methods.
     */
    void playbackDone();

    /**
     * Opens and initializes the playback device. If the initialization
     * worked, it returns a valid pointer. On any errors m_device
     * will be 0. If a device was open before, it will be closed.
     * @param name the name of the logical playback device or the name
     *             of the lowlevel device. If null or zero-length, the
     *             default device will be used.
     * @param playback_params points to a class that holds all playback
     *                        parameters. If null, the default parameters
     *                        of the current signal will be used
     * @return a pointer to an opened PlayBackDevice or null if something
     *         failed
     * @see PlayBackDevice
     */
    PlayBackDevice *openDevice(const QString &name,
	const PlayBackParam *playback_params);

    /**
     * Returns true if the given device name is supported
     * and can be used for openDevice.
     * @param name the name of a playback device
     * @return true if supported
     * @see openDevice
     */
    virtual bool supportsDevice(const QString &name);

    /**
     * Closes the playback device, deletes the instance of the
     * PlayBackDevice and sets m_device to 0.
     * @see m_device
     * @see PlayBackDevice
     */
    void closeDevice();

private:

    /** The playback device used for playback */
    PlayBackDevice *m_device;

    /** Mutex for locking acces to the playback device */
    Mutex m_lock_device;

    /** the parameters used for playback */
    PlayBackParam m_playback_params;

    /** reference to the playback controller */
    PlaybackController &m_playback_controller;

    /**
     * Signal proxy that signals the end of playback out
     * of the playback thread.
     */
    SignalProxy<void> m_spx_playback_done;

    /**
     * Signal proxy that brings the current playback position
     * out of the playback thread.
     */
    SignalProxy1<unsigned int> m_spx_playback_pos;

    /** command flag for stopping the playback thread */
    bool m_stop;

    /** Start of the selection when playback started */
    unsigned int m_old_first;

    /** End of the selection when playback started */
    unsigned int m_old_last;

};

//*****************************************************************************
#endif /* _PLAY_BACK_PLUGIN_H_ */
