/***************************************************************************
 UndoAddLabelAction.cpp  -  Undo action for deleting labels
			     -------------------
    begin                : Sun Sep 03 2006
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
#include "libkwave/undo/UndoModifyLabelAction.h"

#include "libgui/SignalWidget.h"

//***************************************************************************
UndoModifyLabelAction::UndoModifyLabelAction(const Label &label)
    :UndoAction(), m_label(), m_last_position(0)
{
    m_label = label;
}

//***************************************************************************
UndoModifyLabelAction::~UndoModifyLabelAction()
{
    m_label = Label();
}

//***************************************************************************
void UndoModifyLabelAction::setLastPosition(unsigned int pos)
{
    m_last_position = pos;
}

//***************************************************************************
QString UndoModifyLabelAction::description()
{
    return i18n("modify label");
}

//***************************************************************************
unsigned int UndoModifyLabelAction::undoSize()
{
    return sizeof(*this) + sizeof(Label);
}

//***************************************************************************
int UndoModifyLabelAction::redoSize()
{
    return undoSize();
}

//***************************************************************************
bool UndoModifyLabelAction::store(SignalManager &)
{
    // nothing to do, all data has already
    // been stored in the constructor
    return true;
}

//***************************************************************************
UndoAction *UndoModifyLabelAction::undo(SignalManager &manager,
                                        bool with_redo)
{
    Q_ASSERT(!(m_label.isNull()));
    if (m_label.isNull()) return 0;

//     qDebug("undo: last pos=%u, current pos=%u",
// 	   m_last_position, m_label->pos());

    const Label label = manager.findLabel(m_last_position);
    Q_ASSERT(!label.isNull());
    if (label.isNull()) return 0;

    // store data for redo
    UndoModifyLabelAction *redo = 0;
    if (with_redo) {
	redo = new UndoModifyLabelAction(label);
	Q_ASSERT(redo);
	if (redo) {
	    redo->setLastPosition(m_label.pos());
	    redo->store(manager);
	}
    }

    // modify the label
    int index = manager.labelIndex(label);
    manager.modifyLabel(index, m_label.pos(), m_label.name());

    return redo;
}

//***************************************************************************
//***************************************************************************
