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

#ifndef UNDO_TRANSACTION_GUARD_H
#define UNDO_TRANSACTION_GUARD_H

#include "config.h"
#include "libkwave_export.h"

#include <QtGlobal>

class QString;

namespace Kwave {

    class SignalManager;
    class UndoAction;
    class Plugin;

    /**
     * @class UndoTransactionGuard
     * A simple guard class for opening and closing an undo transaction
     * operating on a SignalManager. Several nested UndoTransactionGuards
     * (or undo transactions) are allowed.
     */
    class LIBKWAVE_EXPORT UndoTransactionGuard
    {

    public:
        /**
         * Constructor. Also determines the name of the transaction if
         * it is the first of several nested transactions.
         * @param manager reference to the SignalManager we operate on
         * @param name the name of the transaction as a user-readable and
         *        localized string. [optional]
         */
        explicit UndoTransactionGuard(Kwave::SignalManager &manager,
                                      const QString &name = QString());

        /**
         * Constructor for use from a plugin. Also determines the name of the
         * transaction if it is the first of several nested transactions.
         * @param plugin reference to the plugin (you should pass
         *               <code>*this</code>.
         * @param name the name of the transaction as a user-readable and
         *        localized string. [optional] If you pass null or omit this
         *        parameter, the name of the plugin will be used instead.
         */
        explicit UndoTransactionGuard(Kwave::Plugin &plugin,
                                      const QString &name = QString());

        /** Destructor. */
        virtual ~UndoTransactionGuard();

        /**
         * Tries to free memory for a new undo action and stores all needed
         * data if successful.
         * @param action UndoAction to that is to be registered
         * @return true if the action is allowed, false if the user has
         *         chosen to abort the operation if the memory limit of
         *         the undo buffer would be exceeded. The return value
         *         will also be false if the action is null.
         * @note If undo is currently not enabled, the passed UndoAction
         *       will be ignored and not freed, the return value will
         *       be false. So it is safer not to call this function if
         *       undo is not enabled.
         */
        bool registerUndoAction(UndoAction *action);

        /**
         * Aborts the undo transaction, discards all undo data and restores
         * the previous "modified" state of the signal.
         */
        void abort();

    private:

        /** Reference to the responsible SignalManager */
        Kwave::SignalManager &m_manager;

        /** the initial "modified" state of the signal */
        bool m_initial_modified;

    };

}

#endif /* UNDO_TRANSACTION_GUARD_H */

//***************************************************************************
//***************************************************************************
