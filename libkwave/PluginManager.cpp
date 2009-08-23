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

#include "config.h"

#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>

#include <QApplication>
#include <QMutableListIterator>

#include <kglobal.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kmainwindow.h>
#include <kstandarddirs.h>
#include <klocale.h>

#include "libkwave/KwaveMultiPlaybackSink.h"
#include "libkwave/KwavePlugin.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/Parser.h"
#include "libkwave/PlayBackDevice.h"
#include "libkwave/PlaybackDeviceFactory.h"
#include "libkwave/PluginContext.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SignalManager.h"
#include "libkwave/Writer.h"
#include "libkwave/undo/UndoTransactionGuard.h"
#include "libkwave/undo/UndoAction.h"
#include "libkwave/undo/UndoModifyAction.h"
#include "libkwave/PluginManager.h"

//***************************************************************************
//***************************************************************************
Kwave::PluginManager::PluginDeleter::PluginDeleter(Kwave::Plugin *plugin,
                                                   void *handle)
  :QObject(), m_plugin(plugin), m_handle(handle)
{
}

//***************************************************************************
Kwave::PluginManager::PluginDeleter::~PluginDeleter()
{
    // check: this must be called from the GUI thread only!
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(this->thread() == qApp->thread());

    // empty the event queues before deleting
    qApp->processEvents();
    qApp->flush();

    // delete the plugin, this should also remove everything it has allocated
    if (m_plugin) delete m_plugin;

    // empty the event queues before unmap, in case the destructors
    // queued something
    qApp->processEvents();
    qApp->flush();

    // now the handle of the shared object can be cleared too
    dlclose(m_handle);
}

//***************************************************************************
//***************************************************************************

// static initializers

QMap<QString, QString> Kwave::PluginManager::m_plugin_files;

Kwave::PluginManager::PluginList Kwave::PluginManager::m_unique_plugins;

static QList<PlaybackDeviceFactory *> m_playback_factories;

//***************************************************************************
Kwave::PluginManager::PluginManager(QWidget *parent,
                                    SignalManager &signal_manager)
    :m_loaded_plugins(), m_running_plugins(),
     m_parent_widget(parent), m_signal_manager(signal_manager)
{
    // use all unique plugins
    // this does nothing on the first instance, all other instances
    // will probably find a non-empty list
    foreach (KwavePluginPointer p, m_unique_plugins) {
	Q_ASSERT(p && p->isUnique());
	if (p && p->isUnique()) {
	    p->use();

	    // maybe we will become responsible for releasing
	    // the plugin (when it is in use but the plugin manager
	    // who has created it is already finished)
	    connect(p,    SIGNAL(sigClosed(Kwave::Plugin *)),
	            this, SLOT(pluginClosed(Kwave::Plugin *)),
	            Qt::QueuedConnection);
	}
    }
}

//***************************************************************************
Kwave::PluginManager::~PluginManager()
{
    // inform all plugins and client windows that we close now
    emit sigClosed();

    // wait until all plugins are really closed
    this->sync();

    // release all unique plugins
    while (!m_unique_plugins.isEmpty()) {
	KwavePluginPointer p = m_unique_plugins.takeLast();
	Q_ASSERT(p && p->isUnique());
	if (p && p->isUnique()) p->release();
    }

    // release all own persistent plugins
    while (!m_loaded_plugins.isEmpty()) {
	KwavePluginPointer p = m_loaded_plugins.takeLast();
	Q_ASSERT(p);
	if (p && p->isPersistent()) p->release();
    }

    // release all own plugins that are left
    while (!m_loaded_plugins.isEmpty()) {
	KwavePluginPointer p = m_loaded_plugins.takeLast();
	Q_ASSERT(p);
	if (p) p->release();
    }
}

//***************************************************************************
void Kwave::PluginManager::loadAllPlugins()
{
    // Try to load all plugins. This has to be called only once per
    // instance of the main window!
    // NOTE: this also implicitely makes it resident if it is persistent or unique
    foreach (QString name, m_plugin_files.keys()) {
	KwavePluginPointer plugin = loadPlugin(name);
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
	            "from list", name.toLocal8Bit().data());
	    m_plugin_files.remove(name);
	}
    }
}

//***************************************************************************
Kwave::Plugin *Kwave::PluginManager::loadPlugin(const QString &name)
{
    // check: this must be called from the GUI thread only!
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(this->thread() == qApp->thread());

    // first find out if the plugin is already loaded and persistent
    foreach (KwavePluginPointer p, m_loaded_plugins) {
	if (p && p->isPersistent() && (p->name() == name)) {
	    Q_ASSERT(p->isPersistent());
	    qDebug("PluginManager::loadPlugin('%s')"\
	           "-> returning pointer to persistent",
	           name.toLocal8Bit().data());
	    return p;
	}
    }

    // find out if the plugin is already loaded and unique
    foreach (KwavePluginPointer p, m_unique_plugins) {
	if (p && (p->name() == name)) {
	    Q_ASSERT(p->isUnique());
	    Q_ASSERT(p->isPersistent());
	    qDebug("PluginManager::loadPlugin('%s')"\
	           "-> returning pointer to unique+persistent",
	           name.toLocal8Bit().data());
	    return p;
	}
    }

    // show a warning and abort if the plugin is unknown
    if (!(m_plugin_files.contains(name))) {
	QString message =
	    i18n("oops, plugin '%1' is unknown or invalid!", name);
	Kwave::MessageBox::error(m_parent_widget, message,
	    i18n("error on loading plugin"));
	return 0;
    }
    QString &filename = m_plugin_files[name];

    // try to get the file handle of the plugin's binary
    void *handle = dlopen(filename.toLocal8Bit(), RTLD_NOW);
    if (!handle) {
	QString message = i18n("unable to load the file \n'%1'\n"\
	                       " that contains the plugin '%2' !",
	                       filename, name);
	Kwave::MessageBox::error(m_parent_widget, message,
	    i18n("error on loading plugin"));
	return 0;
    }

    // get the loader function
    Kwave::Plugin *(*plugin_loader)(const PluginContext *c) = 0;

    // hardcoded, should always work when the
    // symbols are declared as extern "C"
    const char *sym_version = "version";
    const char *sym_author  = "author";
    const char *sym_loader  = "load";

    // get the plugin's author
    const char *author = "";
    const char **pauthor =
	static_cast<const char **>(dlsym(handle, sym_author));
    Q_ASSERT(pauthor);
    if (pauthor) author=*pauthor;
    if (!author) author = i18n("(unknown)").toLocal8Bit();

    // get the plugin's version string
    const char *version = "";
    const char **pver =
	static_cast<const char **>(dlsym(handle, sym_version));
    Q_ASSERT(pver);
    if (pver) version=*pver;
    if (!version) version = i18n("(unknown)").toLocal8Bit();

    plugin_loader =
        reinterpret_cast<Kwave::Plugin *(*)(const PluginContext *)>(
	reinterpret_cast<qint64>(dlsym(handle, sym_loader)));
    Q_ASSERT(plugin_loader);
    if (!plugin_loader) {
	// plugin is null, out of memory or not found
	qWarning("PluginManager::loadPlugin('%s'): "\
		"plugin does not contain a loader, "\
		"maybe it is damaged or the wrong version?",
		name.toLocal8Bit().data());
	dlclose(handle);
	return 0;
    }

    PluginContext context(
	*this,
	handle,
	name,
	version,
	author
    );

    // call the loader function to create an instance
    Kwave::Plugin *plugin = (*plugin_loader)(&context);
    Q_ASSERT(plugin);
    if (!plugin) {
	qWarning("PluginManager::loadPlugin('%s'): out of memory",
	         name.toLocal8Bit().data());
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
int Kwave::PluginManager::executePlugin(const QString &name,
                                        QStringList *params)
{
    QString command;
    int result = 0;

    // check: this must be called from the GUI thread only!
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(this->thread() == qApp->thread());

    // synchronize: wait until any currently running plugins are done
    sync();

    // load the new plugin
    KwavePluginPointer plugin = loadPlugin(name);
    if (!plugin) return -ENOMEM;

    if (params) {
	// parameters were specified -> call directly
	// without setup dialog
	result = plugin->start(*params);

	// maybe the start() function has called close() ?
	if (!plugin->isPersistent() && !m_loaded_plugins.contains(plugin)) {
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
	    foreach (const QString &p, *params)
		command += ", " + p;
	    delete params;
	    command += ")";
//	    qDebug("PluginManager: command='%s'",command.data());
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
bool Kwave::PluginManager::onePluginRunning()
{
    // check: this must be called from the GUI thread only!
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(this->thread() == qApp->thread());

    if (m_loaded_plugins.isEmpty()) return false;
    foreach (KwavePluginPointer plugin, m_loaded_plugins)
	if (plugin && plugin->isRunning()) return true;
    return false;
}

//***************************************************************************
void Kwave::PluginManager::sync()
{
    // check: this must be called from the GUI thread only!
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(this->thread() == qApp->thread());

    while (onePluginRunning()) {
	pthread_yield();
	qApp->processEvents();
	qApp->flush();
    }
}

//***************************************************************************
int Kwave::PluginManager::setupPlugin(const QString &name)
{
    // load the plugin
    Kwave::Plugin* plugin = loadPlugin(name);
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
FileInfo &Kwave::PluginManager::fileInfo()
{
    return m_signal_manager.fileInfo();
}

//***************************************************************************
QStringList Kwave::PluginManager::loadPluginDefaults(const QString &name,
                                                     const QString &version)
{
    QString def_version;
    QString section("plugin ");
    QStringList list;
    section += name;

    Q_ASSERT(KGlobal::config());
    if (!KGlobal::config()) return list;
    KConfigGroup cfg = KGlobal::config()->group(section);

    cfg.sync();

    def_version = cfg.readEntry("version");
    if (!def_version.length()) {
	return list;
    }
    if (!(def_version == version)) {
	qDebug("PluginManager::loadPluginDefaults: "\
	    "plugin '%s': defaults for version '%s' not loaded, found "\
	    "old ones of version '%s'.", name.toLocal8Bit().data(),
	    version.toLocal8Bit().data(), def_version.toLocal8Bit().data());
	return list;
    }

    list = cfg.readEntry("defaults").split(',');
    return list;
}

//***************************************************************************
void Kwave::PluginManager::savePluginDefaults(const QString &name,
                                              const QString &version,
                                              QStringList &params)
{
    QString section("plugin ");
    section += name;

    Q_ASSERT(KGlobal::config());
    if (!KGlobal::config()) return;
    KConfigGroup cfg = KGlobal::config()->group(section);

    cfg.sync();
    cfg.writeEntry("version", version);
    cfg.writeEntry("defaults", params);
    cfg.sync();
}

//***************************************************************************
unsigned int Kwave::PluginManager::signalLength()
{
    return m_signal_manager.length();
}

//***************************************************************************
double Kwave::PluginManager::signalRate()
{
    return m_signal_manager.rate();
}

//***************************************************************************
const QList<unsigned int> Kwave::PluginManager::selectedTracks()
{
    return m_signal_manager.selectedTracks();
}

//***************************************************************************
unsigned int Kwave::PluginManager::selectionStart()
{
    return m_signal_manager.selection().first();
}

//***************************************************************************
unsigned int Kwave::PluginManager::selectionEnd()
{
    return m_signal_manager.selection().last();
}

//***************************************************************************
void Kwave::PluginManager::selectRange(unsigned int offset,
                                       unsigned int length)
{
    m_signal_manager.selectRange(offset, length);
}

//***************************************************************************
Kwave::Writer *Kwave::PluginManager::openWriter(unsigned int track,
	InsertMode mode, unsigned int left, unsigned int right)
{
    return m_signal_manager.openWriter(track, mode, left, right, true);
}

//***************************************************************************
Kwave::SampleSink *Kwave::PluginManager::openMultiTrackPlayback(
    unsigned int tracks, const QString *name)
{
    QString device_name;

    // locate the corresponding playback device factory (plugin)
    device_name = (name) ? QString(*name) : QString("");
    PlayBackDevice *device = 0;
    PlaybackDeviceFactory *factory = 0;
    foreach (PlaybackDeviceFactory *f, m_playback_factories) {
	Q_ASSERT(f);
	if (f && f->supportsDevice(device_name)) {
	    factory = f;
	    break;
	}
    }
    if (!factory) return 0;

    // open the playback device with it's default parameters
    device = factory->openDevice(device_name, tracks);
    if (!device) return 0;

    // create the multi track playback sink
    Kwave::SampleSink *sink = new Kwave::MultiPlaybackSink(tracks, device);
    Q_ASSERT(sink);
    return sink;
}

//***************************************************************************
PlaybackController &Kwave::PluginManager::playbackController()
{
    return m_signal_manager.playbackController();
}

//***************************************************************************
void Kwave::PluginManager::enqueueCommand(const QString &command)
{
    emit sigCommand(command);
}

//***************************************************************************
void Kwave::PluginManager::signalClosed()
{
    emit sigClosed();
}

//***************************************************************************
void Kwave::PluginManager::pluginClosed(Kwave::Plugin *p)
{
    // check: this must be called from the GUI thread only!
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(this->thread() == qApp->thread());

    Q_ASSERT(p);
    Q_ASSERT(!m_loaded_plugins.isEmpty() || !m_unique_plugins.isEmpty());
    if (!p) return;

    // disconnect the signals to avoid recursion
    disconnectPlugin(p);

    if (m_loaded_plugins.contains(p)) m_loaded_plugins.removeAll(p);
    if (m_unique_plugins.contains(p)) m_unique_plugins.removeAll(p);

    // schedule the deferred delete/unload of the plugin
    void *handle = p->handle();
    Q_ASSERT(handle);
    PluginDeleter *delete_later = new PluginDeleter(p, handle);
    Q_ASSERT(delete_later);
    if (delete_later) delete_later->deleteLater();

//    qDebug("PluginManager::pluginClosed(): done");
}

//***************************************************************************
void Kwave::PluginManager::pluginStarted(Kwave::Plugin *p)
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
void Kwave::PluginManager::pluginDone(Kwave::Plugin *p)
{
    // check: this must be called from the GUI thread only!
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(this->thread() == qApp->thread());

    Q_ASSERT(p);
    if (!p) return;

    // remove the plugin from the list of running plugins
    m_running_plugins.removeAll(p);

    // release the plugin, at least we do no longer need it
    p->release();
}

//***************************************************************************
void Kwave::PluginManager::connectPlugin(Kwave::Plugin *plugin)
{
    Q_ASSERT(plugin);
    if (!plugin) return;

    if (!plugin->isUnique()) {
	connect(this, SIGNAL(sigClosed()),
	        plugin, SLOT(close()));
    }

    connect(plugin, SIGNAL(sigClosed(Kwave::Plugin *)),
	    this, SLOT(pluginClosed(Kwave::Plugin *)),
	     Qt::QueuedConnection);

    connect(plugin, SIGNAL(sigRunning(Kwave::Plugin *)),
	    this, SLOT(pluginStarted(Kwave::Plugin *)),
	    Qt::DirectConnection);

    connect(plugin, SIGNAL(sigDone(Kwave::Plugin *)),
	    this, SLOT(pluginDone(Kwave::Plugin *)),
	    Qt::QueuedConnection);
}

//***************************************************************************
void Kwave::PluginManager::disconnectPlugin(Kwave::Plugin *plugin)
{
    Q_ASSERT(plugin);
    if (!plugin) return;

    disconnect(plugin, SIGNAL(sigDone(Kwave::Plugin *)),
	       this, SLOT(pluginDone(Kwave::Plugin *)));

    disconnect(plugin, SIGNAL(sigRunning(Kwave::Plugin *)),
	       this, SLOT(pluginStarted(Kwave::Plugin *)));

    disconnect(this, SIGNAL(sigClosed()),
	       plugin, SLOT(close()));

    if (!plugin->isUnique()) {
	disconnect(plugin, SIGNAL(sigClosed(Kwave::Plugin *)),
	           this, SLOT(pluginClosed(Kwave::Plugin *)));
    }
}

//***************************************************************************
void Kwave::PluginManager::setSignalName(const QString &name)
{
    emit sigSignalNameChanged(name);
}

//***************************************************************************
void Kwave::PluginManager::findPlugins()
{
    if (!m_plugin_files.isEmpty()) {
	// this is not the first call -> bail out
	return;
    }

    KStandardDirs dirs;
    QStringList files = dirs.findAllResources("module",
	    "plugins/kwave/*", KStandardDirs::NoDuplicates);

    /* fallback: search also in the old location (useful for developers) */
    files += dirs.findAllResources("data",
	    "kwave/plugins/*", KStandardDirs::NoDuplicates);

    foreach (QString file, files) {
	void *handle = dlopen(file.toLocal8Bit(), RTLD_NOW);
	if (handle) {
	    const char **name    =
		static_cast<const char **>(dlsym(handle, "name"));
	    const char **version =
		static_cast<const char **>(dlsym(handle, "version"));
	    const char **author  =
		static_cast<const char **>(dlsym(handle, "author"));

	    // skip it if something is missing or null
	    if (!name || !version || !author) continue;
	    if (!*name || !*version || !*author) continue;

	    emit sigProgress(i18n("loading plugin %1...", *name));
	    QApplication::processEvents();

	    m_plugin_files.insert(*name, file);
	    qDebug("%16s %5s written by %s",
		*name, *version, *author);

	    dlclose (handle);
	} else {
	    qWarning("error in '%s':\n\t %s",
		file.toLocal8Bit().data(), dlerror());
	}
    }

    qDebug("--- \n found %d plugins\n", m_plugin_files.count());
}

//***************************************************************************
void Kwave::PluginManager::registerPlaybackDeviceFactory(
    PlaybackDeviceFactory *factory)
{
    m_playback_factories.append(factory);
}

//***************************************************************************
void Kwave::PluginManager::unregisterPlaybackDeviceFactory(
    PlaybackDeviceFactory *factory)
{
    m_playback_factories.removeAll(factory);
}

//***************************************************************************
#include "PluginManager.moc"
//***************************************************************************
//***************************************************************************
