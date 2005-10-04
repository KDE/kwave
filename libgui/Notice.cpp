/*************************************************************************
        Notice.cpp  -  replacement for KMessageBox, with auto-close
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

#include "config.h"

#include <qmessagebox.h>
#include <qstringlist.h>
#include <qtimer.h>

#include <klocale.h>
#include <kmessagebox.h>

#include "Notice.h"

//***************************************************************************
Notice::Notice(QWidget *parent, const QString &message)
    :QObject(parent), m_seconds_left(3), m_dialog(0)
{
    m_dialog= new KDialogBase(
	i18n("Sorry"),
	KDialogBase::Yes,
	KDialogBase::Yes,
	KDialogBase::Yes,
	parent, "sorry",
	true,
	true,
	KStdGuiItem::ok()
    );

    // do one manual tick to update the text of the button
    tick();
    m_seconds_left++;

    // start a timer for ticks every second
    QTimer *timer = new QTimer(m_dialog, 0);
    QObject::connect(timer, SIGNAL(timeout()),
                     this, SLOT(tick()));
    timer->start(1000, false);

    // activate the message box
    KMessageBox::createKMessageBox(
	m_dialog, QMessageBox::Warning, message, QStringList(),
	QString::null, 0, 0);
}

//***************************************************************************
Notice::~Notice()
{
}

//***************************************************************************
void Notice::tick()
{
    m_seconds_left--;

    if (m_seconds_left < 0) {
	m_dialog->close();
	return;
    }

    m_dialog->setButtonText(KDialogBase::Yes,
	i18n("&Continue (in %1 seconds...)").arg(m_seconds_left));
}

//***************************************************************************
//***************************************************************************
