/***************************************************************************
    NewSignalPlugin.cpp  -  plugin for creating a new signal
                             -------------------
    begin                : Wed Jul 18 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#include <qstringlist.h>

#include "NewSignalPlugin.h"
#include "NewSignalDialog.h"

KWAVE_PLUGIN(NewSignalPlugin,"newsignal","Thomas Eschenbacher");

//***************************************************************************
NewSignalPlugin::NewSignalPlugin(PluginContext &context)
    :KwavePlugin(context)
{
}

//***************************************************************************
NewSignalPlugin::~NewSignalPlugin()
{
}

//***************************************************************************
QStringList *NewSignalPlugin::setup(QStringList &previous_params)
{
    debug("NewSignalPlugin::setup");
    NewSignalDialog *dialog = new NewSignalDialog(parentWidget());
    ASSERT(dialog);
    if (!dialog) return false;

    if (dialog->exec()) {
	// user has pressed "OK"
    }

    if (dialog) delete dialog;
    return 0;
};

//***************************************************************************
//***************************************************************************
