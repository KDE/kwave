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

#include "libkwave/Plugin.h"
#include "libkwave/PluginManager.h"

#include "AboutPlugin.h"
#include "AboutDialog.h"

KWAVE_PLUGIN(Kwave::AboutPlugin, "about", "2.1",
             I18N_NOOP("About Kwave"), "Ralf Waspe & Gilles Caulier");

//***************************************************************************
Kwave::AboutPlugin::AboutPlugin(const Kwave::PluginContext &c)
    :Kwave::Plugin(c)
{
}

//***************************************************************************
int Kwave::AboutPlugin::start(QStringList& params)
{
    Q_UNUSED(params);

    // create a new "about" dialog and show it
    Kwave::AboutDialog *dlg = new Kwave::AboutDialog(
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
