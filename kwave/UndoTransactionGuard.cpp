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
#include "SignalManager.h"
#include "UndoTransactionGuard.h"

//***************************************************************************
UndoTransactionGuard::UndoTransactionGuard(SignalManager &manager,
                                           const QString &name)
    :TSS_Object(), m_manager(manager)
{
    m_manager.startUndoTransaction(name);
}

//***************************************************************************
UndoTransactionGuard::~UndoTransactionGuard()
{
    m_manager.closeUndoTransaction();
}

//***************************************************************************
//***************************************************************************
