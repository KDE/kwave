/***************************************************************************
			  Dialog.h  -  root class for dialog plugins
			     -------------------
    begin                : Thu May  04 2000
    copyright            : (C) 2000 by Martin Wilz
    email                : Martin Wilz <mwilz@ernie.mi.uni-koeln.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DIALOG_H_
#define _DIALOG_H_ 1

#include <qdialog.h>

#define OK     i18n("&Ok")
#define CANCEL i18n("&Cancel")

/**
 * Base class for all plugins that provide a dialog.
 */
class Dialog : public QDialog
{
    Q_OBJECT
public:

    Dialog(bool modal = false);
    Dialog(const char *name, bool modal = false);
    ~Dialog();

public slots:

    void accept();
    void reject();

    virtual const char *getCommand() = 0;

signals:

    void command(const char *command);

private:
    bool modal;
};

#endif // _DIALOG_H_
