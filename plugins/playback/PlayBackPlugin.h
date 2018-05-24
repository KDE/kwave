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

#ifndef PLAY_BACK_PLUGIN_H
#define PLAY_BACK_PLUGIN_H

#include "config.h"

#include <QMutex>
#include <QPointer>
#include <QString>

#include "libkwave/PlayBackParam.h"
#include "libkwave/PlaybackDeviceFactory.h"
#include "libkwave/Plugin.h"
#include "libkwave/Sample.h"

class QStringList;

namespace Kwave
{
    class PlaybackController;
    class PlayBackDevice;
    class PlayBackDialog;
    class PluginContext;
    class SampleSink;

    class PlayBackPlugin :public Kwave::Plugin,
                          public Kwave::PlaybackDeviceFactory
    {
	Q_OBJECT
    public:

	/**
	 * Constructor
	 * @param parent reference to our plugin manager
	 * @param args argument list [unused]
	 */
	PlayBackPlugin(QObject *parent, const QVariantList &args);

	/** Destructor */
        virtual ~PlayBackPlugin() Q_DECL_OVERRIDE;

	/**
	 * Gets called when the plugin is first loaded and connects itself
	 * to the playback controller and the current signal.
	 */
        virtual void load(QStringList &params) Q_DECL_OVERRIDE;

	/**
	 * Gets called before the plugin is unloaded.
	 */
        virtual void unload() Q_DECL_OVERRIDE;

	/** @see Kwave::Plugin::setup() */
        virtual QStringList *setup(QStringList &previous_params)
            Q_DECL_OVERRIDE;

	/**
	 * Starts a playback test sequence
	 * @param params list of strings with parameters (unused)
	 */
        virtual void run(QStringList params) Q_DECL_OVERRIDE;

    signals:

	/** emits the progress of the playback test, from thread context */
	void sigTestProgress(int percent);

    public slots:

	/**
	 * Plays a sample sound for testing the playback
	 */
	void testPlayBack();

    protected:

	/**
	 * Interpretes a given parameter list and sets up internal
	 * parameters accordingly.
	 * @param params reference to a QStringList with parameters
	 * @return the detected playback parameters
	 */
	Kwave::PlayBackParam interpreteParameters(QStringList &params);

	/**
	 * Create a playback device matching the given playback method.
	 * @param method a playback_method_t (aRts, ALSA, OSS...)
	 * @return a new PlayBackDevice or 0 if failed
	 */
        virtual Kwave::PlayBackDevice *createDevice(Kwave::playback_method_t method)
            Q_DECL_OVERRIDE;

	/**
	 * Returns a list of supported playback methods.
	 * @return list of all supported playback methods, should not contain
	 *         "any" or "invalid"
	 */
        virtual QList<Kwave::playback_method_t> supportedMethods() Q_DECL_OVERRIDE;

    private:

	/** dialog for the playback setup */
	QPointer<Kwave::PlayBackDialog> m_dialog;

	/** reference to the playback controller */
	Kwave::PlaybackController &m_playback_controller;

	/** sample sink, for playback test */
	Kwave::SampleSink *m_playback_sink;
    };
}

#endif /* PLAY_BACK_PLUGIN_H */

//***************************************************************************
//***************************************************************************
