/*************************************************************************
 ConfirmCancelProxy.cpp  -  ask for confirm before cancelling an action
                             -------------------
    begin                : Fri Apr 26 2002
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
#include <qobject.h>
#include <qstring.h>
#include <qwidget.h>

#include <klocale.h>
#include <kmessagebox.h>

#include "ConfirmCancelProxy.h"

//***************************************************************************
ConfirmCancelProxy::ConfirmCancelProxy(QWidget *widget,
    QObject *sender, const char *signal,
    QObject *receiver, const char *slot)
    :QObject(), m_widget(widget)
{
    Q_ASSERT(receiver);
    if (!receiver) return;

    if (sender) QObject::connect(sender, signal, this, SLOT(cancel()));
    QObject::connect(this, SIGNAL(cancelled()), receiver, slot);
}

//***************************************************************************
ConfirmCancelProxy::~ConfirmCancelProxy()
{
}

//***************************************************************************
void ConfirmCancelProxy::cancel()
{
    if (KMessageBox::questionYesNo(m_widget,
        i18n("Do you really want to abort the current action?"))
        != KMessageBox::Yes) return; // no cancel
    emit cancelled();
}

//***************************************************************************
#include "ConfirmCancelProxy.moc"
//***************************************************************************
//***************************************************************************
