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

#include <kmsgbox.h>
#include <kapp.h>

#include <libkwave/DynamicLoader.h>
#include <libkwave/DialogOperation.h>
#include <libkwave/Parser.h>
#include <libkwave/LineParser.h>
#include <libkwave/Global.h>
#include <libkwave/FileLoader.h>

#include <libkwave/Plugin.h>

#include "KwaveApp.h"
#include "TopWidget.h"
#include "libgui/AsyncSyncGuard.h"
#include "libgui/KwavePlugin.h"
#include "libgui/PluginContext.h"

#include "PluginManager.h"

extern Global globals;

//****************************************************************************
//****************************************************************************

PluginManager::PluginManager(TopWidget &parent)
:top_widget(parent)
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
    AsyncSyncGuard sync;

    // inform all plugins and client windows that we close now
    emit sigClosed();

    // remove all remaining plugins
    while (!loaded_plugins.isEmpty()) {
	KwavePlugin *p = loaded_plugins.last();
	if (p) {
//	    debug("PluginManager::~PluginManager(): closing plugin(%p:%p)",p->getHandle(),p);
	    pluginClosed(p, true);
	} else {
	    debug("PluginManager::~PluginManager(): removing null plugin");
	    loaded_plugins.removeRef(p);
	}
    }
}

//**********************************************************
void PluginManager::executePlugin(const char *name, QStrList *params)
{
    QString command;

    /* find the plugin in the global plugin list */
    unsigned int index = 0;
    bool found = false;
    for (index=0; globals.dialogplugins[index]; index++) {
	Plugin *p = globals.dialogplugins[index];
	if (strcmp(name, p->getName()) == 0) {
	    found=true;
	    break;
	}
    }

    /* show a warning and abort if the plugin was not found */
    if ((!found) || (globals.dialogplugins[index] == 0)) {
	char message[256];
	
	snprintf(message, 256, i18n("oops, plugin '%s' is unknown !"), name);
	KMsgBox::message(&top_widget,
	    i18n("error on loading plugin"),
	    (const char *)&message,
	    KMsgBox::EXCLAMATION
	);
	return;
    }

    /* try to get the file handle of the plugin's binary */
    void *handle = dlopen(globals.dialogplugins[index]->getFileName(),
	RTLD_NOW);
    if (!handle) {
	char message[256];

	snprintf(message, 256, i18n(
	    "unable to load the file \n'%s'\n that contains the plugin '%s' !"),
	    globals.dialogplugins[index]->getFileName(), name
	);
	KMsgBox::message(&top_widget,
	    i18n("error on loading plugin"),
	    (const char *)&message,
	    KMsgBox::EXCLAMATION
	);
	return;
    }

    KwavePlugin *(*plugin_loader)(PluginContext *c) = 0;

#ifdef HAVE_CPLUS_MANGLE_OPNAME
    // would be fine, but needs libiberty
    const char *sym=cplus_mangle_opname("load(PluginContext *)",0);
#else
    // hardcoded, fails on some systems :-(
    const char *sym = "load__FR13PluginContext";
#endif

    plugin_loader = (KwavePlugin *(*)(PluginContext *))dlsym(handle, sym);
    ASSERT(plugin_loader);
    if (plugin_loader) {
	PluginContext *context = new PluginContext(
            top_widget.getKwaveApp(),
            *this,
	    0, // LabelManager  *label_mgr,
	    0, // MenuManager   *menu_mgr,
	    top_widget,
	    0, // SignalManager *signal_mgr,
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
		loaded_plugins.append(plugin);

		// connect all signals
		connectPlugin(plugin);

		// now the plugin is present and loaded
		QStrList *last_params = 0;

		if (params) {
		    // parameters were specified -> call directly
		    // without setup dialog
		    debug("PluginManager: starting plugin"); // ###
		    int result = plugin->start(*params);
		    debug("PluginManager: result: %d", result);// ###

		    // maybe the start() function has called close() ?
		    if (loaded_plugins.findRef(plugin) == -1) {
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
		    }
		}
		
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

//**********************************************************
void PluginManager::pluginClosed(KwavePlugin *p, bool remove)
{
    ASSERT(p);
    ASSERT(!loaded_plugins.isEmpty());

//    debug("before:");
//    for (int i=0; i<loaded_plugins.count();i++) {
//	debug("plugin[%d]=%p",i,loaded_plugins.at(i));
//    }
//
//    debug("PluginManager::pluginClosed(%p:%p) [slot]",p?p->getHandle():0,p);
    if (p) {
//	void *h = p->getHandle();

	// disconnect the signals to avoid recursion
	disconnectPlugin(p);

//	debug("PluginManager::pluginClosed(%p:%p): removeRef",h,p);
	loaded_plugins.setAutoDelete(false);
	loaded_plugins.removeRef(p);

	if (remove) {
	    debug("PluginManager::pluginClosed(%p): deleting",p);
	    delete p;

//	    debug("PluginManager::pluginClosed(): closing handle %p",h);
//	    if (h) dlclose(h);
	}

    }

//    debug("after:");
//    for (int i=0; i<loaded_plugins.count();i++) {
//	debug("plugin[%d]=%p",i,loaded_plugins.at(i));
//    }

}

//****************************************************************************
void PluginManager::connectPlugin(KwavePlugin *plugin)
{
    ASSERT(plugin);
    if (!plugin) return;

    connect(this, SIGNAL(sigClosed()),
	    plugin, SLOT(close()));
    connect(this, SIGNAL(sigSignalNameChanged(const QString &)),
            plugin, SLOT(setSignalName(const QString &)));

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
    disconnect(this, SIGNAL(sigSignalNameChanged(const QString &)),
               plugin, SLOT(setSignalName(const QString &)));

    disconnect(plugin, SIGNAL(sigClosed(KwavePlugin *,bool)),
	       this, SLOT(pluginClosed(KwavePlugin *,bool)));

}

//****************************************************************************
void PluginManager::setSignalName(const QString &name)
{
    AsyncSyncGuard sync;
    debug("PluginManager::setSignalName(%s)",name.data());
    emit sigSignalNameChanged(name);
}

//****************************************************************************
//****************************************************************************
/* end of PluginManager.cpp */
