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

#include <klocale.h>

#include "libkwave/Label.h"
#include "SignalWidget.h"
#include "UndoAddLabelAction.h"
#include "UndoDeleteLabelAction.h"

//***************************************************************************
UndoAddLabelAction::UndoAddLabelAction(SignalWidget &signal_widget,
                                       int index)
    :UndoAction(), m_signal_widget(signal_widget), m_index(index)
{
    qDebug("UndoAddLabelAction::UndoAddLabelAction(%d)", index);
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
void UndoAddLabelAction::store(SignalManager &)
{
    // nothing to do, all data has already
    // been stored in the constructor
}

//***************************************************************************
UndoAction *UndoAddLabelAction::undo(SignalManager &manager, bool with_redo)
{
    UndoAction *redo = 0;
    Label *label = m_signal_widget.labelAtIndex(m_index);
    qDebug("label=%p, m_index=%d",label,m_index);

    // store data for redo
    if (with_redo && label) {
	redo = new UndoDeleteLabelAction(m_signal_widget, *label);
	Q_ASSERT(redo);
	if (redo) redo->store(manager);
    }

    // remove the label from the signal manager
    m_signal_widget.deleteLabel(m_index, false);

    return redo;
}

//***************************************************************************
//***************************************************************************
