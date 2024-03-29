/***************************************************************************
 UndoTransactionGuard.cpp  -  guard class for undo transactions
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

#include "config.h"

#include <QString>

#include <KLocalizedString>

#include "libkwave/Plugin.h"
#include "libkwave/PluginManager.h"
#include "libkwave/SignalManager.h"
#include "libkwave/String.h"
#include "libkwave/undo/UndoTransactionGuard.h"

//***************************************************************************
Kwave::UndoTransactionGuard::UndoTransactionGuard(Kwave::SignalManager &manager,
                                                  const QString &name)
    :m_manager(manager),
     m_initial_modified(m_manager.isModified())
{
    m_manager.startUndoTransaction(name);
}

//***************************************************************************
Kwave::UndoTransactionGuard::UndoTransactionGuard(Kwave::Plugin &plugin,
                                                  const QString &name)
    :m_manager(plugin.manager().signalManager()),
     m_initial_modified(m_manager.isModified())
{
    QString description = (name.length()) ?
        name : i18n(UTF8(plugin.name()));
    m_manager.startUndoTransaction(description);
}

//***************************************************************************
Kwave::UndoTransactionGuard::~UndoTransactionGuard()
{
    m_manager.closeUndoTransaction();
}

//***************************************************************************
bool Kwave::UndoTransactionGuard::registerUndoAction(UndoAction *action)
{
    return m_manager.registerUndoAction(action);
}

//***************************************************************************
void Kwave::UndoTransactionGuard::abort()
{
    m_manager.abortUndoTransaction();
    if (m_manager.isModified() != m_initial_modified)
        m_manager.setModified(m_initial_modified);
}

//***************************************************************************
//***************************************************************************
