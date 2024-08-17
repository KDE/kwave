/***************************************************************************
     SelectDateDialog.h  -  dialog for selecting a date
                             -------------------
    begin                : Tue Jul 30 2002
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

#ifndef SELECT_DATE_DIALOG_H
#define SELECT_DATE_DIALOG_H

#include "config.h"

#include <QDateTime>
#include <QObject>

#include "ui_SelectDateDlg.h"

class QWidget;

namespace Kwave
{

    class SelectDateDialog: public QDialog,
                            public Ui::SelectDateDlg
    {
        Q_OBJECT
    public:
        /** Constructor */
        SelectDateDialog(QWidget *parent, QDate &date);

        /** Destructor */
        ~SelectDateDialog() override;

        /** Returns the selected date */
        virtual QDate date();

    public slots:

        /** applies the settings and closes the dialog (OK button) */
        void accept() override;

    private:

        /** stores the last known date */
        QDate m_date;

    };
}

#endif /* SELECT_DATE_DIALOG_H */

//***************************************************************************
//***************************************************************************
