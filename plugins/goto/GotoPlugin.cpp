/***************************************************************************
         GotoPlugin.cpp  -  Plugin for moving the view to a certain position
                             -------------------
    begin                : Sat Dec 06 2008
    copyright            : (C) 2008 by Thomas Eschenbacher
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

#include <QString>
#include <klocale.h>

#include "libkwave/Plugin.h"
#include "GotoPlugin.h"

KWAVE_PLUGIN(GotoPlugin, "goto", "2.1",
             I18N_NOOP("Goto Position"), "Thomas Eschenbacher");

//***************************************************************************
GotoPlugin::GotoPlugin(const PluginContext &c)
    :GotoPluginBase(c)
{
}

//***************************************************************************
GotoPlugin::~GotoPlugin()
{
}

//***************************************************************************
QString GotoPlugin::command() const
{ 
    return "goto";
}
    
//***************************************************************************
QString GotoPlugin::title() const 
{
    return i18n("Goto...");
}

//***************************************************************************
//***************************************************************************
