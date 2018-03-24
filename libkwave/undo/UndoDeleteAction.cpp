/***************************************************************************
   UndoDeleteAction.cpp  -  UndoAction for deletion of a range of samples
			     -------------------
    begin                : Jun 08 2001
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

#include "libkwave/MultiTrackReader.h"
#include "libkwave/SignalManager.h"
#include "libkwave/undo/UndoAction.h"
#include "libkwave/undo/UndoDeleteAction.h"
#include "libkwave/undo/UndoDeleteMetaDataAction.h"
#include "libkwave/undo/UndoInsertAction.h"

//***************************************************************************
Kwave::UndoDeleteAction::UndoDeleteAction(QWidget *parent_widget,
                                          const QList<unsigned int> &track_list,
                                          sample_index_t offset,
                                          sample_index_t length)
    :Kwave::UndoAction(),
     m_parent_widget(parent_widget),
     m_stripes(), m_meta_data(),
     m_track_list(track_list),
     m_offset(offset), m_length(length),
     m_undo_size(sizeof(*this))
{
    // undo size needed for samples
    m_undo_size += m_length * sizeof(sample_t) * m_track_list.count();
}

//***************************************************************************
Kwave::UndoDeleteAction::~UndoDeleteAction()
{
    m_stripes.clear();
}

//***************************************************************************
QString Kwave::UndoDeleteAction::description()
{
    return i18n("Delete");
}

//***************************************************************************
qint64 Kwave::UndoDeleteAction::undoSize()
{
    return m_undo_size;
}

//***************************************************************************
qint64 Kwave::UndoDeleteAction::redoSize()
{
    return sizeof(Kwave::UndoInsertAction);
}

//***************************************************************************
bool Kwave::UndoDeleteAction::store(Kwave::SignalManager &manager)
{
    if (!m_length) return true; // shortcut: this is an empty action

    // fork off a multi track stripe list for the selected range
    const sample_index_t left  = m_offset;
    const sample_index_t right = m_offset + m_length - 1;
    m_stripes = manager.stripes(m_track_list, left, right);
    if (m_stripes.isEmpty())
	return false; // retrieving the stripes failed

    // save the meta data
    m_meta_data = manager.metaData().copy(m_offset, m_length);

    return true;
}

//***************************************************************************
Kwave::UndoAction *Kwave::UndoDeleteAction::undo(Kwave::SignalManager &manager,
                                                 bool with_redo)
{
    Kwave::UndoAction *redo_action = Q_NULLPTR;

    // store data for redo
    if (with_redo) {
	redo_action = new(std::nothrow) Kwave::UndoInsertAction(
	    m_parent_widget, m_track_list,
	    m_offset, m_length
	);
	Q_ASSERT(redo_action);
        if (!redo_action) return Q_NULLPTR;
	redo_action->store(manager);
    }

    if (!m_length) return redo_action; // shortcut: this is an empty action

    // insert space for the stripes
    if (!manager.insertSpace(m_offset, m_length, m_track_list)) {
	qWarning("UndoDeleteAction::undo() FAILED [insertSpace]");
	delete redo_action;
        return Q_NULLPTR;
    }

    // merge the stripes back into the signal
    if (!manager.mergeStripes(m_stripes, m_track_list)) {
	qWarning("UndoDeleteAction::undo() FAILED [mergeStripes]");
	delete redo_action;
        return Q_NULLPTR;
    }

    // restore the saved meta data
    manager.metaData().add(m_meta_data);

    return redo_action;
}

//***************************************************************************
void Kwave::UndoDeleteAction::dump(const QString &indent)
{
    qDebug("%sundo delete from [%lu ... %lu] (%lu)", DBG(indent),
           static_cast<unsigned long int>(m_offset),
           static_cast<unsigned long int>(m_offset + ((m_length) ?
               (m_length - 1) : m_length)),
           static_cast<unsigned long int>(m_length));
}

//***************************************************************************
//***************************************************************************
