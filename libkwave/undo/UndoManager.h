/***************************************************************************
           UndoManager.h -  manager class for undo/redo handling
                             -------------------
    begin                : Sat Feb 01 2014
    copyright            : (C) 2014 by Thomas Eschenbacher
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

#ifndef UNDO_MANAGER_H
#define UNDO_MANAGER_H

#include "config.h"
#include "libkwave_export.h"

#include <QtGlobal>
#include <QList>

namespace Kwave
{

    class UndoHandler;
    class UndoTransaction;

    class LIBKWAVE_EXPORT UndoManager
    {
    public:

        /**
         * Constructor
         */
        UndoManager();

        /**
         * Destructor
         */
        virtual ~UndoManager();

        /**
         * Register an instance of an undo handler
         *
         * @param handler the undo handler to register
         * @return true if successful, false if failed
         *         (e.g. duplicate registration or null pointer)
         */
        bool registerHandler(Kwave::UndoHandler *handler);

        /**
         * Unregister an instance of an undo handler previously
         * registered through registerHandler()
         *
         * @param handler the undo handler to unregister
         * @return true if successful, false if failed
         *         (e.g. if not registered or null pointer)
         */
        bool unregisterHandler(Kwave::UndoHandler *handler);

        /**
         * Should be called when a undo transaction has been started,
         * calls saveUndoData of all undo handlers
         *
         * @param transaction the newly created undo transaction (still empty)
         * @return true if successful, false if failed (e.g. one undo handler
         *         failed saving it's data)
         */
        bool startUndoTransaction(Kwave::UndoTransaction *transaction);

        /**
         * Should be called when a undo transaction is completed
         *
         * @param transaction the undo transaction that has been closed
         * @return true if successful, false if failed
         */
        bool closeUndoTransaction(Kwave::UndoTransaction *transaction);

    private:

        /** list of all registered undo handlers */
        QList<Kwave::UndoHandler *> m_handlers;

    };
}

#endif /* UNDO_MANAGER_H */

//***************************************************************************
//***************************************************************************
