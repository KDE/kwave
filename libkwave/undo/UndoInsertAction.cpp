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

#include <new>

#include <KLocalizedString>

#include "libkwave/SignalManager.h"
#include "libkwave/undo/UndoAction.h"
#include "libkwave/undo/UndoDeleteAction.h"
#include "libkwave/undo/UndoInsertAction.h"

//***************************************************************************
Kwave::UndoInsertAction::UndoInsertAction(QWidget *parent_widget,
                                          const QList<unsigned int> &track_list,
                                          sample_index_t offset,
                                          sample_index_t length)
    :QObject(), UndoAction(),
     m_parent_widget(parent_widget),
     m_track_list(track_list),
     m_offset(offset), m_length(length)
{
}

//***************************************************************************
QString Kwave::UndoInsertAction::description()
{
    return i18n("Insert");
}

//***************************************************************************
qint64 Kwave::UndoInsertAction::undoSize()
{
    return sizeof(*this);
}

//***************************************************************************
qint64 Kwave::UndoInsertAction::redoSize()
{
    return sizeof(UndoDeleteAction) +
	    (m_length * sizeof(sample_t) * m_track_list.count());
}

//***************************************************************************
bool Kwave::UndoInsertAction::store(SignalManager &)
{
    // we delete -> nothing to store
    return true;
}

//***************************************************************************
Kwave::UndoAction *Kwave::UndoInsertAction::undo(
    Kwave::SignalManager &manager, bool with_redo)
{
    Kwave::UndoAction *redo_action = Q_NULLPTR;

    // store data for redo
    if (with_redo) {
	redo_action = new(std::nothrow) Kwave::UndoDeleteAction(
	    m_parent_widget, m_track_list, m_offset, m_length);
	Q_ASSERT(redo_action);
        if (!redo_action) return Q_NULLPTR;
	redo_action->store(manager);
    }

    // now delete the samples
    manager.deleteRange(m_offset, m_length, m_track_list);

    return redo_action;
}

//***************************************************************************
void Kwave::UndoInsertAction::setLength(sample_index_t length)
{
    m_length = length;
}

//***************************************************************************
void Kwave::UndoInsertAction::dump(const QString &indent)
{
    qDebug("%sundo insert from [%lu ... %lu] (%lu)", DBG(indent),
           static_cast<unsigned long int>(m_offset),
           static_cast<unsigned long int>(m_offset + ((m_length) ?
               (m_length - 1) : m_length)),
           static_cast<unsigned long int>(m_length));
}

//***************************************************************************
//***************************************************************************
