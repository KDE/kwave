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

#include <QListIterator>

#include "UndoAction.h"
#include "UndoTransaction.h"

//***************************************************************************
UndoTransaction::UndoTransaction(const QString &name)
    :QList<UndoAction *>(), m_description(name)
{
}

//***************************************************************************
UndoTransaction::~UndoTransaction()
{
    while (!isEmpty()) {
	UndoAction *action = takeLast();
	if (action) delete action;
    }
}

//***************************************************************************
unsigned int UndoTransaction::undoSize()
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
unsigned int UndoTransaction::redoSize()
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
QString UndoTransaction::description()
{
    // if description exists, return it
    if (m_description.length()) return m_description;

    QString str("");
    QListIterator<UndoAction *> it(*this);
    while (it.hasNext()) {
	UndoAction *undo = it.next();
	if (!undo) continue;
	QString d = undo->description();

	// skip duplicates
	if (str.contains(", "+d) || (str == d)) continue;

	// append others
	if (str.length()) str += ", ";
	str += d;
    }
    return str;
}

//***************************************************************************
UndoAction *UndoTransaction::nextUndo()
{
    UndoAction *next = last();

    QListIterator<UndoAction *> it(*this);
    it.toBack();
    while (it.hasPrevious()) {
	UndoAction *action = it.previous();
	Q_ASSERT(action);
	if (!action) continue;
	Q_ASSERT(next);
	if (action->group() < next->group())
	    next = action;
    }
    return next;
}

//***************************************************************************
UndoAction *UndoTransaction::nextRedo()
{
    UndoAction *next = first();

    QListIterator<UndoAction *> it(*this);
    while (it.hasNext()) {
	UndoAction *action = it.next();
	Q_ASSERT(action);
	if (!action) continue;
	Q_ASSERT(next);
	if (action->group() > next->group())
	    next = action;
    }
    return next;
}

//***************************************************************************
//***************************************************************************
