/***************************************************************************
       FileInfoDialog.h  -  dialog for editing file properties
                             -------------------
    begin                : Sat Jul 20 2002
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

#ifndef _FILE_INFO_DIALOG_H_
#define _FILE_INFO_DIALOG_H_

#include "config.h"
#include <qobject.h>
#include "libkwave/FileInfo.h"
#include "FileInfoDlg.uih.h"

class FileInfoDialog: public FileInfoDlg
{
    Q_OBJECT
public:
    /** Constructor */
    FileInfoDialog(QWidget *parent, FileInfo &info);

    /** Destructor */
    virtual ~FileInfoDialog();

    /** Returns the current file info */
    FileInfo &info() { return m_info; };

public slots:

    /** applies the settings and closes the dialog (OK button) */
    virtual void accept();

private slots:

    /** updates the verbose description of the number of tracks */
    void tracksChanged(int tracks);

private:

    /** FileInfo to be edited */
    FileInfo m_info;
};

#endif /* _FILE_INFO_DIALOG_H_ */
