/***************************************************************************
    StringEnterDialog.h  -  dialog for entering a string command
                             -------------------
    begin                : Sat Mar 14 2015
    copyright            : (C) 2015 by Thomas Eschenbacher
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

#ifndef STRING_ENTER_DIALOG_H
#define STRING_ENTER_DIALOG_H

#include "config.h"

#include <QDialog>
#include <QString>

#include "ui_StringEnterDlg.h"

class QWidget;

namespace Kwave
{
    class StringEnterDialog: public QDialog,
                             public Ui::StringEnterDlg
    {
	Q_OBJECT
    public:

	/**
	 * Constructor.
	 * @param parent the parent widget the dialog belongs to
	 * @param preset initial text of the line edit field
	 */
	explicit StringEnterDialog(QWidget *parent, const QString &preset);

	/** Destructor */
	virtual ~StringEnterDialog();

	/** Returns the string that has been entered */
	QString command();

    private slots:

	/** called when the dialog has been accepted (OK pressed) */
	virtual void accept();

	/** invoke the online help */
	void invokeHelp();

    private:

	/** the command that has been entered */
	QString m_command;

    };
}

#endif /* STRING_ENTER_DIALOG_H */

//***************************************************************************
//***************************************************************************
