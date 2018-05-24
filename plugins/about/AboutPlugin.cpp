/***************************************************************************
        AboutPlugin.cpp  -  plugin that shows the Kwave's about dialog
                             -------------------
    begin                : Sun Oct 29 2000
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

#include "config.h"

#include <errno.h>
#include <new>

#include <QPointer>

#include <KLocalizedString>

#include "libkwave/Plugin.h"
#include "libkwave/PluginManager.h"

#include "AboutDialog.h"
#include "AboutPlugin.h"

KWAVE_PLUGIN(about, AboutPlugin)

//***************************************************************************
Kwave::AboutPlugin::AboutPlugin(QObject *parent, const QVariantList &args)
    :Kwave::Plugin(parent, args)
{
}

//***************************************************************************
int Kwave::AboutPlugin::start(QStringList& params)
{
    Q_UNUSED(params);

    // create a new "about" dialog and show it
    QPointer<Kwave::AboutDialog> dlg = new(std::nothrow) Kwave::AboutDialog(
	parentWidget(),
	manager().pluginInfoList()
    );
    Q_ASSERT(dlg);
    if (!dlg) return ENOMEM;
    dlg->exec();
    delete dlg;

    return 0;
}

//***************************************************************************
#include "AboutPlugin.moc"
//***************************************************************************
//***************************************************************************
