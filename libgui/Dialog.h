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

class KTMainWindow;

/**
 * Base class for all plugins that provide a dialog.
 * @deprecated, use the "KwavePlugin" class instead !
 */
class Dialog : public QDialog
{
    Q_OBJECT
public:

    /**
     * Constructor.
     * @param modal can be set to true in order to make the
     *              dialog modal. Note that this is only
     *              provided for compatibility and should not be
     *              used any more !
     */
    Dialog(bool modal = false);

    /**
     * Constructor.
     * @param name title of the dialog
     * @param modal can be set to true in order to make the
     *              dialog modal. Note that this is only
     *              provided for compatibility and should not be
     *              used any more !
     */
    Dialog(const char *name, bool modal = false);

    /**
     * Destructor. Also detaches the window from it's
     * parent (if this exists).
     */
    ~Dialog();

public slots:

    void accept();
    void reject();

    virtual const char *getCommand() = 0;

signals:

    void command(const char *command);

private:
    bool modal;

    /**
     * parent window or null, normally a kwave TopWidget
     * @see TopWidget
     */
    KTMainWindow *parentWindow;
};

#endif // _DIALOG_H_
