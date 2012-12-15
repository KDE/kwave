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
#include <unistd.h>

#include <QtGui/QApplication>
#include <QtCore/QLatin1Char>
#include <QtCore/QMutableListIterator>

#include <kglobal.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kmainwindow.h>
#include <kstandarddirs.h>
#include <klocale.h>

#include "libkwave/Plugin.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MultiPlaybackSink.h"
#include "libkwave/PlayBackDevice.h"
#include "libkwave/PlaybackDeviceFactory.h"
#include "libkwave/PluginContext.h"
#include "libkwave/SignalManager.h"
#include "libkwave/Utils.h"
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
    Q_ASSERT(m_plugin);
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
    Q_ASSERT(m_plugin);
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

QMap<QString, Kwave::PluginManager::PluginInfo> \
    Kwave::PluginManager::m_found_plugins;

Kwave::PluginManager::PluginList Kwave::PluginManager::m_unique_plugins;

static QList<Kwave::PlaybackDeviceFactory *> m_playback_factories;

//***************************************************************************
Kwave::PluginManager::PluginManager(QWidget *parent,
                                    Kwave::SignalManager &signal_manager)
    :m_loaded_plugins(),
     m_running_plugins(),
     m_parent_widget(parent),
     m_signal_manager(signal_manager),
     m_view_manager(0)
{
    // connect all unique plugins
    foreach (KwavePluginPointer p, m_unique_plugins) {
	Q_ASSERT(p && p->isUnique());
	if (p) {
	    // increase the use count of the existing unique plugin
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
    foreach (KwavePluginPointer p, m_unique_plugins) {
	Q_ASSERT(p && p->isUnique());
	if (p) p->release();
	this->sync();
    }

    // release all own persistent plugins
    foreach (KwavePluginPointer p, m_loaded_plugins) {
	Q_ASSERT(p);
	if (p && p->isPersistent()) p->release();
	this->sync();
    }

    // release all own plugins that are left (should be an empty list!)
    while (!m_loaded_plugins.isEmpty()) {
	KwavePluginPointer p = m_loaded_plugins.takeLast();
	Q_ASSERT(p);
	qWarning("RELEASING LOADED PLUGIN '%s'", DBG(p->name()));
	if (p) p->release();
	this->sync();
    }

}

//***************************************************************************
void Kwave::PluginManager::loadAllPlugins()
{
    // Try to load all plugins. This has to be called only once per
    // instance of the main window!
    // NOTE: this also implicitely makes it resident if it is persistent or unique
    foreach (QString name, m_found_plugins.keys()) {
	KwavePluginPointer plugin = loadPlugin(name);
	if (plugin) {
// 	    QString state = "";
// 	    if (plugin->isPersistent()) state += "(persistent)";
// 	    if (plugin->isUnique()) state += "(unique)";
// 	    if (!state.length()) state = "(normal)";
// 	    qDebug("PluginManager::loadAllPlugins(): plugin '%s' %s",
// 		   DBG(plugin->name()),
// 		   DBG(state));

	    // reduce use count again, unique and persistent plugins
	    // stay loaded with use count 1
	    plugin->release();
	} else {
	    // loading failed => remove it from the list
	    qWarning("PluginManager::loadAllPlugins(): removing '%s' "
	            "from list", DBG(name));
	    m_found_plugins.remove(name);
	}
    }
}

//***************************************************************************
void Kwave::PluginManager::stopAllPlugins()
{
    // check: this must be called from the GUI thread only!
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(this->thread() == qApp->thread());

    if (!m_loaded_plugins.isEmpty())
	foreach (KwavePluginPointer plugin, m_loaded_plugins)
	    if (plugin && plugin->isRunning())
		plugin->stop() ;
    if (!m_unique_plugins.isEmpty())
	foreach (KwavePluginPointer plugin, m_unique_plugins)
	    if (plugin && plugin->isRunning())
		plugin->stop();

    sync();
}

//***************************************************************************
Kwave::Plugin *Kwave::PluginManager::loadPlugin(const QString &name)
{
    // check: this must be called from the GUI thread only!
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(this->thread() == qApp->thread());

    // first find out if the plugin is already loaded and persistent
    foreach (KwavePluginPointer p, m_loaded_plugins) {
	Q_ASSERT(p);
	if (p && p->isPersistent() && (p->name() == name)) {
	    qDebug("PluginManager::loadPlugin('%s')"
	           "-> returning pointer to persistent",
	           DBG(name));
	    p->use();
	    return p;
	}
    }

    // find out if the plugin is already loaded and unique
    foreach (KwavePluginPointer p, m_unique_plugins) {
	if (p && (p->name() == name)) {
	    Q_ASSERT(p->isUnique());
	    Q_ASSERT(p->isPersistent());
	    qDebug("PluginManager::loadPlugin('%s')"
	           "-> returning pointer to unique+persistent",
	           DBG(name));
	    p->use();
	    return p;
	}
    }

    // show a warning and abort if the plugin is unknown
    if (!(m_found_plugins.contains(name))) {
	QString message =
	    i18n("The plugin '%1' is unknown or invalid.", name);
	Kwave::MessageBox::error(m_parent_widget, message,
	    i18n("Error On Loading Plugin"));
	return 0;
    }
    QString &filename = m_found_plugins[name].m_filename;

    // try to get the file handle of the plugin's binary
    void *handle = dlopen(filename.toLocal8Bit(), RTLD_NOW);
    if (!handle) {
	QString message = i18n("Unable to load the file\n'%1'\n"\
	                       "that contains the plugin '%2'.",
	                       filename, name);
	Kwave::MessageBox::error(m_parent_widget, message,
	    i18n("Error On Loading Plugin"));
	return 0;
    }

    // get the loader function
    Kwave::Plugin *(*plugin_loader)(const Kwave::PluginContext *c) = 0;

    // hardcoded, should always work when the
    // symbols are declared as extern "C"
    const char *sym_version = "version";
    const char *sym_author  = "author";
    const char *sym_loader  = "load";

    // get the plugin's author
    QString author = i18n("(Unknown)");
    const char **pauthor =
	static_cast<const char **>(dlsym(handle, sym_author));
    Q_ASSERT(pauthor);
    if (pauthor) author = _(*pauthor);

    // get the plugin's version string
    QString version = i18n("(Unknown)");
    const char **pver =
	static_cast<const char **>(dlsym(handle, sym_version));
    Q_ASSERT(pver);
    if (pver) version = _(*pver);

    plugin_loader =
        reinterpret_cast<Kwave::Plugin *(*)(const Kwave::PluginContext *)>(
	reinterpret_cast<qint64>(dlsym(handle, sym_loader)));
    Q_ASSERT(plugin_loader);
    if (!plugin_loader) {
	// plugin is null, out of memory or not found
	qWarning("PluginManager::loadPlugin('%s'): "
		"plugin does not contain a loader, "
		"maybe it is damaged or the wrong version?",
		DBG(name));
	dlclose(handle);
	return 0;
    }

    Kwave::PluginContext context(
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
	qWarning("PluginManager::loadPlugin('%s'): out of memory", DBG(name));
	dlclose(handle);
	return 0;
    }

    if (plugin->isUnique()) {
	// append unique plugins to the global list of unique plugins
	plugin->use();
	m_unique_plugins.append(plugin);
    } else {
	// append the plugin into our list of loaded plugins
	if (plugin->isPersistent()) {
	    plugin->use();
	}
	m_loaded_plugins.append(plugin);
    }

    // now we have a newly created plugin, the use count is
    // 1 for normal plugins
    // 2 for persistent plugins
    // 2 for unique plugins

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
	if (!m_unique_plugins.contains(plugin) &&
	    !m_loaded_plugins.contains(plugin)) {
	    qDebug("PluginManager: plugin closed itself in start()");
	    result = -1;
	    plugin = 0;
	}

	if (plugin && (result >= 0)) {
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
	    command = _("plugin:execute(");
	    command += name;
	    foreach (const QString &p, *params)
		command += _(", ") + p;
	    delete params;
	    command += _(")");
//	    qDebug("PluginManager: command='%s'",command.data());
	}
    }

    // now the plugin is no longer needed here, so delete it
    // if it has not already been detached
    if (plugin) plugin->release();

    // emit a command, let the toplevel window (and macro recorder) get
    // it and call us again later...
    if (command.length()) emit sigCommand(command);

    return result;
}

//***************************************************************************
bool Kwave::PluginManager::canClose()
{
    // check: this must be called from the GUI thread only!
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(this->thread() == qApp->thread());

    if (!m_loaded_plugins.isEmpty())
	foreach (KwavePluginPointer plugin, m_loaded_plugins)
	    if (plugin && !plugin->canClose()) return false;
    if (!m_unique_plugins.isEmpty())
	foreach (KwavePluginPointer plugin, m_unique_plugins)
	    if (plugin && !plugin->canClose()) return false;
    return true;
}


//***************************************************************************
bool Kwave::PluginManager::onePluginRunning()
{
    // check: this must be called from the GUI thread only!
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(this->thread() == qApp->thread());

    if (!m_loaded_plugins.isEmpty())
	foreach (KwavePluginPointer plugin, m_loaded_plugins)
	    if (plugin && plugin->isRunning()) return true;
    if (!m_unique_plugins.isEmpty())
	foreach (KwavePluginPointer plugin, m_unique_plugins)
	    if (plugin && plugin->isRunning()) return true;
    return false;
}

//***************************************************************************
void Kwave::PluginManager::sync()
{
    // check: this must be called from the GUI thread only!
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(this->thread() == qApp->thread());

    qApp->processEvents();
    qApp->flush();

    while (onePluginRunning()) {
	Kwave::yield();
	qApp->processEvents();
	qApp->flush();
	usleep(100000);
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
    } else {
	plugin->release();
	return -1;
    }

    plugin->release();
    return 0;
}

//***************************************************************************
QStringList Kwave::PluginManager::loadPluginDefaults(
    const QString &name, const QString &version)
{
    QString def_version;
    QString section = _("plugin ");
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
	qDebug("PluginManager::loadPluginDefaults: "
	    "plugin '%s': defaults for version '%s' not loaded, found "
	    "old ones of version '%s'.",
	    DBG(name), DBG(version), DBG(def_version));
	return list;
    }

    list = cfg.readEntry("defaults").split(QLatin1Char(','));
    return list;
}

//***************************************************************************
void Kwave::PluginManager::savePluginDefaults(const QString &name,
                                              const QString &version,
                                              QStringList &params)
{
    QString section = _("plugin ");
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
sample_index_t Kwave::PluginManager::signalLength()
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
sample_index_t Kwave::PluginManager::selectionStart()
{
    return m_signal_manager.selection().first();
}

//***************************************************************************
sample_index_t Kwave::PluginManager::selectionEnd()
{
    return m_signal_manager.selection().last();
}

//***************************************************************************
void Kwave::PluginManager::selectRange(sample_index_t offset,
                                       sample_index_t length)
{
    m_signal_manager.selectRange(offset, length);
}

//***************************************************************************
Kwave::SampleSink *Kwave::PluginManager::openMultiTrackPlayback(
    unsigned int tracks, const QString *name)
{
    QString device_name;

    // locate the corresponding playback device factory (plugin)
    device_name = (name) ? QString(*name) : _("");
    Kwave::PlayBackDevice *device = 0;
    Kwave::PlaybackDeviceFactory *factory = 0;
    foreach (Kwave::PlaybackDeviceFactory *f, m_playback_factories) {
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
Kwave::PlaybackController &Kwave::PluginManager::playbackController()
{
    return m_signal_manager.playbackController();
}

//***************************************************************************
void Kwave::PluginManager::insertView(Kwave::SignalView *view, QWidget *controls)
{
    if (m_view_manager)
	m_view_manager->insertView(view, controls);
}

//***************************************************************************
void Kwave::PluginManager::registerViewManager(Kwave::ViewManager *view_manager)
{
    Q_ASSERT(!view_manager || !m_view_manager);
    m_view_manager = view_manager;
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
    if (!m_found_plugins.isEmpty()) {
	// this is not the first call -> bail out
	return;
    }

    KStandardDirs dirs;
    QStringList files = dirs.findAllResources("module",
	    _("plugins/kwave/*"), KStandardDirs::NoDuplicates);

    /* fallback: search also in the old location (useful for developers) */
    files += dirs.findAllResources("data",
	    _("kwave/plugins/*"), KStandardDirs::NoDuplicates);

    foreach (QString file, files) {
	void *handle = dlopen(file.toLocal8Bit(), RTLD_NOW);
	if (handle) {
	    const char **name    =
		static_cast<const char **>(dlsym(handle, "name"));
	    const char **version =
		static_cast<const char **>(dlsym(handle, "version"));
	    const char **description =
		static_cast<const char **>(dlsym(handle, "description"));
	    const char **author  =
		static_cast<const char **>(dlsym(handle, "author"));

	    // skip it if something is missing or null
	    if (!name || !version || !author || !description) continue;
	    if (!*name || !*version || !*author || !*description) continue;

	    emit sigProgress(i18n("Loading plugin %1...", _(*name)));
	    QApplication::processEvents();

	    PluginInfo info;
	    info.m_filename    = file;
	    info.m_author      = QString::fromUtf8(*author);
	    info.m_description = i18n(*description);
	    info.m_version     = QString::fromUtf8(*version);

	    m_found_plugins.insert(_(*name), info);

	    qDebug("%16s %5s written by %s", *name, *version, *author);

	    dlclose (handle);
	} else {
	    qWarning("error in '%s':\n\t %s", DBG(file), dlerror());
	}
    }

    qDebug("--- \n found %d plugins\n", m_found_plugins.count());
}

//***************************************************************************
const QList<Kwave::PluginManager::PluginInfo>
    Kwave::PluginManager::pluginInfoList() const
{
    return m_found_plugins.values();
}

//***************************************************************************
void Kwave::PluginManager::registerPlaybackDeviceFactory(
    Kwave::PlaybackDeviceFactory *factory)
{
    m_playback_factories.append(factory);
}

//***************************************************************************
void Kwave::PluginManager::unregisterPlaybackDeviceFactory(
    Kwave::PlaybackDeviceFactory *factory)
{
    m_playback_factories.removeAll(factory);
}

//***************************************************************************
#include "PluginManager.moc"
//***************************************************************************
//***************************************************************************
