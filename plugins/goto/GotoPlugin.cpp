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

#include <KLocalizedString>
#include <QString>

#include "libkwave/Plugin.h"
#include "libkwave/String.h"

#include "GotoPlugin.h"

KWAVE_PLUGIN(goto, GotoPlugin)

//***************************************************************************
Kwave::GotoPlugin::GotoPlugin(QObject *parent, const QVariantList &args)
    :Kwave::GotoPluginBase(parent, args)
{
}

//***************************************************************************
Kwave::GotoPlugin::~GotoPlugin()
{
}

//***************************************************************************
QString Kwave::GotoPlugin::command() const
{
    return _("goto");
}

//***************************************************************************
QString Kwave::GotoPlugin::title() const
{
    return i18n("Goto...");
}

//***************************************************************************
#include "GotoPlugin.moc"
//***************************************************************************
//***************************************************************************

#include "moc_GotoPlugin.cpp"
