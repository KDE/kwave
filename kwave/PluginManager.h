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
#include <qobject.h>
#include <qarray.h>
#include <qlist.h>
#include <qvector.h>
#include <qmap.h>

#include <arts/artsflow.h>

#include "mt/SignalProxy.h"
#include "libkwave/InsertMode.h"

class FileInfo;
class KwavePlugin;
class PlaybackController;
class QBitmap;
class QString;
class QStringList;
class SampleWriter;
class TopWidget;
class UndoAction;
class MultiTrackReader;
class MultiTrackWriter;

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
     * Returns true if this instance was successfully initialized, or
     * false if something went wrong during initialization.
     */
    virtual bool isOK();

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
    const QArray<unsigned int> selectedTracks();

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
     * Returns a set of opened SampleReader objects for reading from
     * multiple tracks. The list of tracks may contain indices of tracks
     * in any order and even duplicate entries are allowed. One useful
     * application is passing the output of selectedTracks() in order
     * to read from only from selected tracks.
     * @param readers reference to the MultiTrackReader to be filled.
     * @note the returned vector has set "autoDelete" to true, so you
     *       don't have to care about cleaning up
     * @param track_list array of indices of tracks for reading
     * @param first index of the first sample
     * @param last index of the last sample
     * @see SampleReader
     * @see selectedTracks()
     */
    void openMultiTrackReader(MultiTrackReader &readers,
	const QArray<unsigned int> &track_list,
	unsigned int first, unsigned int last);

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
     * Opens a set of SampleWriters and internally handles the creation of
     * needed undo information. This is useful for multi-channel operations.
     * @param writers reference to a vector that receives all writers.
     * @param track_list list of indices of tracks to be modified.
     * @param mode specifies where and how to insert
     * @param left start of the input (only useful in insert and
     *             overwrite mode)
     * @param right end of the input (only useful with overwrite mode)
     * @see InsertMode
     */
    void openMultiTrackWriter(MultiTrackWriter &writers,
	const QArray<unsigned int> &track_list, InsertMode mode,
	unsigned int left, unsigned int right);

    /**
     * Opens a set of SampleWriters using the currently selected list of
     * tracks and the current selection. If nothing is selected, the whole
     * signal will be selected.
     * @param writers reference to a vector that receives all writers.
     * @param mode specifies where and how to insert
     */
    void openMultiTrackWriter(MultiTrackWriter &writers,
                              InsertMode mode);

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

    /** Returns a reference to the global aRts dispatcher */
    inline Arts::Dispatcher *artsDispatcher() {
	return Arts::Dispatcher::the();
    };

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

    /** emits sigSignalNameChanged to the plugins (threadsafe) */
    void emitNameChanged();

    /**
     * de-queues a command from m_spx_command and executes it
     * through the toplevel widget.
     */
    void forwardCommand();

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
     * @param var the version number of the plugin
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
     * @param ver the version number of the plugin
     * @param param a list of configuration strings
     */
    void savePluginDefaults(const QString &name, const QString &version,
                            QStringList &params);

    /** connects all signals of and for a plugin */
    void connectPlugin(KwavePlugin *plugin);

    /** connects all signals from and to a plugin */
    void disconnectPlugin(KwavePlugin *plugin);

    /** threadsafe signal proxy for setSignalName / sigSignalNameChanged */
    SignalProxy1<const QString> m_spx_name_changed;

    /** threadsafe proxy for sigCommand() */
    SignalProxy1<const QString> m_spx_command;

    /** map for finding plugin files through their name */
    static QMap<QString, QString> m_plugin_files;

    /** list of loaded plugins */
    QList<KwavePlugin> m_loaded_plugins;

    /** reference to our parent toplevel widget */
    TopWidget &m_top_widget;

};

#endif /* _PLUGIN_MANAGER_H_ */
