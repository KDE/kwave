/*************************************************************************
       RecordDialog.cpp  -  dialog window for controlling audio recording
                             -------------------
    begin                : Wed Aug 20 2003
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

#include "RecordDialog.h"

//***************************************************************************
RecordDialog::RecordDialog(QWidget *parent, QStringList &params)
    :RecordDlg(parent,0), m_params(params)
{
}

//***************************************************************************
RecordDialog::~RecordDialog()
{
}

QStringList RecordDialog::params()
{
    return m_params;
}

//***************************************************************************
//***************************************************************************

