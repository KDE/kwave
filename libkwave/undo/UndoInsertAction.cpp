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

#include "libkwave/SignalManager.h"
#include "libkwave/undo/UndoAction.h"
#include "libkwave/undo/UndoDeleteAction.h"
#include "libkwave/undo/UndoInsertAction.h"
#include "libkwave/undo/UndoTransaction.h"

//***************************************************************************
UndoInsertAction::UndoInsertAction(QWidget *parent_widget,
                                   const QList<unsigned int> &track_list,
                                   unsigned int offset, unsigned int length)
    :QObject(), UndoAction(),
     m_parent_widget(parent_widget),
     m_track_list(track_list),
     m_offset(offset), m_length(length)
{
}

//***************************************************************************
QString UndoInsertAction::description()
{
    return i18n("Insert");
}

//***************************************************************************
unsigned int UndoInsertAction::undoSize()
{
    return sizeof(*this);
}

//***************************************************************************
int UndoInsertAction::redoSize()
{
    return sizeof(UndoDeleteAction) +
	    (m_length * sizeof(sample_t) * m_track_list.count());
}

//***************************************************************************
bool UndoInsertAction::store(SignalManager &)
{
    // we delete -> nothing to store
    return true;
}

//***************************************************************************
UndoAction *UndoInsertAction::undo(SignalManager &manager, bool with_redo)
{
    UndoAction *redo_action = 0;

    // store data for redo
    if (with_redo) {
	redo_action = new UndoDeleteAction(
	    m_parent_widget, m_track_list, m_offset, m_length);
	Q_ASSERT(redo_action);
	if (!redo_action) return 0;
	redo_action->store(manager);
    }

    // now delete the samples
    manager.deleteRange(m_offset, m_length, m_track_list);

    return redo_action;
}

//***************************************************************************
void UndoInsertAction::setLength(unsigned int length)
{
    m_length = length;
}

//***************************************************************************
void UndoInsertAction::dump(const QString &indent)
{
    qDebug("%sundo insert from [%u ... %u] (%d)", indent.toLocal8Bit().data(),
           m_offset,
           m_offset + ((m_length) ? (m_length - 1) : m_length),
           m_length);
}

//***************************************************************************
#include "UndoInsertAction.moc"
//***************************************************************************
//***************************************************************************
