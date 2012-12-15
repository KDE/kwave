/***************************************************************************
    UndoTransaction.cpp  -  groups mulitple UndoAction objects together
			     -------------------
    begin                : Fri May 25 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
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

#include <QtCore/QListIterator>

#include "libkwave/String.h"
#include "libkwave/undo/UndoAction.h"
#include "libkwave/undo/UndoTransaction.h"

//***************************************************************************
Kwave::UndoTransaction::UndoTransaction(const QString &name)
    :QList<UndoAction *>(), m_description(name), m_aborted(false)
{
}

//***************************************************************************
Kwave::UndoTransaction::~UndoTransaction()
{
    while (!isEmpty()) {
	UndoAction *action = takeLast();
	if (action) delete action;
    }
}

//***************************************************************************
unsigned int Kwave::UndoTransaction::undoSize()
{
    unsigned int s = 0;
    QListIterator<UndoAction *> it(*this);
    while (it.hasNext()) {
	UndoAction *undo = it.next();
	if (undo) s += undo->undoSize();
    }
    return s;
}

//***************************************************************************
unsigned int Kwave::UndoTransaction::redoSize()
{
    unsigned int s = 0;
    QListIterator<UndoAction *> it(*this);
    while (it.hasNext()) {
	UndoAction *undo = it.next();
	if (undo) s += undo->redoSize();
    }
    return s;
}

//***************************************************************************
QString Kwave::UndoTransaction::description()
{
    // if description exists, return it
    if (m_description.length()) return m_description;

    QString str;
    QListIterator<UndoAction *> it(*this);
    while (it.hasNext()) {
	UndoAction *undo = it.next();
	if (!undo) continue;
	QString d = undo->description();

	// skip duplicates
	if (str.contains(_(", ") + d) || (str == d)) continue;

	// append others
	if (str.length()) str += _(", ");
	str += d;
    }
    return str;
}

//***************************************************************************
bool Kwave::UndoTransaction::containsModification() const
{
    if (isEmpty()) return false;
    QListIterator<UndoAction *> it(*this);
    while (it.hasNext()) {
	UndoAction *action = it.next();
	if (!action) continue;
	if (action->containsModification()) return true;
    }
    return false;
}

//***************************************************************************
void Kwave::UndoTransaction::abort()
{
    m_aborted = true;
}

//***************************************************************************
void Kwave::UndoTransaction::dump(const QString &indent)
{
    qDebug("%s [%s]", indent.toLocal8Bit().data(),
	   description().toLocal8Bit().data());
    if (isEmpty()) return;

    QListIterator<UndoAction *> it(*this);
    it.toBack();
    while (it.hasPrevious()) {
	UndoAction *action = it.previous();
	Q_ASSERT(action);
	if (!action) continue;
	action->dump(_("    "));
    }
}

//***************************************************************************
//***************************************************************************
