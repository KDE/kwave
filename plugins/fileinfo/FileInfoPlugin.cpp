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
#include <kmessagebox.h>

#include "kwave/PluginManager.h"
#include "kwave/SignalManager.h"
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
    Q_ASSERT(dialog);
    if (!dialog) return 0;

    QStringList *list = new QStringList();
    Q_ASSERT(list);
    if (list && dialog->exec()) {
	// user has pressed "OK" -> apply the new properties
	apply(dialog->info());
    } else {
	// user pressed "Cancel"
	if (list) delete list;
	list = 0;
    }

    if (dialog) delete dialog;
    return list;
};

//***************************************************************************
void FileInfoPlugin::apply(FileInfo &new_info)
{
    if (fileInfo() == new_info) return; // nothing to do

    /* sample rate */
    if (fileInfo().rate() != new_info.rate()) {
	// sample rate changed -> only change rate or resample ?
	double new_rate = new_info.rate();
	int res = KMessageBox::questionYesNoCancel(parentWidget(),
	    i18n("You have changed the sample rate. Do you want to convert "
		 "the whole file to the new sample rate or do "
		 "you only want to set the rate information in order "
		 "to repair a damaged file? Note: changing only the sample "
		 "rate can cause \"mickey mouse\" effects!"),
	    0,
	    i18n("&Convert"), i18n("&Set Rate"));
	if (res == KMessageBox::Yes) {
	    // resample
	    emitCommand(QString("convert_rate(%1)").arg(new_rate));
	} else if (res == KMessageBox::No) {
	    // change the rate only
	    // fileInfo().setRate(new_rate);
	} else {
	    // cancelled -> use old setting
	    new_info.setRate(fileInfo().rate());
	}
    }

    // just copy all other properties
    signalManager().setFileInfo(new_info);
}

//***************************************************************************
//***************************************************************************
