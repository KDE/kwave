/***************************************************************************
              AboutDialog.h  -  dialog for Kwave's "Help-About"
                             -------------------
    begin                : Sun Oct 29 2000
    copyright            : (C) 2000 by Thomas Eschenbacher
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

#ifndef _ABOUT_DIALOG_H_
#define _ABOUT_DIALOG_H_

#include <qdialog.h>

class LogoWidget;
class QMultiLineEdit;
class QPushButton;
class QResizeEvent;

//**********************************************************
/**
 * @class AboutDialog
 * Dialog for Help/About
 * @todo use AboutData from the KwaveApp
 */
class AboutDialog : public QDialog {
    Q_OBJECT

public:

    /** Constructor */
    AboutDialog(QWidget *parent);

    /** destructor */
    virtual ~AboutDialog();

private:
    /** the text area on the right side */
    QMultiLineEdit *m_abouttext;

    /** the animated logo on the left side */
    LogoWidget *m_logo;

    /** the OK button at the buttom */
    QPushButton *m_ok;
};

#endif  /* _ABOUT_DIALOG_H_ */
