/***************************************************************************
  AmplifyFreePlugin.cpp  -  Plugin for free amplification curves
                             -------------------
    begin                : Sun Sep 02 2001
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
#include "errno.h"

#include <qstringlist.h>

#include "libkwave/Parser.h"
#include "AmplifyFreePlugin.h"
#include "AmplifyFreeDialog.h"

KWAVE_PLUGIN(AmplifyFreePlugin,"amplifyfree","Thomas Eschenbacher");

//***************************************************************************
AmplifyFreePlugin::AmplifyFreePlugin(PluginContext &context)
    :KwavePlugin(context), m_params()
{
}

//***************************************************************************
AmplifyFreePlugin::~AmplifyFreePlugin()
{
}

//***************************************************************************
int AmplifyFreePlugin::interpreteParameters(QStringList &params)
{
    // all params are curve params
    m_params = params;
    return 0;
}

//***************************************************************************
QStringList *AmplifyFreePlugin::setup(QStringList &previous_params)
{
    // try to interprete the previous parameters
    interpreteParameters(previous_params);

    // create the setup dialog
    AmplifyFreeDialog *dialog = new AmplifyFreeDialog(parentWidget());
    ASSERT(dialog);
    if (!dialog) return 0;

    if (!m_params.isEmpty()) dialog->setParams(m_params);

    QStringList *list = new QStringList();
    ASSERT(list);
    if (list && dialog->exec()) {
	// user has pressed "OK"
	QString cmd = dialog->getCommand();
	Parser p(cmd);
	while (!p.isDone()) *list << p.nextParam();
	emitCommand(cmd);
    } else {
	// user pressed "Cancel"
	if (list) delete list;
	list = 0;
    }

    if (dialog) delete dialog;
    return list;
};

//***************************************************************************
//***************************************************************************
