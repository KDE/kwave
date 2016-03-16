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

#include <KLocalizedString>
#include <QString>

#include "libkwave/Plugin.h"
#include "libkwave/String.h"

#include "InsertAtPlugin.h"

KWAVE_PLUGIN(insert_at, InsertAtPlugin)

//***************************************************************************
Kwave::InsertAtPlugin::InsertAtPlugin(QObject *parent,
                                      const QVariantList &args)
    :Kwave::GotoPluginBase(parent, args)
{
}

//***************************************************************************
Kwave::InsertAtPlugin::~InsertAtPlugin()
{
}

//***************************************************************************
QString Kwave::InsertAtPlugin::command() const
{
    return _("insert_at");
}

//***************************************************************************
QString Kwave::InsertAtPlugin::title() const
{
    return i18n("Insert At...");
}

//***************************************************************************
#include "InsertAtPlugin.moc"
//***************************************************************************
//***************************************************************************
