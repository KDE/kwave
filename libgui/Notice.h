/*************************************************************************
          Notice.h  -  replacement for KMessageBox, with auto-close
                             -------------------
    begin                : Tue Oct 04 2005
    copyright            : (C) 2005 by Thomas Eschenbacher
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

#ifndef _NOTICE_H_
#define _NOTICE_H_

#include "config.h"

#include <qobject.h>
#include <qstring.h>
#include <qwidget.h>
#include <kdialogbase.h>

class Notice: public QObject
{
Q_OBJECT
public:
    /**
     * Constructor
     *
     * @param parent the parent widget
     * @param message text to be shown in the message box
     */
    Notice(QWidget *parent, const QString &message);

    /** destructor */
    virtual ~Notice();

private slots:

    /** update the "OK" button and count down */
    void tick();

private:
    /** number of seconds to auto-close */
    int m_seconds_left;

    /** reference to the dialog, needed for changing the button */
    KDialogBase *m_dialog;
};

#endif /* _NOTICE_H_ */
