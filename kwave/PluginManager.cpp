/***************************************************************************
       PluginManager.cpp -  manager class for Kwave's plugins
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

#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>

#include "qobject.h"

#include <kglobal.h>
#include <kconfig.h>
#include <kmainwindow.h>
#include <kstddirs.h>
#include <kmessagebox.h>
#include <klocale.h>

#include "mt/SignalProxy.h"
#include "mt/TSS_Object.h"
#include "mt/ThreadCondition.h"

#include "libkwave/ArtsMultiPlaybackSink.h"
#include "libkwave/LineParser.h"
#include "libkwave/FileLoader.h"
#include "libkwave/KwavePlugin.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/Parser.h"
#include "libkwave/PlayBackDevice.h"
#include "libkwave/PlaybackDeviceFactory.h"
#include "libkwave/PluginContext.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SampleWriter.h"

#include "KwaveApp.h"
#include "TopWidget.h"
#include "SignalManager.h"
#include "UndoAction.h"
#include "UndoModifyAction.h"
#include "UndoTransactionGuard.h"

#include "PluginManager.h"

//***************************************************************************
//***************************************************************************
PluginManager::PluginDeleter::PluginDeleter(KwavePlugin *plugin, void *handle)
  :QObject(), m_plugin(plugin), m_handle(handle)
{
}

//***************************************************************************
PluginManager::PluginDeleter::~PluginDeleter()
{
    // delete the plugin, this should also remove everything it has allocated
    delete m_plugin;

    // now the handle of the shared object can be cleared too
    dlclose(m_handle);
}

//****************************************************************************
//****************************************************************************

// static initializers
QMap<QString, QString> PluginManager::m_plugin_files;
QPtrList<KwavePlugin> PluginManager::m_unique_plugins;
static QPtrList<PlaybackDeviceFactory> m_playback_factories;

//****************************************************************************
PluginManager::PluginManager(TopWidget &parent)
    :m_spx_name_changed(this, SLOT(emitNameChanged())),
     m_spx_command(this, SLOT(forwardCommand())),
     m_loaded_plugins(), m_running_plugins(),
     m_top_widget(parent)
{
    // use all unique plugins
    // this does nothing on the first instance, all other instances
    // will probably find a non-empty list
    QPtrListIterator<KwavePlugin> itp(m_unique_plugins);
    for ( ; itp.current(); ++itp ) {
	KwavePlugin *p = itp.current();
	Q_ASSERT(p && p->isUnique());
	if (p && p->isUnique()) {
	    p->use();

	    // maybe we will become responsible for releasing
	    // the plugin (when it is in use but the plugin manager
	    // who has created it is already finished)
	    connect(p,    SIGNAL(sigClosed(KwavePlugin *)),
	            this, SLOT(pluginClosed(KwavePlugin *)));
	}
    }
}

//****************************************************************************
bool PluginManager::isOK()
{
    return true;
}

//****************************************************************************
PluginManager::~PluginManager()
{
    // inform all plugins and client windows that we close now
    emit sigClosed();

    // release all unique plugins
    QPtrListIterator<KwavePlugin> itu(m_unique_plugins);
    for (itu.toLast() ; itu.current(); ) {
	KwavePlugin *p = itu.current();
	--itu;
	Q_ASSERT(p && p->isUnique());
	if (p && p->isUnique()) {
	    p->release();
	}
    }

    // release all own persisiten plugins
    QPtrListIterator<KwavePlugin> itp(m_loaded_plugins);
    for (itp.toLast() ; itp.current(); ) {
	KwavePlugin *p = itp.current();
	--itp;
	Q_ASSERT(p);
	if (p && p->isPersistent()) p->release();
    }

    // release all own plugins that are left
    QPtrListIterator<KwavePlugin> it(m_loaded_plugins);
    for (it.toLast() ; it.current(); ) {
	KwavePlugin *p = it.current();
	--it;
	Q_ASSERT(p);
	if (p) p->release();
    }
}

//***************************************************************************
void PluginManager::loadAllPlugins()
{
    // Try to load all plugins. This has to be called only once per
    // instance of the main window!
    // NOTE: this also implicitely makes it resident if it is persistent or unique
    QMap<QString, QString>::Iterator it;
    for (it=m_plugin_files.begin(); it != m_plugin_files.end(); ++it) {
	QString name = it.key();

	KwavePlugin *plugin = loadPlugin(name);
	if (plugin) {
// 	    QString state = "";
// 	    if (plugin->isPersistent()) state += "(persistent)";
// 	    if (plugin->isUnique()) state += "(unique)";
// 	    if (!state.length()) state = "(normal)";
// 	    qDebug("PluginManager::loadAllPlugins(): plugin '"+
// 		   plugin->name()+"' "+state);
	    if (!plugin->isUnique() && !plugin->isPersistent()) {
		// remove it again if it is neither unique nor persistent
		plugin->release();
	    }
	} else {
	    // loading failed => remove it from the list
	    qWarning("PluginManager::loadAllPlugins(): removing '%s' "\
	            "from list", name.latin1());
	    m_plugin_files.remove(name);
	}
    }
}

//***************************************************************************/
KwavePlugin *PluginManager::loadPlugin(const QString &name)
{

    // first find out if the plugin is already loaded and persistent
    QPtrListIterator<KwavePlugin> itp(m_loaded_plugins);
    for ( ; itp.current(); ++itp ) {
	KwavePlugin *p = itp.current();
	if (p->isPersistent() && (p->name() == name)) {
	    Q_ASSERT(p->isPersistent());
	    qDebug("PluginManager::loadPlugin('"+name+
	           "') -> returning pointer to persistent");
	    return p;
	}
    }

    // find out if the plugin is already loaded and unique
    QPtrListIterator<KwavePlugin> it(m_unique_plugins);
    for ( ; it.current(); ++it ) {
	KwavePlugin *p = it.current();
	if (p->name() == name) {
	    Q_ASSERT(p->isUnique());
	    Q_ASSERT(p->isPersistent());
	    qDebug("PluginManager::loadPlugin('"+name+
	           "') -> returning pointer to unique+persistent");
	    return p;
	}
    }

    // show a warning and abort if the plugin is unknown
    if (!(m_plugin_files.contains(name))) {
	QString message =i18n("oops, plugin '%1' is unknown or invalid!");
	message = message.arg(name);
	KMessageBox::error(&m_top_widget, message,
	    i18n("error on loading plugin"));
	return 0;
    }
    QString &filename = m_plugin_files[name];

    // try to get the file handle of the plugin's binary
    void *handle = dlopen(filename, RTLD_NOW);
    if (!handle) {
	QString message = i18n("unable to load the file \n'%1'\n"\
	                       " that contains the plugin '%2' !");
	message = message.arg(filename).arg(name);
	KMessageBox::error(&m_top_widget, message,
	    i18n("error on loading plugin"));
	return 0;
    }

    // get the loader function
    KwavePlugin *(*plugin_loader)(PluginContext *c) = 0;

    // hardcoded, should always work when the
    // symbols are declared as extern "C"
    const char *sym_version = "version";
    const char *sym_author  = "author";
    const char *sym_loader  = "load";

    // get the plugin's author
    const char *author = "";
    const char **pauthor = (const char **)dlsym(handle, sym_author);
    Q_ASSERT(pauthor);
    if (pauthor) author=*pauthor;
    if (!author) author = i18n("(unknown)");

    // get the plugin's version string
    const char *version = "";
    const char **pver = (const char **)dlsym(handle, sym_version);
    Q_ASSERT(pver);
    if (pver) version=*pver;
    if (!version) version = i18n("(unknown)");

    plugin_loader =
        (KwavePlugin *(*)(PluginContext *))dlsym(handle, sym_loader);
    Q_ASSERT(plugin_loader);
    if (!plugin_loader) {
	// plugin is null, out of memory or not found
	qWarning("PluginManager::loadPlugin('"+name+"'): "\
		"plugin does not contain a loader, "\
		"maybe it is damaged or the wrong version?");
	dlclose(handle);
	return 0;
    }

    PluginContext *context = new PluginContext(
    m_top_widget.getKwaveApp(),
	*this,
	0, // LabelManager  *label_mgr,
	0, // MenuManager   *menu_mgr,
	m_top_widget,
	handle,
	name,
	version,
	author
    );

    Q_ASSERT(context);
    if (!context) {
	qWarning("PluginManager::loadPlugin('"+name+"'): out of memory");
	dlclose(handle);
	return 0;
    }

    // call the loader function to create an instance
    KwavePlugin *plugin = (*plugin_loader)(context);
    Q_ASSERT(plugin);
    if (!plugin) {
	qWarning("PluginManager::loadPlugin('"+name+"'): out of memory");
	dlclose(handle);
	return 0;
    }

    if (plugin->isUnique()) {
	// append unique plugins to the global list of unique plugins
	m_unique_plugins.append(plugin);
	plugin->use();
    } else {
	// append the plugin into our list of loaded plugins
	m_loaded_plugins.append(plugin);
	if (plugin->isPersistent()) {
	    // increment the usage if it is persistent, we will
	    // only load it once in this instance
	    plugin->use();
	}
    }

    // connect all necessary signals/slots
    connectPlugin(plugin);

    // get the last settings and call the "load" function
    // now the plugin is present and loaded
    QStringList last_params = loadPluginDefaults(name, plugin->version());
    plugin->load(last_params);

    return plugin;
}

//***************************************************************************
int PluginManager::executePlugin(const QString &name, QStringList *params)
{
    QString command;
    int result = 0;

    // synchronize: wait until any currently running plugins are done
    sync();

    // load the new plugin
    KwavePlugin *plugin = loadPlugin(name);
    if (!plugin) return -ENOMEM;

    if (params) {
	// parameters were specified -> call directly
	// without setup dialog
	result = plugin->start(*params);

	// maybe the start() function has called close() ?
	if (m_loaded_plugins.findRef(plugin) == -1) {
	    qDebug("PluginManager: plugin closed itself in start()"); // ###
	    result = -1;
	    plugin = 0;
	}

	if (plugin && (result >= 0)) {
	    plugin->use(); // plugin->release() will be called after run()
	    plugin->execute(*params);
	}
    } else {
	// load previous parameters from config
	QStringList last_params = loadPluginDefaults(name, plugin->version());

	// call the plugin's setup function
	params = plugin->setup(last_params);
	if (params) {
	    // we have a non-zero parameter list, so
	    // the setup function has not been aborted.
	    // Now we can create a command string and
	    // emit a new command.

	    // store parameters for the next time
	    savePluginDefaults(name, plugin->version(), *params);

	    // We DO NOT call the plugin's "execute"
	    // function directly, as it should be possible
	    // to record all function calls in the
	    // macro recorder
	    command = "plugin:execute(";
	    command += name;
	    for (unsigned int i=0; i < params->count(); i++) {
		command += ", ";
		command += *(params->at(i));
	    }
	    delete params;
	    command += ")";
	    qDebug("PluginManager: command='%s'",command.latin1()); // ###
	}
    }

    // now the plugin is no longer needed here, so delete it
    // if it has not already been detached
    if (plugin && !plugin->isPersistent()) plugin->release();

    // emit a command, let the toplevel window (and macro recorder) get
    // it and call us again later...
    if (command.length()) emit sigCommand(command);

    return result;
}

//***************************************************************************
void PluginManager::sync()
{
    bool one_is_running = true;
    while (one_is_running) {
	QPtrListIterator<KwavePlugin> it(m_loaded_plugins);
	one_is_running = false;

	for (; it.current(); ++it) {
	    KwavePlugin *plugin = it.current();
	    if (plugin->isRunning()) {
		qDebug("waiting for plugin '%s'", plugin->name().latin1());
		one_is_running = true;
		break;
	    }
	}

	if (one_is_running) {
	    qApp->processEvents();
	    sleep(1000);
	}
    }
}

//***************************************************************************
int PluginManager::setupPlugin(const QString &name)
{
    // load the plugin
    KwavePlugin* plugin = loadPlugin(name);
    if (!plugin) return -ENOMEM;

    // now the plugin is present and loaded
    QStringList last_params = loadPluginDefaults(name, plugin->version());

    // call the plugin's setup function
    QStringList *params = plugin->setup(last_params);
    if (params) {
	// we have a non-zero parameter list, so
	// the setup function has not been aborted.
	savePluginDefaults(name, plugin->version(), *params);
	delete params;
    } else return -1;

    // now the plugin is no longer needed here, so delete it
    // if it has not already been detached and is not persistent
    if (!plugin->isPersistent()) plugin->release();

    return 0;
}

//***************************************************************************
FileInfo &PluginManager::fileInfo()
{
    return m_top_widget.signalManager().fileInfo();
}

//***************************************************************************
QStringList PluginManager::loadPluginDefaults(const QString &name,
	const QString &version)
{
    QString def_version;
    QString section("plugin ");
    QStringList list;
    section += name;

    KConfig *cfg = KGlobal::config();
    Q_ASSERT(cfg);
    if (!cfg) return list;

    cfg->sync();
    cfg->setGroup(section);

    def_version = cfg->readEntry("version");
    if (!def_version.length()) {
	return list;
    }
    if (!(def_version == version)) {
	qDebug("PluginManager::loadPluginDefaults: "\
	    "plugin '%s': defaults for version '%s' not loaded, found "\
	    "old ones of version '%s'.", name.latin1(), version.latin1(),
	    def_version.latin1());
	return list;
    }

    list = cfg->readListEntry("defaults", ',');
    return list;
}

//***************************************************************************
void PluginManager::savePluginDefaults(const QString &name,
                                       const QString &version,
                                       QStringList &params)
{
    QString section("plugin ");
    section += name;

    KConfig *cfg = KGlobal::config();
    Q_ASSERT(cfg);
    if (!cfg) return;

    cfg->sync();
    cfg->setGroup(section);
    cfg->writeEntry("version", version);
    cfg->writeEntry("defaults", params);
    cfg->sync();
}

//***************************************************************************
unsigned int PluginManager::signalLength()
{
    return m_top_widget.signalManager().length();
}

//***************************************************************************
double PluginManager::signalRate()
{
    return m_top_widget.signalManager().rate();
}

//***************************************************************************
const QMemArray<unsigned int> PluginManager::selectedTracks()
{
    return m_top_widget.signalManager().selectedTracks();
}

//***************************************************************************
unsigned int PluginManager::selectionStart()
{
    return m_top_widget.signalManager().selection().first();
}

//***************************************************************************
unsigned int PluginManager::selectionEnd()
{
    return m_top_widget.signalManager().selection().last();
}

//***************************************************************************
void PluginManager::selectRange(unsigned int offset, unsigned int length)
{
    m_top_widget.signalManager().selectRange(offset, length);
}

//***************************************************************************
SampleWriter *PluginManager::openSampleWriter(unsigned int track,
	InsertMode mode, unsigned int left, unsigned int right)
{
    SignalManager &manager = m_top_widget.signalManager();
    return manager.openSampleWriter(track, mode, left, right, true);
}

//***************************************************************************
void PluginManager::openMultiTrackReader(MultiTrackReader &readers,
    const QMemArray<unsigned int> &track_list,
    unsigned int first, unsigned int last)
{
    SignalManager &manager = m_top_widget.signalManager();
    manager.openMultiTrackReader(readers, track_list, first, last);
}

//***************************************************************************
void PluginManager::openMultiTrackWriter(MultiTrackWriter &writers,
    const QMemArray<unsigned int> &track_list, InsertMode mode,
    unsigned int left, unsigned int right)
{
    SignalManager &manager = m_top_widget.signalManager();
    manager.openMultiTrackWriter(writers, track_list, mode, left, right);
}

//***************************************************************************
void PluginManager::openMultiTrackWriter(MultiTrackWriter &writers,
                                         InsertMode mode)
{
    SignalManager &manager = m_top_widget.signalManager();
    QMemArray<unsigned int> tracks = manager.selectedTracks();
    unsigned int left = 0;
    unsigned int right = 0;

    if (signalLength()) {
	// default if signal is present: current selection
	left  = selectionStart();
	right = selectionEnd();
	if (left == right) {
	    // if no selection: whole signal
	    left  = 0;
	    right = signalLength();
	}
    }

    openMultiTrackWriter(writers, tracks, mode, left, right);
}

//***************************************************************************
ArtsMultiSink *PluginManager::openMultiTrackPlayback(unsigned int tracks,
                                                     const QString *name)
{
    QString device_name;

    // locate the corresponding playback device factory (plugin)
    device_name = (name) ? QString(*name) : QString("");
    PlayBackDevice *device = 0;
    PlaybackDeviceFactory *factory = 0;
    QPtrListIterator<PlaybackDeviceFactory> it(m_playback_factories);
    for (; it.current(); ++it) {
	PlaybackDeviceFactory *f = it.current();
	Q_ASSERT(f);
	if (f && f->supportsDevice(device_name)) {
	    factory = f;
	    break;
	}
    }
    Q_ASSERT(factory);
    if (!factory) return 0;

    // open the playback device with it's default parameters
    device = factory->openDevice(device_name, 0);
    Q_ASSERT(device);
    if (!device) return 0;

    // create the multi track playback sink
    ArtsMultiSink *sink = new ArtsMultiPlaybackSink(tracks, device);
    Q_ASSERT(sink);
    if (!sink) return 0;

    return sink;
}

//***************************************************************************
PlaybackController &PluginManager::playbackController()
{
    return m_top_widget.signalManager().playbackController();
}

//***************************************************************************
void PluginManager::enqueueCommand(const QString &command)
{
    m_spx_command.enqueue(command);
}

//***************************************************************************
void PluginManager::forwardCommand()
{
    const QString *command = m_spx_command.dequeue();
    if (command) {
	m_top_widget.executeCommand(*command);
	delete command;
    }
}

//***************************************************************************
void PluginManager::pluginClosed(KwavePlugin *p)
{
    Q_ASSERT(p);
    Q_ASSERT(!m_loaded_plugins.isEmpty() || !m_unique_plugins.isEmpty());
    if (!p) return;

    // disconnect the signals to avoid recursion
    disconnectPlugin(p);

    if (m_loaded_plugins.findRef(p) != -1) {
	m_loaded_plugins.setAutoDelete(false);
	m_loaded_plugins.removeRef(p);
    }
    if (m_unique_plugins.findRef(p) != -1) {
	m_unique_plugins.setAutoDelete(false);
	m_unique_plugins.removeRef(p);
    }

    // schedule the deferred delete/unload of the plugin
    void *handle = p->handle();
    Q_ASSERT(handle);
    PluginDeleter *delete_later = new PluginDeleter(p, handle);
    Q_ASSERT(delete_later);
    if (delete_later) delete_later->deleteLater();

//    qDebug("PluginManager::pluginClosed(): done");
}

//***************************************************************************
void PluginManager::pluginStarted(KwavePlugin *p)
{
    Q_ASSERT(p);
    if (!p) return;

    // the plugin is running -> increase the usage count in order to
    // prevent our lists from containing invalid entries
    p->use();

    // add the plugin to the list of running plugins
    m_running_plugins.append(p);
}

//***************************************************************************
void PluginManager::pluginDone(KwavePlugin *p)
{
    Q_ASSERT(p);
    if (!p) return;

    // remove the plugin from the list of running plugins
    m_running_plugins.setAutoDelete(false);
    m_running_plugins.removeRef(p);

    // release the plugin, at least we do no longer need it
    p->release();
}

//****************************************************************************
void PluginManager::connectPlugin(KwavePlugin *plugin)
{
    Q_ASSERT(plugin);
    if (!plugin) return;

    if (!plugin->isUnique()) {
	connect(this, SIGNAL(sigClosed()),
	        plugin, SLOT(close()));
    }

    connect(plugin, SIGNAL(sigClosed(KwavePlugin *)),
	    this, SLOT(pluginClosed(KwavePlugin *)));

    connect(plugin, SIGNAL(sigRunning(KwavePlugin *)),
	    this, SLOT(pluginStarted(KwavePlugin *)));

    connect(plugin, SIGNAL(sigDone(KwavePlugin *)),
	    this, SLOT(pluginDone(KwavePlugin *)));
}

//****************************************************************************
void PluginManager::disconnectPlugin(KwavePlugin *plugin)
{
    Q_ASSERT(plugin);
    if (!plugin) return;

    disconnect(plugin, SIGNAL(sigDone(KwavePlugin *)),
	       this, SLOT(pluginDone(KwavePlugin *)));

    disconnect(plugin, SIGNAL(sigRunning(KwavePlugin *)),
	       this, SLOT(pluginStarted(KwavePlugin *)));

    disconnect(this, SIGNAL(sigClosed()),
	       plugin, SLOT(close()));

    if (!plugin->isUnique()) {
	disconnect(plugin, SIGNAL(sigClosed(KwavePlugin *)),
	           this, SLOT(pluginClosed(KwavePlugin *)));
    }
}

//****************************************************************************
void PluginManager::emitNameChanged()
{
    const QString *name = m_spx_name_changed.dequeue();
    Q_ASSERT(name);
    if (name) {
	emit sigSignalNameChanged(*name);
	delete name;
    }
}

//****************************************************************************
void PluginManager::setSignalName(const QString &name)
{
    m_spx_name_changed.enqueue(name);
}

//***************************************************************************
void PluginManager::findPlugins()
{
    QStringList files = KGlobal::dirs()->findAllResources("data",
	    "kwave/plugins/*", false, true);

    QStringList::Iterator it;
    for (it=files.begin(); it != files.end(); ++it) {
	QString file = *it;

	void *handle = dlopen(file, RTLD_NOW);
	if (handle) {
	    const char **name    = (const char **)dlsym(handle, "name");
	    const char **version = (const char **)dlsym(handle, "version");
	    const char **author  = (const char **)dlsym(handle, "author");

	    // skip it if something is missing or null
	    if (!name || !version || !author) continue;
	    if (!*name || !*version || !*author) continue;

	    m_plugin_files.insert(*name, file);
	    qDebug(i18n("%16s %5s written by %s"), *name, *version, *author);

	    dlclose (handle);
	} else {
	    qWarning(i18n("error in '%s':\n\t %s"), file.latin1(),
		dlerror());
	}
    }

    qDebug(i18n("--- \n found %d plugins\n"), m_plugin_files.count());
}

//****************************************************************************
void PluginManager::registerPlaybackDeviceFactory(
    PlaybackDeviceFactory *factory)
{
    m_playback_factories.append(factory);

}

//****************************************************************************
void PluginManager::unregisterPlaybackDeviceFactory(
    PlaybackDeviceFactory *factory)
{
    m_playback_factories.setAutoDelete(false);
    m_playback_factories.removeRef(factory);
}

//****************************************************************************
//****************************************************************************
/* end of PluginManager.cpp */
