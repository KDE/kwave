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

#include "mt/TSS_Object.h"
#include "mt/ThreadCondition.h"
#include "mt/ThreadsafeX11Guard.h"

#include "libkwave/Parser.h"
#include "libkwave/LineParser.h"
#include "libkwave/FileLoader.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SampleWriter.h"

#include "mt/SignalProxy.h"

#include "libgui/KwavePlugin.h"
#include "libgui/PluginContext.h"

#include "KwaveApp.h"
#include "TopWidget.h"
#include "SignalManager.h"
#include "UndoAction.h"
#include "UndoModifyAction.h"
#include "UndoTransactionGuard.h"

#include "PluginManager.h"

//****************************************************************************
//****************************************************************************

//class PluginGarbageCollector: public Thread
//{
//public:
//    virtual void run();
//
//slots:
//    void remove(KwavePlugin *plugin);
//
//private:
//    QQueue<Plugin> m_queue;
//    SignalProxy1<KwavePlugin *> m_spx_plugin;
//}
//
//void PluginGarbageCollector::run()
//{
//    KwavePlugin *plugin;
//    do {
//	plugin = m_spx_plugin.dequeue();
//	if (!plugin) {
//	    // dequeued a null plugin -> signal to shut down
//	    debug("PluginGarbageCollector: shutting down...");
//	    break;
//	}
//	
//	// wait until the plugin's thread is down
//	
//	// delete the plugin
//	
//	// remove it's handle
//	
//    } while (plugin);
//}

// static initializer
QMap<QString, QString> PluginManager::m_plugin_files;

//****************************************************************************
PluginManager::PluginManager(TopWidget &parent)
    :m_spx_name_changed(this, SLOT(emitNameChanged())),
     m_spx_command(this, SLOT(forwardCommand())),
     m_top_widget(parent)
{
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

    // remove all remaining plugins
    while (!m_loaded_plugins.isEmpty()) {
	KwavePlugin *p = m_loaded_plugins.last();
	if (p) {
	    debug("PluginManager::~PluginManager(): closing plugin(%s)",
	           p->name().data());
	    pluginClosed(p, true);
	} else {
	    debug("PluginManager::~PluginManager(): removing null plugin");
	    m_loaded_plugins.removeRef(p);
	}
    }
}

//***************************************************************************
void PluginManager::loadAllPlugins()
{
    // try to load all plugins
    // NOTE: this also makes it resident if it is persistent
    QMap<QString, QString>::Iterator it;
    for (it=m_plugin_files.begin(); it != m_plugin_files.end(); ++it) {
	QString name = it.key();
	
	KwavePlugin *plugin = loadPlugin(name);
	if (plugin) {
	    // remove it again if it is not persistent
	    if (!plugin->isPersistent()) pluginClosed(plugin, true);
	} else {
	    // loading failed => remove it from the list
	    warning("PluginManager::loadAllPlugins(): removing '%s' "\
	            "from list", name.data());
	    m_plugin_files.remove(name);
	}
    }
}

//**********************************************************
KwavePlugin *PluginManager::loadPlugin(const QString &name)
{
    // first find out if the plugin is already loaded AND persistent
    // or unique
    QListIterator<KwavePlugin> it(m_loaded_plugins);
    for ( ; it.current(); ++it ) {
	KwavePlugin *p = it.current();
	if (p->name() == name) {
	    if (p->isPersistent()) {
		// persistent -> return reference only
		debug("PluginManager::loadPlugin(): returning reference "\
		      "to persistent plugin '%s'", name.data());
		return p;
	    }
	    if (p->isUnique()) {
		// prevent from re-loading of a unique plugin
		warning("PluginManager::loadPlugin(): attempt to re-load "\
		        "unique plugin '%s'", name.data());
		return 0;
	    }
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

#ifdef HAVE_CPLUS_MANGLE_OPNAME
    // would be fine, but needs libiberty
    const char *sym_version = cplus_mangle_opname("const char *", 0);
    const char *sym_author  = cplus_mangle_opname("author", 0);
    const char *sym_loader  = cplus_mangle_opname("load(PluginContext *)",0);
#else
    // hardcoded, might fail on some systems :-(
    const char *sym_version = "version";
    const char *sym_author  = "author";
    const char *sym_loader  = "load__FR13PluginContext";
#endif

    // get the plugin's author
    const char *author = "";
    const char **pauthor = (const char **)dlsym(handle, sym_author);
    ASSERT(pauthor);
    if (pauthor) author=*pauthor;
    if (!author) author = i18n("(unknown)");

    // get the plugin's version string
    const char *version = "";
    const char **pver = (const char **)dlsym(handle, sym_version);
    ASSERT(pver);
    if (pver) version=*pver;
    if (!version) version = i18n("(unknown)");

    plugin_loader = (KwavePlugin *(*)(PluginContext *))dlsym(handle, sym_loader);
    ASSERT(plugin_loader);
    if (!plugin_loader) {
	// plugin is null, out of memory or not found
	warning("PluginManager::loadPlugin(): "\
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

    ASSERT(context);
    if (!context) {
	warning("PluginManager::loadPlugin(): out of memory");
	dlclose(handle);
	return 0;
    }

    // call the loader function to create an instance
    KwavePlugin *plugin = (*plugin_loader)(context);
    ASSERT(plugin);
    if (!plugin) {
	warning("PluginManager::loadPlugin(): out of memory");
	dlclose(handle);
	return 0;
    }

    // append the plugin into our list of plugins
    m_loaded_plugins.append(plugin);

    // connect all signals
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

    // load the plugin
    KwavePlugin *plugin = loadPlugin(name);
    if (!plugin) return -ENOMEM;

    if (params) {
	// parameters were specified -> call directly
	// without setup dialog
	result = plugin->start(*params);
	
	// maybe the start() function has called close() ?
	if (m_loaded_plugins.findRef(plugin) == -1) {
	    debug("PluginManager: plugin closed itself in start()"); // ###
	    result = -1;
	    plugin = 0;
	}
	
	if (plugin && (result >= 0)) {
	    plugin->execute(*params);
	
	    // now it's the the task of the plugin's thread
	    // to clean up after execution
	    plugin = 0;
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
	    debug("PluginManager: command='%s'",command.data()); // ###
	}
    }

    // now the plugin is no longer needed here, so delete it
    // if it has not already been detached
    if (plugin && !plugin->isPersistent()) pluginClosed(plugin, true);

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
	{
//	    MutexGuard lock_list(m_lock_plugin_list);
	    QListIterator<KwavePlugin> it(m_loaded_plugins);
	    one_is_running = false;
	    for (; it.current(); ++it) {
		KwavePlugin *plugin = it.current();
		if (plugin->isRunning()) {
		    debug("waiting for plugin '%s'", plugin->name().data());
		    one_is_running = true;
		    break;
		}
	    }
	}
	if (one_is_running) qApp->processEvents();
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
    if (!plugin->isPersistent()) pluginClosed(plugin, true);

    return 0;
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
    ASSERT(cfg);
    if (!cfg) return list;

    cfg->sync();
    cfg->setGroup(section);

    def_version = cfg->readEntry("version");
    if (!def_version.length()) {
	return list;
    }
    if (!(def_version == version)) {
	debug("PluginManager::loadPluginDefaults: "\
	    "plugin '%s': defaults for version '%s' not loaded, found "\
	    "old ones of version '%s'.", name.data(), version.data(),
	    def_version.data());
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
    QString def_version;
    QString section("plugin ");
    section += name;

    KConfig *cfg = KGlobal::config();
    ASSERT(cfg);
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
const QArray<unsigned int> PluginManager::selectedTracks()
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
    const QArray<unsigned int> &track_list,
    unsigned int first, unsigned int last)
{
    SignalManager &manager = m_top_widget.signalManager();
    manager.openMultiTrackReader(readers, track_list, first, last);
}

//***************************************************************************
void PluginManager::openMultiTrackWriter(MultiTrackWriter &writers,
    const QArray<unsigned int> &track_list, InsertMode mode,
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
    QArray<unsigned int> tracks = manager.selectedTracks();
    unsigned int left  = selectionStart();
    unsigned int right = selectionEnd();
    if (left == right) {
	left = 0;
	right = signalLength()-1;
    }

    openMultiTrackWriter(writers, tracks, mode, left, right);
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
void PluginManager::pluginClosed(KwavePlugin *p, bool remove)
{
    ASSERT(p);
    ASSERT(!m_loaded_plugins.isEmpty());

    if (p) {
//	void *h = p->handle();
	
	// disconnect the signals to avoid recursion
	disconnectPlugin(p);

	m_loaded_plugins.setAutoDelete(false);
	m_loaded_plugins.removeRef(p);

	if (remove) {
//	    debug("PluginManager::pluginClosed(%p): deleting",p);
	    delete p;
//	    if (h) dlclose(h);
	}

    }

//    debug("PluginManager::pluginClosed(): done");
}

//****************************************************************************
void PluginManager::connectPlugin(KwavePlugin *plugin)
{
    ASSERT(plugin);
    if (!plugin) return;

    connect(this, SIGNAL(sigClosed()),
	    plugin, SLOT(close()));

    connect(plugin, SIGNAL(sigClosed(KwavePlugin *,bool)),
	    this, SLOT(pluginClosed(KwavePlugin *,bool)));

}

//****************************************************************************
void PluginManager::disconnectPlugin(KwavePlugin *plugin)
{
    ASSERT(plugin);
    if (!plugin) return;

    disconnect(this, SIGNAL(sigClosed()),
	       plugin, SLOT(close()));

    disconnect(plugin, SIGNAL(sigClosed(KwavePlugin *,bool)),
	       this, SLOT(pluginClosed(KwavePlugin *,bool)));

}

//****************************************************************************
void PluginManager::emitNameChanged()
{
    const QString *name = m_spx_name_changed.dequeue();
    ASSERT(name);
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
	    printf("%16s %5s written by %s", *name, *version, *author);
	
	    dlclose (handle);
	
	} else {
	    printf("error in '%s':\n\t %s\n", file.data(),
		dlerror());
	}
	
	printf("\n");
    }

    printf("--- \n found %d plugins\n", m_plugin_files.count());
}

//****************************************************************************
//****************************************************************************
/* end of PluginManager.cpp */
