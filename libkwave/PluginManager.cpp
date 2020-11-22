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

#include <errno.h>
#include <unistd.h>
#include <new>

#include <QApplication>
#include <QDir>
#include <QLatin1Char>
#include <QLibrary>
#include <QLibraryInfo>
#include <QMutableListIterator>
#include <QtGlobal>
#include <QVariantList>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMainWindow>
#include <KPluginInfo>
#include <KPluginFactory>
#include <KPluginMetaData>
#include <KSharedConfig>

#include "libkwave/MessageBox.h"
#include "libkwave/MultiPlaybackSink.h"
#include "libkwave/PlayBackDevice.h"
#include "libkwave/PlaybackDeviceFactory.h"
#include "libkwave/Plugin.h"
#include "libkwave/PluginManager.h"
#include "libkwave/SignalManager.h"
#include "libkwave/Utils.h"
#include "libkwave/Writer.h"
#include "libkwave/undo/UndoAction.h"
#include "libkwave/undo/UndoModifyAction.h"
#include "libkwave/undo/UndoTransactionGuard.h"

//***************************************************************************
// static initializers

QMap<QString, Kwave::PluginManager::PluginModule>
    Kwave::PluginManager::m_plugin_modules;

Kwave::PluginManager *Kwave::PluginManager::m_active_instance = Q_NULLPTR;

//***************************************************************************
Kwave::PluginManager::PluginManager(QWidget *parent,
                                    Kwave::SignalManager &signal_manager)
    :m_plugin_instances(),
     m_running_plugins(),
     m_parent_widget(parent),
     m_signal_manager(signal_manager),
     m_view_manager(Q_NULLPTR)
{
}

//***************************************************************************
Kwave::PluginManager::~PluginManager()
{
    // inform all plugins and client windows that we close now
    emit sigClosed();

    // wait until all plugins are really closed
    this->sync();

    // give all plugins that still are loaded the chance to do some cleanups
    // or unregistration tasks. Ideally this should also trigger a "release"
    // of these remaining plugins, so that afterwards we have no more
    // plugin instances left.
    while (!m_plugin_instances.isEmpty()) {
	KwavePluginPointer p = m_plugin_instances.takeLast();
	Q_ASSERT(p);
	if (p) p->unload();
    }

    // this should make the cleanup handlers run (deferred delete)
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    // release all loaded modules
    for (QMap<QString, PluginModule>::iterator it(m_plugin_modules.begin());
         it != m_plugin_modules.end(); )
    {
// 	const QString &name = it.key();
	PluginModule  &p    = it.value();
	p.m_use_count--;

// 	qDebug("PluginManager: releasing module '%s' [refcnt=%d]",
// 	       DBG(name), p.m_use_count);
	if (p.m_use_count == 0) {
	    // take out the pointer to the loadable module
	    KPluginFactory *factory = p.m_factory;
            p.m_factory = Q_NULLPTR;

	    // remove the module from the list
	    it = m_plugin_modules.erase(it);

	    // now the handle of the shared object can be released too
	    if (factory) delete factory;
	} else {
	    // still in use
	    ++it;
	}
    }

    // we are no longer the active instance
    if (m_active_instance == this)
        m_active_instance = Q_NULLPTR;
}

//***************************************************************************
bool Kwave::PluginManager::loadAllPlugins()
{
    // Try to load all plugins. This has to be called only once per
    // instance of the main window!
    // NOTE: this also gives each plugin the chance to stay in memory
    //       if necessary (e.g. for codecs)
    for (QMap<QString, PluginModule>::iterator it(m_plugin_modules.begin());
	 it != m_plugin_modules.end(); ++it)
    {
	const QString      &name   = it.key();
	KwavePluginPointer  plugin = createPluginInstance(name);
	if (plugin) {
// 	    qDebug("PluginManager::loadAllPlugins(): plugin '%s'",
// 		   DBG(plugin->name()));

	    // get the last settings and call the "load" function
	    // now the plugin is present and loaded
	    QStringList last_params = defaultParams(name);
	    plugin->load(last_params);

	    // reduce use count again, we loaded the plugin only to give
	    // it a chance to register some service if necessary (e.g. a
	    // codec)
	    // Most plugins fall back to use count zero and will be
	    // deleted again.
	    plugin->release();
	} else {
	    // loading failed => remove it from the list
	    qWarning("PluginManager::loadAllPlugins(): removing '%s' "
	            "from list", DBG(name));
	    m_plugin_modules.remove(name);
	}
    }

    return !m_plugin_modules.isEmpty();
}

//***************************************************************************
void Kwave::PluginManager::stopAllPlugins()
{
    // check: this must be called from the GUI thread only!
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(this->thread() == qApp->thread());

    if (!m_plugin_instances.isEmpty())
	foreach (const KwavePluginPointer &plugin, m_plugin_instances)
	    if (plugin && plugin->isRunning())
		plugin->stop() ;

    sync();
}

//***************************************************************************
Kwave::Plugin *Kwave::PluginManager::createPluginInstance(const QString &name)
{
    // check: this must be called from the GUI thread only!
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(this->thread() == qApp->thread());

    // show an error message and abort if the plugin is unknown
    if (!(m_plugin_modules.contains(name))) {
	Kwave::MessageBox::error(m_parent_widget,
	    i18n("The plugin '%1' is unknown or invalid.", name),
	    i18n("Error On Loading Plugin"));
        return Q_NULLPTR;
    }

    PluginModule &info = m_plugin_modules[name];
//     qDebug("loadPlugin(%s) [module use count=%d]",
//         DBG(name), info.m_use_count);

    KPluginFactory *factory = info.m_factory;
    Q_ASSERT(factory);

    // call the loader function to create an instance
    QVariantList args;
    args << info.m_name;
    args << info.m_description;
    Kwave::Plugin *plugin = factory->create<Kwave::Plugin>(this, args);
    Q_ASSERT(plugin);
    if (!plugin) {
	qWarning("PluginManager::loadPlugin('%s'): out of memory", DBG(name));
        return Q_NULLPTR;
    }
    // now we have a newly created plugin, the use count is 1

    // append to our list of loaded plugins
    m_plugin_instances.append(plugin);

    // connect all necessary signals/slots
    connectPlugin(plugin);

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
    this->sync();

    // load the new plugin
    KwavePluginPointer plugin = createPluginInstance(name);
    if (!plugin) return -ENOMEM;

    if (params) {
	// parameters were specified -> call directly
	// without setup dialog
	result = plugin->start(*params);

	// maybe the start() function has called close() ?
	if (!m_plugin_instances.contains(plugin)) {
	    qDebug("PluginManager: plugin closed itself in start()");
	    result = -1;
            plugin = Q_NULLPTR;
	}

	if (plugin && (result >= 0)) {
	    plugin->execute(*params);
	}
    } else {
	// load previous parameters from config
	QStringList last_params = defaultParams(name);

	// call the plugin's setup function
	params = plugin->setup(last_params);
	if (params) {
	    // we have a non-zero parameter list, so
	    // the setup function has not been aborted.
	    // Now we can create a command string and
	    // emit a new command.

	    // store parameters for the next time
	    savePluginDefaults(name, *params);

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

    // now the plugin is no longer needed here, release it
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

    if (!m_plugin_instances.isEmpty())
	foreach (const KwavePluginPointer &plugin, m_plugin_instances)
	    if (plugin && !plugin->canClose()) return false;

    return true;
}

//***************************************************************************
bool Kwave::PluginManager::onePluginRunning()
{
    // check: this must be called from the GUI thread only!
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(this->thread() == qApp->thread());

    if (!m_plugin_instances.isEmpty())
	foreach (const KwavePluginPointer &plugin, m_plugin_instances)
	    if (plugin && plugin->isRunning()) return true;

    return false;
}

//***************************************************************************
void Kwave::PluginManager::sync()
{
    // check: this must be called from the GUI thread only!
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(this->thread() == qApp->thread());

    // this triggers all kinds of garbage collector (objects queued for
    // deletion through obj->deleteLater()
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    // wait until all plugins have finished their work...
    while (onePluginRunning()) {
	Kwave::yield();
	qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
	usleep(100000);
    }
}

//***************************************************************************
int Kwave::PluginManager::setupPlugin(const QString &name,
                                      const QStringList &params)
{
    // load the plugin
    Kwave::Plugin *plugin = createPluginInstance(name);
    if (!plugin) return -ENOMEM;

    // now the plugin is present and loaded
    QStringList prev_params = (params.isEmpty()) ?
	defaultParams(name) : params;

    // call the plugins' setup function
    QStringList *new_params = plugin->setup(prev_params);
    if (new_params) {
	// we have a non-zero parameter list, so
	// the setup function has not been aborted.
	savePluginDefaults(name, *new_params);
	delete new_params;
    } else {
	plugin->release();
	return 1;
    }

    plugin->release();
    return 0;
}

//***************************************************************************
QStringList Kwave::PluginManager::defaultParams(const QString &name)
{
    QString def_version;
    QString section = _("plugin ");
    QStringList list;
    section += name;

    // get the plugin version
    if (!m_plugin_modules.contains(name)) return list;
    const PluginModule &info = m_plugin_modules[name];
    QString version = info.m_version;

    Q_ASSERT(KSharedConfig::openConfig());
    if (!KSharedConfig::openConfig()) return list;
    KConfigGroup cfg = KSharedConfig::openConfig()->group(section);

    cfg.sync();

    def_version = cfg.readEntry("version");
    if (!def_version.length()) {
	return list;
    }
    if (!(def_version == version)) {
	qDebug("PluginManager::defaultParams: "
	    "plugin '%s': defaults for version '%s' not loaded, found "
	    "old ones of version '%s'.",
	    DBG(name), DBG(version), DBG(def_version));

	// delete the old settings
	cfg.deleteEntry("version");
	cfg.deleteEntry("defaults");

	return list;
    }

    list = cfg.readEntry("defaults").split(QLatin1Char(','));
    return list;
}

//***************************************************************************
void Kwave::PluginManager::savePluginDefaults(const QString &name,
                                              QStringList &params)
{

    // get the plugin version
    if (!m_plugin_modules.contains(name)) return;
    const PluginModule &info = m_plugin_modules[name];
    QString version = info.m_version;

    QString section = _("plugin ");
    section += name;

    Q_ASSERT(KSharedConfig::openConfig());
    if (!KSharedConfig::openConfig()) return;
    KConfigGroup cfg = KSharedConfig::openConfig()->group(section);

    cfg.sync();
    cfg.writeEntry("version", version);
    cfg.writeEntry("defaults", params.join(QLatin1Char(',')));
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
    unsigned int tracks,
    const Kwave::PlayBackParam *playback_params
)
{
    Kwave::PlayBackDevice *device =
	m_signal_manager.playbackController().openDevice(
	    tracks, playback_params);
    if (!device) return Q_NULLPTR;

    // create the multi track playback sink
    Kwave::SampleSink *sink = new(std::nothrow)
	Kwave::MultiPlaybackSink(tracks, device);
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
    if (!p) return;

    // disconnect the signals to avoid recursion
    disconnectPlugin(p);

    if (m_plugin_instances.contains(p))
        m_plugin_instances.removeAll(p);

    // schedule the deferred delete/unload of the plugin
    p->deleteLater();
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

    connect(this, SIGNAL(sigClosed()),
	    plugin, SLOT(close()));

    connect(plugin, SIGNAL(sigClosed(Kwave::Plugin*)),
	    this, SLOT(pluginClosed(Kwave::Plugin*)),
	    Qt::QueuedConnection);

    connect(plugin, SIGNAL(sigRunning(Kwave::Plugin*)),
	    this, SLOT(pluginStarted(Kwave::Plugin*)),
	    Qt::DirectConnection);

    connect(plugin, SIGNAL(sigDone(Kwave::Plugin*)),
	    this, SLOT(pluginDone(Kwave::Plugin*)),
	    Qt::QueuedConnection);
}

//***************************************************************************
void Kwave::PluginManager::disconnectPlugin(Kwave::Plugin *plugin)
{
    Q_ASSERT(plugin);
    if (!plugin) return;

    disconnect(plugin, SIGNAL(sigDone(Kwave::Plugin*)),
	       this, SLOT(pluginDone(Kwave::Plugin*)));

    disconnect(plugin, SIGNAL(sigRunning(Kwave::Plugin*)),
	       this, SLOT(pluginStarted(Kwave::Plugin*)));

    disconnect(this, SIGNAL(sigClosed()),
	       plugin, SLOT(close()));

    disconnect(plugin, SIGNAL(sigClosed(Kwave::Plugin*)),
	       this, SLOT(pluginClosed(Kwave::Plugin*)));

}

//***************************************************************************
void Kwave::PluginManager::setSignalName(const QString &name)
{
    emit sigSignalNameChanged(name);
}

//***************************************************************************
void Kwave::PluginManager::searchPluginModules()
{
    if (!m_plugin_modules.isEmpty()) {
	// this is not the first call -> increment module use count only
	for (QMap<QString, PluginModule>::iterator it(m_plugin_modules.begin());
	     it != m_plugin_modules.end(); ++it)
	{
	    it.value().m_use_count++;
	}
	return;
    }

    KPluginInfo::List plugins = KPluginInfo::fromMetaData(
	KPluginLoader::findPlugins(_("kwave"))
    );
    foreach (const KPluginInfo &i, plugins) {
	QString library     = i.libraryPath();
	QString description = i.name();
	QString name        = i.pluginName();
	QString version_raw = i.version();
	QString version;
	QString settings;
	QString author      = i.author();

	if (version_raw.contains(_(":"))) {
	    version  = version_raw.split(_(":")).at(0);
	    settings = version_raw.split(_(":")).at(1);
	}

// 	qDebug("file='%s', name='%s', description='%s', binary_version='%s', "
// 	       "settings_version='%s', author='%s'",
// 	       DBG(library), DBG(name), DBG(description), DBG(version),
// 	       DBG(settings), DBG(author)
// 	);

	if ( library.isEmpty() || description.isEmpty() ||
	     name.isEmpty() || version.isEmpty() ) {
	    qWarning("plugin '%s' has no library, name or version", DBG(name));
	    continue;
	}

	if (version != _(KWAVE_VERSION)) {
	    qWarning("plugin '%s' has wrong ABI version: '%s' (should be %s)",
		     DBG(name), DBG(version), KWAVE_VERSION);
	    continue;
	}

	KPluginLoader loader(library);
	KPluginFactory* factory = loader.factory();
	if (!factory) {
	    qWarning("plugin '%s': loading failed", DBG(name));
	    continue;
	}

	emit sigProgress(i18n("Loading plugin %1...", name));
	QApplication::processEvents();

	PluginModule info;
	info.m_name        = name;
	info.m_author      = author;
	info.m_description = i18n(description.toUtf8());
	info.m_version     = settings;
	info.m_factory     = factory;
	info.m_use_count   = 1;

	m_plugin_modules.insert(info.m_name, info);

	qDebug("%16s %5s written by %s", DBG(name), DBG(settings), DBG(author));
    }

    qDebug("--- \n found %d plugins\n", m_plugin_modules.count());
}

//***************************************************************************
const QList<Kwave::PluginManager::PluginModule>
    Kwave::PluginManager::pluginInfoList() const
{
    return m_plugin_modules.values();
}

//***************************************************************************
void Kwave::PluginManager::migratePluginToActiveContext(Kwave::Plugin *plugin)
{
    // check: this must be called from the GUI thread only!
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(this->thread() == qApp->thread());

    Q_ASSERT(plugin);
    if (!plugin) return;
    if (m_active_instance == this) return; // nothing to do
    Q_ASSERT(m_active_instance);
    if (!m_active_instance) return; // should never happen

    Kwave::PluginManager *old_mgr = this;
    old_mgr->m_plugin_instances.removeAll(plugin);
    old_mgr->m_running_plugins.removeAll(plugin);
    old_mgr->disconnectPlugin(plugin);

    Kwave::PluginManager *new_mgr = m_active_instance;
    new_mgr->m_plugin_instances.append(plugin);
    new_mgr->m_running_plugins.append(plugin);
    new_mgr->connectPlugin(plugin);

    plugin->setPluginManager(new_mgr);
}

//***************************************************************************
//***************************************************************************
