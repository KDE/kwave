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

#include <kapp.h>
#include <kaboutapplication.h>
#include <kaboutkde.h>

#include "libkwave/KwavePlugin.h"
#include "AboutPlugin.h"
#include "AboutKwaveDialog.h"

KWAVE_PLUGIN(AboutPlugin,"about","Ralf Waspe & Gilles Caulier");

//***************************************************************************
AboutPlugin::AboutPlugin(PluginContext &c)
    :KwavePlugin(c)
{
    i18n("about");
}

//***************************************************************************
int AboutPlugin::start(QStringList& params)
{
    if (params.first() == "kde") {
        // create a new "about kde" dialog and show it
        KAboutKDE *dlg = new KAboutKDE(parentWidget());

        Q_ASSERT(dlg);
        if (!dlg) return ENOMEM;
        dlg->exec();
        delete dlg;
    } else {
        // create a new "about" dialog and show it
        AboutKwaveDialog *dlg = new AboutKwaveDialog(parentWidget());

        Q_ASSERT(dlg);
        if (!dlg) return ENOMEM;
        dlg->exec();
        delete dlg;
    }

    return 0;
}

//***************************************************************************
//***************************************************************************
