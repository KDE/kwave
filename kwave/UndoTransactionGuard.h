/***************************************************************************
 UndoTransactionGuard.h  -  guard class for undo transactions
			     -------------------
    begin                : Sat May 26, 2001
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

#ifndef _UNDO_TRANSACTION_GUARD_H_
#define _UNDO_TRANSACTION_GUARD_H_

#include "mt/TSS_Object.h"

class QString;
class SignalManager;

/**
 * @class UndoTransactionGuard
 * A simple guard class for opening and closing an undo transaction
 * operating on a SignalManager. Several nested UndoTransactionGuards
 * (or undo transactions) are allowed.
 */
class UndoTransactionGuard: public TSS_Object
{

public:
    /**
     * Constructor. Also determines the name of the transaction if
     * it is the first of several nested transactions.
     * @param manager reference to the SignalManager we operate on
     * @param name the name of the transaction as a user-readable and
     *        localized string. [optional]
     */
    UndoTransactionGuard(SignalManager &manager, const QString &name = 0);

    /** Destructor. */
    virtual ~UndoTransactionGuard();

private:
    /** Reference to the responsible SignalManager */
    SignalManager &m_manager;

};

#endif /* _UNDO_TRANSACTION_GUARD_H_ */
