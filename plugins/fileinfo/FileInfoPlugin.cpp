/***************************************************************************
     FileInfoPlugin.cpp  -  plugin for editing file properties
                             -------------------
    begin                : Fri Jul 19 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
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

#include "FileInfoDialog.h"
#include "FileInfoPlugin.h"

KWAVE_PLUGIN(FileInfoPlugin,"fileinfo","Thomas Eschenbacher");

//***************************************************************************
FileInfoPlugin::FileInfoPlugin(PluginContext &context)
    :KwavePlugin(context)
{
}

//***************************************************************************
FileInfoPlugin::~FileInfoPlugin()
{
}

//***************************************************************************
QStringList *FileInfoPlugin::setup(QStringList &)
{
    FileInfo oldInfo(fileInfo());

    // create the setup dialog
    FileInfoDialog *dialog = new FileInfoDialog(parentWidget(), oldInfo);
    ASSERT(dialog);
    if (!dialog) return 0;

    QStringList *list = new QStringList();
    ASSERT(list);
    if (list && dialog->exec()) {
	// user has pressed "OK" -> apply the new properties
	fileInfo().copy(dialog->info());
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
