/***************************************************************************
      UndoTransaction.h  -  groups moulitple UndoAction objects together
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

#include "UndoAction.h"
#include "UndoTransaction.h"

//***************************************************************************
UndoTransaction::UndoTransaction(const QString &name)
    :QList<UndoAction>(), m_description(name)
{
}

//***************************************************************************
unsigned int UndoTransaction::undoSize()
{
    unsigned int s = 0;
    QListIterator<UndoAction> it(*this);
    for ( ; it.current(); ++it ) {
	s += it.current()->undoSize();
    }
    return s;
}

//***************************************************************************
unsigned int UndoTransaction::redoSize()
{
    long int current = 0;
    long int max = 0;

    QListIterator<UndoAction> it(*this);
    for (it.toLast() ; it.current(); --it ) {
	int s = it.current()->redoSize();
	current += s;
	if (current > max) max = current;
    }
    return max;
}

//***************************************************************************
QString UndoTransaction::description()
{
    // if description exists, return it
    if (m_description.length()) return m_description;

    QString str("");
    QListIterator<UndoAction> it(*this);
    for ( ; it.current(); ++it ) {
	QString d = it.current()->description();
	// skip duplicates
	if (str.contains(d+", ") || (str == d)) continue;
	
	// append others
	if (str.length()) str += ", ";
	str += d;
    }
    return str;
}

//***************************************************************************
//***************************************************************************
