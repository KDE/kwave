/***************************************************************************
     InsertAtPlugin.cpp  -  plugin for insertin the clipboard at a position
                             -------------------
    begin                : Thu May 12 2011
    copyright            : (C) 2011 by Thomas Eschenbacher
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

#include "libkwave/KwavePlugin.h"
#include "InsertAtPlugin.h"

KWAVE_PLUGIN(InsertAtPlugin, "insert_at", "2.1",
             I18N_NOOP("Insert At"), "Thomas Eschenbacher");

//***************************************************************************
InsertAtPlugin::InsertAtPlugin(const PluginContext &c)
    :GotoPluginBase(c)
{
}

//***************************************************************************
InsertAtPlugin::~InsertAtPlugin()
{
}

//***************************************************************************
QString InsertAtPlugin::command() const
{
    return "insert_at";
}
   
//***************************************************************************
QString InsertAtPlugin::title() const 
{
    return i18n("Insert At...");
}

//***************************************************************************
//***************************************************************************
