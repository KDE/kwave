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

#ifndef _UNDO_TRANSACTION_H_
#define _UNDO_TRANSACTION_H_

#include <qlist.h>
#include <qstring.h>

class UndoAction;

/**
 * @class UndoTransaction
 * Groups multiple UndoAction objects together to one transaction. As most
 * user actions consist of a number of small actions that belong together
 * and don't make sense or leave an inconsistent state if separated, they
 * get grouped together to one transaction.
 */
class UndoTransaction: public QList<UndoAction>
{

public:

    /** Returns the size in bytes summed up over all undo actions */
    unsigned int size();

    /**
     * Returns the list of descriptions
     * @todo avoid duplicates, give a useful name/description
     */
    QString description();

};

#endif /* _UNDO_TRANSACTION_H_ */
