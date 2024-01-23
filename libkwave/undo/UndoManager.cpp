/***************************************************************************
         UndoManager.cpp -  manager class for undo/redo handling
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

#include "config.h"

#include "libkwave/undo/UndoHandler.h"
#include "libkwave/undo/UndoManager.h"

//***************************************************************************
Kwave::UndoManager::UndoManager()
    :m_handlers()
{
}

//***************************************************************************
Kwave::UndoManager::~UndoManager()
{
    m_handlers.clear();
}

//***************************************************************************
bool Kwave::UndoManager::registerHandler(Kwave::UndoHandler *handler)
{
    if (!handler) return false;
    if (m_handlers.contains(handler)) return false;
    m_handlers.append(handler);
    return true;
}

//***************************************************************************
bool Kwave::UndoManager::unregisterHandler(Kwave::UndoHandler *handler)
{
    if (!handler) return false;
    if (!m_handlers.contains(handler)) return false;
    m_handlers.removeAll(handler);
    return true;
}

//***************************************************************************
bool Kwave::UndoManager::startUndoTransaction(
    Kwave::UndoTransaction *transaction)
{
    if (!transaction) return false;

    foreach (Kwave::UndoHandler *handler, m_handlers) {
        Q_ASSERT(handler);
        if (!handler) continue;
        if (!(handler->saveUndoData(*transaction)))
            return false;
    }

    return true;
}

//***************************************************************************
bool Kwave::UndoManager::closeUndoTransaction(
    Kwave::UndoTransaction *transaction)
{
    Q_UNUSED(transaction)
    return true;
}

//***************************************************************************
//***************************************************************************
