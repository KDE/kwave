/***************************************************************************
   SelectDateDialog.cpp  -  dialog for selecting a date
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

#include "config.h"

#include <kdatepicker.h>

#include "SelectDateDialog.h"

//***************************************************************************
SelectDateDialog::SelectDateDialog(QWidget *parent, QDate &date)
    :QDialog(parent), Ui::SelectDateDlg(), m_date(date)
{
    setupUi(this);
    datePicker->setDate(date);
}

//***************************************************************************
SelectDateDialog::~SelectDateDialog()
{
}

//***************************************************************************
QDate SelectDateDialog::date()
{
    return m_date;
}

//***************************************************************************
void SelectDateDialog::accept()
{
    m_date = datePicker->date();
    QDialog::accept();
}

//***************************************************************************
#include "SelectDateDialog.moc"
//***************************************************************************
//***************************************************************************
