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

class KwaveApp;
class PluginManager;
class MenuManager;
class TopWidget;
class SignalManager;
class LabelManager;

class PluginContext
{
public:
    /** Constructor */
    PluginContext(
	KwaveApp      &app,
	PluginManager &plugin_mgr,
	LabelManager  *label_mgr,
	MenuManager   *menu_mgr,
	TopWidget     &topwidget,
	SignalManager *signal_mgr,
	void *mod_handle
    )
    :kwave_app(app),
    manager(plugin_mgr),
    label_manager(label_mgr),
    menu_manager(menu_mgr),
    top_widget(topwidget),
    signal_manager(signal_mgr)
    {};

    KwaveApp      &kwave_app;
    PluginManager &manager;
    LabelManager  *label_manager;
    MenuManager   *menu_manager;
    TopWidget     &top_widget;
    SignalManager *signal_manager;

    void *handle;
};

#endif // _PLUGIN_CONTEXT_H_

/* end of libgui/PluginContext.h */
