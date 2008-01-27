/***************************************************************************
   UndoInsertAction.cpp  -  UndoAction for insertion of a range of samples
			     -------------------
    begin                : Jun 14 2001
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
#include <klocale.h>

#include "SignalManager.h"
#include "UndoAction.h"
#include "UndoDeleteAction.h"
#include "UndoInsertAction.h"

//***************************************************************************
UndoInsertAction::UndoInsertAction(unsigned int track,
                                   unsigned int offset, unsigned int length)
    :QObject(), UndoAction(), m_track(track),
     m_offset(offset), m_length(length)
{
}

//***************************************************************************
QString UndoInsertAction::description()
{
    return i18n("insert");
}

//***************************************************************************
unsigned int UndoInsertAction::undoSize()
{
    return 0;
}

//***************************************************************************
int UndoInsertAction::redoSize()
{
    return sizeof(UndoDeleteAction) + (m_length * sizeof(sample_t));
}

//***************************************************************************
void UndoInsertAction::store(SignalManager &)
{
    // we delete -> nothing to store
}

//***************************************************************************
UndoAction *UndoInsertAction::undo(SignalManager &manager, bool with_redo)
{
    UndoAction *redo_action = 0;

    // store data for redo
    if (with_redo) {
	redo_action = new UndoDeleteAction(m_track, m_offset, m_length);
	Q_ASSERT(redo_action);
	if (!redo_action) return 0;
	redo_action->store(manager);
    }

    // now delete the samples
    manager.deleteRange(m_track, m_offset, m_length);

    return redo_action;
}

//***************************************************************************
void UndoInsertAction::setLength(unsigned int length)
{
    m_length = length;
}

//***************************************************************************
#include "UndoInsertAction.moc"
//***************************************************************************
//***************************************************************************
