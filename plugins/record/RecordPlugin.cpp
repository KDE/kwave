/*************************************************************************
       RecordPlugin.cpp  -  plugin for recording audio data
                             -------------------
    begin                : Wed Jul 09 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
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

#include <qstringlist.h>

#include "RecordDialog.h"
#include "RecordPlugin.h"

KWAVE_PLUGIN(RecordPlugin,"record","Thomas Eschenbacher");

//***************************************************************************
RecordPlugin::RecordPlugin(PluginContext &context)
    :KwavePlugin(context), m_params()
{
    i18n("record");
}

//***************************************************************************
RecordPlugin::~RecordPlugin()
{
}

//***************************************************************************
QStringList *RecordPlugin::setup(QStringList &previous_params)
{
    qDebug("RecordPlugin::setup");

    m_params.fromList(previous_params);

    // create the setup dialog
    RecordDialog *dialog = new RecordDialog(parentWidget(), m_params);
    Q_ASSERT(dialog);
    if (!dialog) return 0;

    QStringList *list = new QStringList();
    Q_ASSERT(list);
    if (list && dialog->exec()) {
	// user has pressed "OK"
	*list = dialog->params().toList();
    } else {
	// user pressed "Cancel"
	if (list) delete list;
	list = 0;
    }

    if (dialog) delete dialog;
    return list;
}

//***************************************************************************
//***************************************************************************
