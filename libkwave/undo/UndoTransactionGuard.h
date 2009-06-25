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

#include "config.h"

#include <kdemacros.h>

class QString;
class SignalManager;
class UndoAction;

namespace Kwave { class Plugin; }

/**
 * @class UndoTransactionGuard
 * A simple guard class for opening and closing an undo transaction
 * operating on a SignalManager. Several nested UndoTransactionGuards
 * (or undo transactions) are allowed.
 */
class KDE_EXPORT UndoTransactionGuard
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

    /**
     * Constructor for use from a plugin. Also determines the name of the
     * transaction if it is the first of several nested transactions.
     * @param plugin reference to the plugin (you should pass
     *               <code>*this</code>.
     * @param name the name of the transaction as a user-readable and
     *        localized string. [optional] If you pass null or omit this
     *        parameter, the name of the plugin will be used instead.
     */
    UndoTransactionGuard(Kwave::Plugin &plugin, const QString &name = 0);

    /** Destructor. */
    virtual ~UndoTransactionGuard();

    /**
     * Tries to free memory for a new undo action and stores all needed
     * data if successful.
     * @param action UndoAction to that is to be registered
     * @return true if the action is allowed, false if the user has
     *         choosen to abort the operation if the memory limit of
     *         the undo buffer would be exceeded. The return value
     *         will also be false if the action is null.
     * @note If undo is currently not enabled, the passed UndoAction
     *       will be ignored and not freed, the return value will
     *       be false. So it is safer not to call this function if
     *       undo is not enabled.
     */
    bool registerUndoAction(UndoAction *action);

private:
    /** Reference to the responsible SignalManager */
    SignalManager &m_manager;

};

#endif /* _UNDO_TRANSACTION_GUARD_H_ */
