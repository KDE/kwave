/***************************************************************************
      UndoTransaction.h  -  groups moulitple UndoAction objects together
			     -------------------
    begin                : May 25 2001
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
unsigned int UndoTransaction::size()
{
    unsigned int s = 0;
    QListIterator<UndoAction> it(*this);
    for ( ; it.current(); ++it ) {
	s += it.current()->size();
    }
    return s;
}

//***************************************************************************
QString UndoTransaction::description()
{
    QString str("");
    QListIterator<UndoAction> it(*this);
    for ( ; it.current(); ++it ) {
	str += it.current()->description();
	if (it == last()) str += ", ";
    }
    return str;
}

//***************************************************************************
//***************************************************************************
