/***************************************************************************
	    PluginManager.h  -  manager class for kwave's plugins
			     -------------------
    begin                : Sun Aug 27 2000
    copyright            : (C) 2000 by Thomas Eschenbacher
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

#ifndef _PLUGIN_MANAGER_H_
#define _PLUGIN_MANAGER_H_

#include "config.h"

#include <QList>
#include <QListIterator>
#include <QMap>
#include <QMutableListIterator>
#include <QObject>
#include <QPointer>
#include <QVector>

#include <kdemacros.h>

#include "libkwave/InsertMode.h"
#include "libkwave/Sample.h"
#include "libkwave/ViewManager.h"

class FileInfo;
class PlaybackController;
class PlaybackDeviceFactory;
class QString;
class QStringList;
class SignalManager;
class QWidget;
namespace Kwave { class Plugin; }
namespace Kwave { class SampleSink; }
namespace Kwave { class Writer; }

    /**
     * Manages the loading, initializing, starting, running and closing
     * of the plugins of kwave. Each instance of a TopWidget creates a
     * new instance of the PluginManager to be independent from other
     * toplevel widgets.
     */
namespace Kwave {
    class KDE_EXPORT PluginManager : public QObject
    {
	Q_OBJECT

    public:

	/**
	 * Constructor.
	 * @param parent reference to the toplevel widget (our parent)
	 * @param signal_manager reference to a SignalManager
	 */
	PluginManager(QWidget *parent, SignalManager &signal_manager);

	/**
	 * Default destructor
	 */
	virtual ~PluginManager();

	/**
	 * Tries to load all plugins. If a pesistent plugin is found,
	 * it will stay loaded in memory, all other (non-persistent)
	 * plugins will be unloaded afterwards. This also filters out
	 * all plugins that do not correctly load.
	 * @internal used once by each toplevel window at startup
	 */
	void loadAllPlugins();

	/**
	 * Stops all currently running plugins
	 */
	void stopAllPlugins();

	/**
	 * Executes a plugin in the context of a given parent widget.
	 * @return zero on success or negative error code
	 */
	int executePlugin(const QString &name, QStringList *params);

	/**
	 * Returns true if there is no running plugin that blocks
	 * a "close" operation.
	 */
	bool canClose();

	/** Returns true if at least one plugin is currently running */
	bool onePluginRunning();

	/**
	 * Waits until all currently running acions have completed.
	 */
	void sync();

	/**
	 * Loads a plugin, calls it's setup function and then closes
	 * it again. The parameters will be loaded before the setup
	 * and saved if the setup has not been aborted.
	 */
	int setupPlugin(const QString &name);

	/**
	 * Returns the length of the current signal in samples.
	 * If no signal is present the return value will be 0.
	 */
	sample_index_t signalLength();

	/**
	 * Returns the current sample rate in samples per second.
	 * If no signal is present the return value will be 0.
	 */
	double signalRate();

	/**
	 * Returns an array of indices of currently selected channels.
	 */
	const QList<unsigned int> selectedTracks();

	/**
	 * Returns the start of the selection. If nothing is currently
	 * selected this will be the first sample (0).
	 */
	sample_index_t selectionStart();

	/**
	 * Returns the end of the selection. If nothing is currently
	 * selected this will be the last sample (length-1).
	 */
	sample_index_t selectionEnd();

	/**
	 * Sets the current start and length of the selection to new values.
	 * @param offset index of the first sample
	 * @param length number of samples
	 */
	void selectRange(sample_index_t offset, sample_index_t length);

	/**
	 * Opens a Kwave::MultiTrackSink for playback purposes.
	 * @param tracks number of tracks
	 * @param name of the device, optional. If not given, the default
	 *        playback device is used
	 * @return a multitrack sink that receives the playback stream
	 */
	Kwave::SampleSink *openMultiTrackPlayback(unsigned int tracks,
	    const QString *name = 0);

	/**
	 * Returns a reference to the current playback controller. This is
	 * only needed for plugins doing playback.
	 */
	PlaybackController &playbackController();

	/** returns a pointer to the parent widget */
	inline QPointer<QWidget> parentWidget()
	{
	    return m_parent_widget;
	}

	/** returns a reference to our signal manager */
	inline SignalManager &signalManager()
	{
	    return m_signal_manager;
	}

	/**
	 * Insert a new signal view into this widget (or the upper/lower
	 * dock area.
	 * @param view the signal view, must not be a null pointer
	 * @param controls a widget with controls, optionally, can be null
	 */
	void insertView(Kwave::SignalView *view, QWidget *controls);

	/**
	 * registers a view manager, must only be called once!
	 */
	void registerViewManager(Kwave::ViewManager *view_manager);

	/**
	 * Enqueues a command that will be processed threadsafe in the X11
	 * thread.
	 */
	void enqueueCommand(const QString &command);

	/**
	 * Searches the standard KDE data directories for plugins (through
	 * the KDE's standard search algorithm) and creates a map of
	 * plugin names and file names. First it collects a list of
	 * filenames and then filters it to sort out invalid entries.
	 */
	void findPlugins();

	/**
	 * Registers a PlaybackDeviceFactory
	 */
	void registerPlaybackDeviceFactory(PlaybackDeviceFactory *factory);

	/**
	 * Unregisters a PlaybackDeviceFactory
	 */
	void unregisterPlaybackDeviceFactory(PlaybackDeviceFactory *factory);

	/** structure with information about a plugin */
	typedef struct  {
	    QString m_filename;    /**< name of the file           */
	    QString m_author;      /**< name of the author         */
	    QString m_description; /**< verbose name / description */
	    QString m_version;     /**< plugin API version         */
	} PluginInfo;

	/** returns a list with info of all known plugins */
	const QList<PluginInfo> pluginInfoList() const;

    signals:

	/**
	 * Forwards commands to the parent TopWidget execute a command
	 */
	void sigCommand(const QString &command);

	/**
	 * Informs all plugins and client windows that we close down
	 */
	void sigClosed();

	/**
	 * Informs the plugins that the name of the signal has changed.
	 * This might be used to update the caption of a window.
	 */
	void sigSignalNameChanged(const QString &name);

	/**
	 * informs about progress, e.g. for showing a message in
	 * a splash screen or status bar.
	 */
	void sigProgress(const QString &message);

    public slots:

	/**
	 * Notify all plugins that the signal or file is to be closed
	 */
	void signalClosed();

	/**
	 * Will be connected to the plugin's "closed" signal.
	 * @param p pointer to the plugin to be closed
	 */
	void pluginClosed(Kwave::Plugin *p);

	/**
	 * Called if the name of the current signal has changed. This will be
	 * forwarded to all plugins by emitting the signal sigSignalNameChanged.
	 * @see sigSignalNameChanged()
	 */
	void setSignalName(const QString &name);

    private slots:

	/** called when a plugin has started (running) it's worker thread */
	void pluginStarted(Kwave::Plugin *p);

	/** called when a plugin has finished it's worker thread */
	void pluginDone(Kwave::Plugin *p);

    private:

	/** typedef: QPointer to a Kwave::Plugin */
	typedef QPointer<Kwave::Plugin> KwavePluginPointer;

	/** typedef: list of pointers to kwave plugins */
	typedef QList< KwavePluginPointer > PluginList;

	/** typedef: mutable iterator for PluginList */
	typedef QMutableListIterator< KwavePluginPointer >
	    PluginListMutableIterator;

	/** typedef: const iterator for PluginList */
	typedef QListIterator< KwavePluginPointer >
	    PluginListIterator;

    private:

	/**
	 * Helper class for deferred deleting and unloading a plugin
	 * @internal
	 */
	class PluginDeleter: public QObject
	{
	public:
	    /** Constructor, stores data for later removal */
	    PluginDeleter(Kwave::Plugin *plugin, void *handle);

	    /** Destructor, deletes and unloads the plugin */
	    virtual ~PluginDeleter();

	private:
	    KwavePluginPointer m_plugin; /**< Plugin to be deleted */
	    void *m_handle;              /**< Handle of the shared object */
	};

	/**
	 * Uses the dynamic linker to load a plugin into memory.
	 * @param name the name of the plugin (filename)
	 * @return pointer to the loaded plugin or zero if the
	 *         plugin was not found or invalid
	 */
	Kwave::Plugin *loadPlugin(const QString &name);

	/**
	 * loads a plugin's default parameters from the user's
	 * configuration file. If nothing is found in the config file,
	 * the return value will be 0. If the current version number of
	 * the plugin does not match the version number in the config file,
	 * the return value will also be 0.
	 * @param name the name of the plugin
	 * @param version the version number of the plugin
	 * @return list of strings
	 */
	QStringList loadPluginDefaults(const QString &name,
				    const QString &version);

	/**
	 * Saves a plugin's default parameters to the user's configuration
	 * file. The whole section in the configuration file will be deleted
	 * before saving the new settings in order to wipe out invalid
	 * entries and settings that belong to an older version.
	 * @param name the name of the plugin
	 * @param version the version number of the plugin
	 * @param params a list of configuration strings
	 */
	void savePluginDefaults(const QString &name, const QString &version,
				QStringList &params);

	/** connects all signals of and for a plugin */
	void connectPlugin(Kwave::Plugin *plugin);

	/** connects all signals from and to a plugin */
	void disconnectPlugin(Kwave::Plugin *plugin);

    private:

	/**
	 * map with plugin information: key = short name of the plugin,
	 * data = plugin info (description, author, version etc...)
	 */
	static QMap<QString, PluginInfo> m_found_plugins;

	/** list of own loaded plugins */
	PluginList m_loaded_plugins;

	/** global list of loaded unique plugins */
	static PluginList m_unique_plugins;

	/** list of currently running plugins */
	PluginList m_running_plugins;

	/** reference to our parent toplevel widget */
	QPointer<QWidget> m_parent_widget;

	/** reference to our signal manager */
	SignalManager &m_signal_manager;

	/** interface for registering a SignalView */
	ViewManager *m_view_manager;

    };
}

#endif /* _PLUGIN_MANAGER_H_ */
