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
class MenuManager;
class TopWidget;
class SignalManager;
class LabelManager;

class PluginContext
{
public:
    KwaveApp      *kwave_app;
    LabelManager  *label_manager;
    MenuManager   *menu_manager;
    TopWidget     *top_widget;
    SignalManager *signal_manager;

    void *handle;
};

 #endif // _PLUGIN_CONTEXT_H_

/* end of libgui/PluginContext.h */
