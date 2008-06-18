/***************************************************************************
        PluginContext.h  -  collects all references a plugin needs
                             -------------------
    begin                : Fri Jul 28 2000
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

#ifndef _PLUGIN_CONTEXT_H_
#define _PLUGIN_CONTEXT_H_

#include "config.h"

class KwaveApp;
class PluginManager;
class MenuManager;
class TopWidget;

class PluginContext
{
public:
    /** Constructor */
    PluginContext(
	KwaveApp      &app,
	PluginManager &plugin_mgr,
	MenuManager   *menu_mgr,
	TopWidget     &topwidget,
	void *mod_handle,
	const QString &mod_name,
	const QString &mod_version,
	const QString &mod_author
    )
    :kwave_app(app),
    manager(plugin_mgr),
    menu_manager(menu_mgr),
    top_widget(topwidget),
    handle(mod_handle),
    name(mod_name),
    version(mod_version),
    author(mod_author)
    {};

    KwaveApp      &kwave_app;
    PluginManager &manager;
    MenuManager   *menu_manager;
    TopWidget     &top_widget;

    void *handle;
    QString name;
    QString version;
    QString author;
};

#endif // _PLUGIN_CONTEXT_H_

/* end of libgui/PluginContext.h */
