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

#include <qstring.h>
#include <mt/SignalProxy.h>
#include <libkwave/PlayBackParam.h> // for struct playback_param_t
#include <libgui/KwavePlugin.h>

class QStringList;
class PlaybackController;
class PluginContext;

class PlayBackPlugin: public KwavePlugin
{
    Q_OBJECT
public:

    /** Constructor */
    PlayBackPlugin(PluginContext &c);

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

private:

    /**
     * Internally used for setting up the OSS playback device.
     * @param audio file descriptor of the opened playback device
     * @param channels number of output channels
     * @param rate playback rate [bits/second]
     * @param bufbase exponent of playback buffer size (buffer size
     *        will be (2 ^ bufbase)
     * @return size of the playback buffer if successful, zero on errors
     */
    int setSoundParams(int audio, int bitspersample, unsigned int channels,
	int rate, int bufbase);

private:

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
};

//*****************************************************************************
#endif /* _PLAY_BACK_PLUGIN_H_ */
