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
#define _PLUGIN_MANAGER_H_ 1

#include <qobject.h>
#include <qarray.h>
#include <qlist.h>

#include "mt/SignalProxy.h"

class KwavePlugin;
class QBitmap;
class QStrList;
class TopWidget;

/**
 * Manages the loding, initializing, starting, running and closing
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
     * Executes a plugin in the context of a given parent widget.
     */
    void executePlugin(const char *name, QStrList *params);

    /**
     * Returns the length of the current signal in samples.
     * If no signal is present the return value will be 0.
     */
    unsigned int signalLength();

    /**
     * Returns the current sample rate in samples per second.
     * If no signal is present the return value will be 0.
     */
    unsigned int signalRate();

    /**
     * Returns an array of indices of currently selected channels.
     */
    const QArray<unsigned int> selectedChannels();

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
     * Returns the value of one single sample of a specified channel.
     * If the channel does not exist or the index of the sample is
     * out of range the return value will be zero.
     */
    int singleSample(unsigned int channel, unsigned int offset);

    /**
     * Returns the value of one single sample averaged over all active channels.
     * If no channel do exist or the index of the sample is out of range the
     * return value will be zero. If the optional list of channels is omitted,
     * the sample will be averaged over all currently selected channels.
     * @param offset sample offset [0...length-1]
     * @param channels an array of channel numbers, optional
     * @return value of the sample
     */
    int averageSample(unsigned int offset,
                      const QArray<unsigned int> *channels = 0);

    /**
     * Returns a QBitmap with an overview of all currently present
     * signals.
     */
    QBitmap *overview(unsigned int width, unsigned int height,
                      unsigned int offset, unsigned int length);

signals:
    /**
     * Forwards commands to the parent TopWidget execute a command
     */
    void sigCommand(const char *command);

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
     * @param remove if true, the plugin will also be deleted,
     *        if false the plugin is expected to delete itself
     */
    void pluginClosed(KwavePlugin *p, bool remove);

    /**
     * Called if the name of the current signal has changed. This will be
     * forwarded to all plugins by emitting the signal sigSignalChanged.
     * @see #sigSignalChanged()
     */
    void setSignalName(const QString &name);

private slots:
    /** emits sigSignalNameChanged to the plugins (threadsafe) */
    void emitNameChanged();

private:

    /**
     * Uses the dynamic linker to load a plugin into memory.
     * @param name the name of the plugin (filename)
     * @return memory handle of the loaded plugin or zero if the
     *         plugin was not found or invalid
     */
    void *loadPlugin(const char *name);

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
    QStrList *loadPluginDefaults(const QString &name,
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
                            QStrList &params);

    /** connects all signals of and for a plugin */
    void connectPlugin(KwavePlugin *plugin);

    /** connects all signals from and to a plugin */
    void disconnectPlugin(KwavePlugin *plugin);

    /** threadsafe signal proxy for setSignalName / sigSignalNameChanged */
    SignalProxy1<const QString> *m_spx_name_changed;

    /** list of loaded plugins */
    QList<KwavePlugin> m_loaded_plugins;

    /** reference to our parent toplevel widget */
    TopWidget &m_top_widget;
};

#endif // _PLUGIN_MANAGER_H_
