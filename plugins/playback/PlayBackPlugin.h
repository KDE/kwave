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

public slots:

    /**
     * Starts playback.
     */
    void startDevicePlayBack();

    /**
     * Stops playback.
     */
    void stopDevicePlayBack();

protected:

    /**
     * Interpretes a given parameter list and sets up internal
     * parameters accordingly.
     * @param params reference to a QStringList with parameters
     * @return 0 if ok, or an error code if failed
     */
    int interpreteParameters(QStringList &params);

private:
    /** the parameters used for playback */
    playback_param_t m_playback_params;

};

//*****************************************************************************
#endif /* _PLAY_BACK_PLUGIN_H_ */
