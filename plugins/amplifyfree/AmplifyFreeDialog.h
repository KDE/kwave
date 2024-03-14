/***************************************************************************
    AmplifyFreeDialog.h  -  dialog for the "amplifyfree" plugin
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

#ifndef AMPLIFY_FREE_DIALOG_H
#define AMPLIFY_FREE_DIALOG_H

#include "config.h"

#include <QDialog>
#include <QObject>
#include <QString>

#include "ui_AmplifyFreeDlg.h"

namespace Kwave
{
    class AmplifyFreeDialog: public QDialog, public Ui::AmplifyFreeDlg
    {
        Q_OBJECT
    public:

        /** Constructor */
        explicit AmplifyFreeDialog(QWidget *parent);

        /** Destructor */
        virtual ~AmplifyFreeDialog();

        /** Returns a command string for the curve */
        QString getCommand();

        /** Sets the curve parameters and points from a list of parameters */
        void setParams(QStringList &params);

    private slots:

        /** invoke the online help */
        void invokeHelp();

    };
}

#endif /* AMPLIFY_FREE_DIALOG_H */

//***************************************************************************
//***************************************************************************
