/***************************************************************************
          PluginManager.cpp -  manager class for kwave's plugins
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

#include <kglobal.h>
#include <kconfig.h>
#include <kmainwindow.h>
#include <kstddirs.h>
#include <kmessagebox.h>
#include <klocale.h>

#include <libkwave/Parser.h>
#include <libkwave/LineParser.h>
#include <libkwave/FileLoader.h>

#include "mt/SignalProxy.h"

#include "libgui/KwavePlugin.h"
#include "libgui/PluginContext.h"

#include "KwaveApp.h"
#include "TopWidget.h"
#include "SignalManager.h"

#include "PluginManager.h"

//#include <sys/types.h>
//#include <sys/stat.h>
//
//#ifdef IN_GCC
//#include "gansidecl.h"
//#define PARAMS(ARGS) PROTO(ARGS)
//#else /* ! IN_GCC */
//#include <ansidecl.h>
//#endif /* IN_GCC */
//
//extern const char *cplus_mangle_opname PARAMS ((const char
//	*opname, int options));

//****************************************************************************
//****************************************************************************

// static initializer
QMap<QString, QString> PluginManager::m_plugin_files;

//****************************************************************************
PluginManager::PluginManager(TopWidget &parent)
    :m_top_widget(parent)
{
    m_spx_name_changed = new SignalProxy1<const QString>(
	this, SLOT(emitNameChanged()));
}

//****************************************************************************
bool PluginManager::isOK()
{
    return true;//(spx_name_changed);
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
//	    debug("PluginManager::~PluginManager(): closing plugin(%p:%p)",p->getHandle(),p);
	    pluginClosed(p, true);
	} else {
	    debug("PluginManager::~PluginManager(): removing null plugin");
	    m_loaded_plugins.removeRef(p);
	}
    }
}

//**********************************************************
void *PluginManager::loadPlugin(const QString &name)
{
    /* show a warning and abort if the plugin is unknown */
    if (!(m_plugin_files.contains(name))) {
	char message[256];
	snprintf(message, 256, i18n("oops, plugin '%s' is unknown !"), name.data());
	KMessageBox::error(&m_top_widget,
	    (const char *)&message,
	    i18n("error on loading plugin")
	);
	return 0;
    }

    QString &filename = m_plugin_files[name];

    /* try to get the file handle of the plugin's binary */
    void *handle = dlopen(filename, RTLD_NOW);
    if (!handle) {
	char message[256];

	snprintf(message, 256, i18n(
	    "unable to load the file \n'%s'\n that contains the plugin '%s' !"),
	    filename.data(), name.data()
	);
	KMessageBox::error(&m_top_widget,
	    (const char *)&message,
	    i18n("error on loading plugin")
	);
	return 0;
    }

    return handle;
}

//**********************************************************
void PluginManager::executePlugin(const QString &name, QStrList *params)
{
    QString command;

    // load the plugin
    void* handle = loadPlugin(name);
    if (!handle) return;

    KwavePlugin *(*plugin_loader)(PluginContext *c) = 0;

#ifdef HAVE_CPLUS_MANGLE_OPNAME
    // would be fine, but needs libiberty
    const char *sym_loader  = cplus_mangle_opname("load(PluginContext *)",0);
    const char *sym_version = cplus_mangle_opname("const char *", 0);
#else
    // hardcoded, fails on some systems :-(
    const char *sym_loader  = "load__FR13PluginContext";
    const char *sym_version = "version";
#endif

    const char *version = "";
    const char **pver = (const char **)dlsym(handle, sym_version);
    ASSERT(pver);
    if (pver) version=*pver;
    if (!version) version = "?";

    plugin_loader = (KwavePlugin *(*)(PluginContext *))dlsym(handle, sym_loader);
    ASSERT(plugin_loader);
    if (plugin_loader) {
	PluginContext *context = new PluginContext(
            m_top_widget.getKwaveApp(),
            *this,
	    0, // LabelManager  *label_mgr,
	    0, // MenuManager   *menu_mgr,
	    m_top_widget,
	    handle
	);

	ASSERT(context);
	if (context) {
	    // call the loader function
	    debug("PluginManager: loading plugin instance"); // ###
	    KwavePlugin *plugin = (*plugin_loader)(context);
	    debug("PluginManager: loading plugin instance done."); // ###
	
	    ASSERT(plugin);
	    if (plugin) {
		// append the plugin into our list of plugins
		m_loaded_plugins.append(plugin);

		// connect all signals
		connectPlugin(plugin);

		// now the plugin is present and loaded
		QStrList *last_params = loadPluginDefaults(name, version);

		if (params) {
		    // parameters were specified -> call directly
		    // without setup dialog
		    debug("PluginManager: starting plugin"); // ###
		    int result = plugin->start(*params);
		    debug("PluginManager: result: %d", result);// ###

		    // maybe the start() function has called close() ?
		    if (m_loaded_plugins.findRef(plugin) == -1) {
			debug("PluginManager: plugin closed itself in start()"); // ###
			result = -1;
			plugin = 0;
			handle = 0;
		    }
		
		    if (plugin && (result >= 0)) {
			debug("PluginManager: executing plugin"); // ###
			plugin->execute(*params);
			debug("PluginManager: execute plugin done"); // ###

			// now it's the the task of the plugin's thread
			// to clean up after execution
			plugin = 0;
			handle = 0;
			context = 0;
		    };
		} else {
	            // call the plugin's setup function
		    params = plugin->setup(last_params);
		
		    if (params) {
			// we have a non-zero parameter list, so
			// the setup function has not been aborted.
			// Now we can create a command string and
			// emit a new command.

			savePluginDefaults(name, version, *params);

			// We DO NOT call the plugin's "execute"
			// function directly, as it should be possible
			// to record all function calls in the
			// macro recorder
			
			command = "plugin:execute(";
			command += name;
			for (unsigned int i=0; i < params->count(); i++) {
			    command += ", ";
			    command += params->at(i);
			}
			delete params;
			command += ")";
			debug("PluginManager: command='%s'",command.data()); // ###
		    }
		}

		// previous parameters are no longer needed
		if (last_params) delete last_params;
		last_params=0;
		
		// now the plugin is no longer needed here, so delete it
		// if it has not already been detached
		if (plugin) {
		    pluginClosed(plugin, true);
		    handle = 0;
		}
	    } else {
		// plugin is null, out of memory or not found
		warning("plugin = null");
		delete context;
	    }
	}
    } else warning("%s", dlerror());

    if (handle) dlclose(handle);
    if (!command.isNull()) emit sigCommand(command);
}

//***************************************************************************
QStrList *PluginManager::loadPluginDefaults(const QString &name,
	const QString &version)
{
    QString def_version;
    QString section("plugin ");
    section += name;

    KConfig *cfg = KGlobal::config();
    ASSERT(cfg);
    if (!cfg) return 0;

    cfg->sync();
    cfg->setGroup(section);

    def_version = cfg->readEntry("version");
    if (!def_version.length()) {
	debug("PluginManager::loadPluginDefaults: "\
	      "plugin '%s': no defaults found", name.data());
	return 0;
    }
    if (!(def_version == version)) {
	debug("PluginManager::loadPluginDefaults: "\
	    "plugin '%s': defaults for version '%s' not loaded, found "\
	    "old ones of version '%s'.", name.data(), version.data(),
	    def_version.data());
    }

    QStrList list;
    int count = cfg->readListEntry("defaults", list, ',');
    debug("PluginManager::loadPluginDefaults: list with %d entries loaded",
	  count);

    return new QStrList(list);
}

//***************************************************************************
void PluginManager::savePluginDefaults(const QString &name,
                                       const QString &version,
                                       QStrList &params)
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
    SignalManager *sig = m_top_widget.getSignalManager();
    return ((sig) ? sig->getLength() : 0);
}

//***************************************************************************
unsigned int PluginManager::signalRate()
{
    SignalManager *sig = m_top_widget.getSignalManager();
    return ((sig) ? sig->getRate() : 0);
}

//***************************************************************************
const QArray<unsigned int> PluginManager::selectedChannels()
{
    const QArray<unsigned int> empty;

    SignalManager *sig = m_top_widget.getSignalManager();
    return ((sig) ? (sig->selectedChannels()) : (empty));
}

//***************************************************************************
unsigned int PluginManager::selectionStart()
{
    SignalManager *sig = m_top_widget.getSignalManager();
    return (sig) ? sig->getLMarker() : 0;
}

//***************************************************************************
unsigned int PluginManager::selectionEnd()
{
    SignalManager *sig = m_top_widget.getSignalManager();
    return (sig) ? sig->getRMarker() : 0;
}

//***************************************************************************
int PluginManager::singleSample(unsigned int channel, unsigned int offset)
{
    SignalManager *sig = m_top_widget.getSignalManager();
    return (sig) ? sig->singleSample(channel, offset) : 0;
}

//***************************************************************************
int PluginManager::averageSample(unsigned int offset,
                                 const QArray<unsigned int> *channels)
{
    SignalManager *sig = m_top_widget.getSignalManager();
    return (sig) ? sig->averageSample(offset, channels) : 0;
}

//***************************************************************************
QBitmap *PluginManager::overview(unsigned int width, unsigned int height,
                                 unsigned int offset, unsigned int length)
{
    SignalManager *sig = m_top_widget.getSignalManager();
    return (sig) ? sig->overview(width, height, offset, length) : 0;
}

//***************************************************************************
void PluginManager::pluginClosed(KwavePlugin *p, bool remove)
{
    ASSERT(p);
    ASSERT(!m_loaded_plugins.isEmpty());

    if (p) {
//	void *h = p->getHandle();
	// disconnect the signals to avoid recursion
	disconnectPlugin(p);

	m_loaded_plugins.setAutoDelete(false);
	m_loaded_plugins.removeRef(p);

	if (remove) {
	    debug("PluginManager::pluginClosed(%p): deleting",p);
	    delete p;
//	    if (h) dlclose(h);
	}

    }

    debug("PluginManager::pluginClosed(): done");
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
    ASSERT(m_spx_name_changed);
    if (!m_spx_name_changed) return;

    const QString *name = m_spx_name_changed->dequeue();
    ASSERT(name);
    if (name) {
	emit sigSignalNameChanged(*name);
	delete name;
    }
}

//****************************************************************************
void PluginManager::setSignalName(const QString &name)
{
    ASSERT(m_spx_name_changed);
    if (!m_spx_name_changed) return;
    m_spx_name_changed->enqueue(name);
}

//***************************************************************************
void PluginManager::findPlugins()
{
    unsigned int i;
    QStringList files = KGlobal::dirs()->findAllResources("data",
	    "kwave/plugins/*", false, true);

    for (i=0; i < files.count(); i++) {

	void *handle = dlopen(files[i].data(), RTLD_NOW);
	if (handle) {
	    const char **name = (const char **)dlsym(handle, "name");
	    const char **version = (const char **)dlsym(handle, "version");
	    const char **author = (const char **)dlsym(handle, "author");

	    // skip it if something is missing or null
	    if (!name || !version || !author) continue;
	    if (!*name || !*version || !*author) continue;

	    m_plugin_files.insert(*name, files[i]);
	    printf("%10s %5s written by %s", *name, *version, *author);

	    dlclose (handle);
	}
	else
	    printf("error in '%s':\r\n\t %s\r\n", files[i].data(),
		dlerror());

	printf("\r\n");
    }

    printf("--- \r\n found %d plugins\r\n", m_plugin_files.count());
}

//****************************************************************************
//****************************************************************************
/* end of PluginManager.cpp */
