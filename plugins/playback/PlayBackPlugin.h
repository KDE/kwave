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

#include "PlayBackParam.h" // for struct playback_param_t

class QStringList;
class PlaybackController;
class PluginContext;
class PlayBackDevice;

class PlayBackPlugin: public KwavePlugin
{
    Q_OBJECT
public:

    /** Constructor */
    PlayBackPlugin(PluginContext &c);

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
     * worked, m_device holds a valid pointer. On any errors m_device
     * will be 0. If a device was open before, it will be closed.
     * @see m_device
     * @see PlayBackDevice
     */
    void openDevice();

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
    playback_param_t m_playback_params;

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
