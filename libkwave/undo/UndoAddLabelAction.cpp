/***************************************************************************
 UndoAddLabelAction.cpp  -  Undo action for insertion of labels
			     -------------------
    begin                : Wed Aug 16 2006
    copyright            : (C) 2006 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de>

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
#include <klocale.h>

#include "libkwave/Label.h"
#include "libkwave/SignalManager.h"
#include "libkwave/undo/UndoAddLabelAction.h"
#include "libkwave/undo/UndoDeleteLabelAction.h"

//***************************************************************************
UndoAddLabelAction::UndoAddLabelAction(int index)
    :UndoAction(), m_index(index)
{
}

//***************************************************************************
UndoAddLabelAction::~UndoAddLabelAction()
{
}

//***************************************************************************
QString UndoAddLabelAction::description()
{
    return i18n("add label");
}

//***************************************************************************
unsigned int UndoAddLabelAction::undoSize()
{
    return sizeof(*this);
}

//***************************************************************************
int UndoAddLabelAction::redoSize()
{
    return sizeof(Label);
}

//***************************************************************************
bool UndoAddLabelAction::store(SignalManager &)
{
    // nothing to do, all data has already
    // been stored in the constructor
    return true;
}

//***************************************************************************
UndoAction *UndoAddLabelAction::undo(SignalManager &manager, bool with_redo)
{
    UndoAction *redo = 0;
    Label *label = manager.labelAtIndex(m_index);

    // store data for redo
    if (with_redo && label) {
	redo = new UndoDeleteLabelAction(*label);
	Q_ASSERT(redo);
	if (redo) redo->store(manager);
    }

    // remove the label from the signal manager
    manager.deleteLabel(m_index, false);

    return redo;
}

//***************************************************************************
//***************************************************************************
