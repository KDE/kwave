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

#include <errno.h>
#include <new>

#include <QPointer>

#include "libkwave/MessageBox.h"
#include "libkwave/PluginManager.h"
#include "libkwave/SignalManager.h"
#include "libkwave/String.h"

#include "FileInfoDialog.h"
#include "FileInfoPlugin.h"

KWAVE_PLUGIN(fileinfo, FileInfoPlugin)

//***************************************************************************
Kwave::FileInfoPlugin::FileInfoPlugin(QObject *parent,
                                      const QVariantList &args)
    :Kwave::Plugin(parent, args)
{
}

//***************************************************************************
Kwave::FileInfoPlugin::~FileInfoPlugin()
{
}

//***************************************************************************
QStringList *Kwave::FileInfoPlugin::setup(QStringList &)
{
    Kwave::FileInfo oldInfo(signalManager().metaData());

    // create the setup dialog
    QPointer<Kwave::FileInfoDialog> dialog =
        new(std::nothrow) Kwave::FileInfoDialog(parentWidget(), oldInfo);
    Q_ASSERT(dialog);
    if (!dialog) return nullptr;

    QStringList *list = new(std::nothrow) QStringList();
    Q_ASSERT(list);
    if (list && dialog->exec() && dialog) {
        // user has pressed "OK" -> apply the new properties
        apply(dialog->info());
    } else {
        // user pressed "Cancel"
        delete list;
        list = nullptr;
    }

    delete dialog;
    return list;
}

//***************************************************************************
void Kwave::FileInfoPlugin::apply(Kwave::FileInfo &new_info)
{
    Kwave::FileInfo old_info(signalManager().metaData());
    if (old_info == new_info) return; // nothing to do

    /* sample rate */
    if (!qFuzzyCompare(old_info.rate(), new_info.rate())) {

        // sample rate changed -> only change rate or resample ?
        double new_rate = new_info.rate();
        int res = Kwave::MessageBox::questionYesNoCancel(parentWidget(),
            i18n("You have changed the sample rate. Do you want to convert "
                 "the whole file to the new sample rate or do "
                 "you only want to set the rate information in order "
                 "to repair a damaged file? Note: changing only the sample "
                 "rate can cause \"Mickey Mouse\" effects."),
            QString(),
            i18n("&Convert"),
            i18n("&Set Rate"));
        if (res == KMessageBox::PrimaryAction) {
            // Yes -> resample

            // take over all properties except the new sample rate, this will
            // be detected and changed in the sample rate plugin
            new_info.setRate(old_info.rate());
            if (new_info != old_info) {
                signalManager().setFileInfo(new_info, true);
            } // else: nothing except sample rate changed

            // NOTE: this command could be executed asynchronously, thus
            //       we cannot change the sample rate afterwards
            emitCommand(_("plugin:execute(samplerate,%1,all)").arg(new_rate));
            return;
        } else if (res == KMessageBox::SecondaryAction) {
            // No -> only change the rate in the file info
            new_info.setRate(new_rate);
        } else {
            // canceled -> use old sample rate
            new_info.setRate(old_info.rate());
        }
    }

    // just copy all other properties
    if (new_info != old_info) {
        signalManager().setFileInfo(new_info, true);
    }
}

//***************************************************************************
#include "FileInfoPlugin.moc"
//***************************************************************************
//***************************************************************************

#include "moc_FileInfoPlugin.cpp"
