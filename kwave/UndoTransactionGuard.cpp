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

#include <qstring.h>

#include "libkwave/KwavePlugin.h"

#include "PluginManager.h"
#include "SignalManager.h"
#include "TopWidget.h"
#include "UndoTransactionGuard.h"

//***************************************************************************
UndoTransactionGuard::UndoTransactionGuard(SignalManager &manager,
                                           const QString &name)
    :m_manager(manager)
{
    m_manager.startUndoTransaction(name);
}

//***************************************************************************
UndoTransactionGuard::UndoTransactionGuard(KwavePlugin &plugin,
                                           const QString &name)
    :m_manager(plugin.manager().topWidget().signalManager())
{
    QString description = (name.length()) ? name : i18n(plugin.name());
    m_manager.startUndoTransaction(description);
}

//***************************************************************************
UndoTransactionGuard::~UndoTransactionGuard()
{
    m_manager.closeUndoTransaction();
}

//***************************************************************************
//***************************************************************************
