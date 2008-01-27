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

#include "libkwave/InsertMode.h"

class FileInfo;
class KwavePlugin;
class PlaybackController;
class PlaybackDeviceFactory;
class QString;
class QStringList;
class SampleWriter;
class TopWidget;
namespace Kwave { class SampleSink; }

/**
 * Manages the loading, initializing, starting, running and closing
 * of the plugins of kwave. Each instance of a TopWidget creates a
 * new instance of the PluginManager to be independent from other
 * toplevel widgets.
 * @author Thomas Eschenbacher
 */
class PluginManager : public QObject
{
    Q_OBJECT

public:

    /**
     * Constructor.
     * @param parent reference to the toplevel widget (our parent)
     */
    PluginManager(TopWidget &parent);

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
     * Executes a plugin in the context of a given parent widget.
     * @return zero on success or negative error code
     */
    int executePlugin(const QString &name, QStringList *params);

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
     * Returns a reference to the FileInfo object associated with the
     * currently opened file.
     */
    FileInfo &fileInfo();

    /**
     * Returns the length of the current signal in samples.
     * If no signal is present the return value will be 0.
     */
    unsigned int signalLength();

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
    unsigned int selectionStart();

    /**
     * Returns the end of the selection. If nothing is currently
     * selected this will be the last sample (length-1).
     */
    unsigned int selectionEnd();

    /**
     * Sets the current start and length of the selection to new values.
     * @param offset index of the first sample
     * @param length number of samples
     */
    void selectRange(unsigned int offset, unsigned int length);

    /** Returns a reference to our toplevel widget */
    inline TopWidget &topWidget() { return m_top_widget; };

    /**
     * Opens an input stream for a track, starting at a specified sample
     * position. Also handles undo information.
     * @param track index of the track. If the track does not exist, this
     *        function will fail and return 0
     * @param mode specifies where and how to insert
     * @param left start of the input (only useful in insert and
     *             overwrite mode)
     * @param right end of the input (only useful with overwrite mode)
     * @see InsertMode
     */
    SampleWriter *openSampleWriter(unsigned int track, InsertMode mode,
	unsigned int left = 0, unsigned int right = 0);

    /**
     * Opens a Kwave::MultiTrackSink for playback purposes.
     * @param tracks number of tracks
     * @param name of the device, optional. If not given, the default
     *        playback device is used
     * @return a multitrack aRts sink that receives the playback stream
     */
    Kwave::SampleSink *openMultiTrackPlayback(unsigned int tracks,
                                              const QString *name = 0);

    /**
     * Returns a reference to the current playback controller. This is
     * only needed for plugins doing playback.
     */
    PlaybackController &playbackController();

    /**
     * Enqueues a command that will be processed threadsafe in the X11
     * thread. Calls m_spx_command.enqueue().
     */
    void enqueueCommand(const QString &command);

    /**
     * Searches the standard KDE data directories for plugins (through
     * the KDE's standard search algorithm) and creates a map of
     * plugin names and file names. First it collects a list of
     * filenames and then filters it to sort out invalid entries.
     */
    static void findPlugins();

    /**
     * Registers a PlaybackDeviceFactory
     */
    void registerPlaybackDeviceFactory(PlaybackDeviceFactory *factory);

    /**
     * Unregisters a PlaybackDeviceFactory
     */
    void unregisterPlaybackDeviceFactory(PlaybackDeviceFactory *factory);

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

public slots:

    /**
     * Notify all plugins that the signal or file is to be closed
     */
    void signalClosed();

    /**
     * Will be connected to the plugin's "closed" signal.
     * @param p pointer to the plugin to be closed
     */
    void pluginClosed(KwavePlugin *p);

    /**
     * Called if the name of the current signal has changed. This will be
     * forwarded to all plugins by emitting the signal sigSignalChanged.
     * @see #sigSignalChanged()
     */
    void setSignalName(const QString &name);

private slots:

    /** called when a plugin has started (running) it's worker thread */
    void pluginStarted(KwavePlugin *p);

    /** called when a plugin has finished it's worker thread */
    void pluginDone(KwavePlugin *p);

private:

    /** typedef: QPointer to a KwavePlugin */
    typedef QPointer<KwavePlugin> KwavePluginPointer;

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
	PluginDeleter(KwavePlugin *plugin, void *handle);

	/** Destructor, deletes and unloads the plugin */
	virtual ~PluginDeleter();

    private:
	KwavePlugin *m_plugin; /**< Plugin to be deleted */
	void *m_handle;        /**< Handle of the shared object */
    };

    /**
     * Uses the dynamic linker to load a plugin into memory.
     * @param name the name of the plugin (filename)
     * @return pointer to the loaded plugin or zero if the
     *         plugin was not found or invalid
     */
    KwavePlugin *loadPlugin(const QString &name);

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
    void connectPlugin(KwavePlugin *plugin);

    /** connects all signals from and to a plugin */
    void disconnectPlugin(KwavePlugin *plugin);

    /** map for finding plugin files through their name */
    static QMap<QString, QString> m_plugin_files;

    /** list of own loaded plugins */
    PluginList m_loaded_plugins;

    /** global list of loaded unique plugins */
    static PluginList m_unique_plugins;

    /** list of currently running plugins */
    PluginList m_running_plugins;

    /** reference to our parent toplevel widget */
    TopWidget &m_top_widget;

};

#endif /* _PLUGIN_MANAGER_H_ */
