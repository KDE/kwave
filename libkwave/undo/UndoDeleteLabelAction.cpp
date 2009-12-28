/***************************************************************************
 UndoAddLabelAction.cpp  -  Undo action for deleting labels
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
UndoDeleteLabelAction::UndoDeleteLabelAction(Label &label)
    :UndoAction(), m_label(label)
{
}

//***************************************************************************
UndoDeleteLabelAction::~UndoDeleteLabelAction()
{
    m_label = Label();
}

//***************************************************************************
QString UndoDeleteLabelAction::description()
{
    return i18n("Delete Label");
}

//***************************************************************************
unsigned int UndoDeleteLabelAction::undoSize()
{
    return sizeof(*this) + sizeof(Label);
}

//***************************************************************************
int UndoDeleteLabelAction::redoSize()
{
    return sizeof(UndoAddLabelAction) + sizeof(Label);
}

//***************************************************************************
bool UndoDeleteLabelAction::store(SignalManager &)
{
    // nothing to do, all data has already
    // been stored in the constructor
    return true;
}

//***************************************************************************
UndoAction *UndoDeleteLabelAction::undo(SignalManager &manager,
                                        bool with_redo)
{
    Q_ASSERT(!m_label.isNull());
    if (m_label.isNull()) return 0;

    UndoAction *redo = 0;

    // add a new label to the signal manager
    Label label = manager.addLabel(m_label.pos(), m_label.name());

    // store data for redo
    if (with_redo) {
	int index = manager.labelIndex(label);
	redo = new UndoAddLabelAction(index);
	Q_ASSERT(redo);
	if (redo) redo->store(manager);
    }

    return redo;
}

//***************************************************************************
void UndoDeleteLabelAction::dump(const QString &indent)
{
    qDebug("%sundo delete label, %u, '%s'",
           indent.toLocal8Bit().data(),
           m_label.pos(),
           m_label.name().toLocal8Bit().data()
          );
}

//***************************************************************************
//***************************************************************************
