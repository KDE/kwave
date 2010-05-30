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
#include <klocale.h>

#include "libkwave/MultiTrackReader.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SignalManager.h"
#include "libkwave/undo/UndoAction.h"
#include "libkwave/undo/UndoDeleteAction.h"
#include "libkwave/undo/UndoDeleteMetaDataAction.h"
#include "libkwave/undo/UndoInsertAction.h"

//***************************************************************************
UndoDeleteAction::UndoDeleteAction(QWidget *parent_widget,
                                   const QList<unsigned int> &track_list,
                                   sample_index_t offset, sample_index_t length)
    :UndoAction(),
     m_parent_widget(parent_widget),
     m_track_list(track_list),
     m_offset(offset), m_length(length),
     m_mime_data(), m_undo_size(0)
{
    // undo size needed for samples
    m_undo_size += m_length * sizeof(sample_t) * m_track_list.count();
}

//***************************************************************************
UndoDeleteAction::~UndoDeleteAction()
{
    m_mime_data.clear();
}

//***************************************************************************
QString UndoDeleteAction::description()
{
    return i18n("Delete");
}

//***************************************************************************
unsigned int UndoDeleteAction::undoSize()
{
    return m_undo_size;
}

//***************************************************************************
int UndoDeleteAction::redoSize()
{
    return sizeof(UndoInsertAction);
}

//***************************************************************************
bool UndoDeleteAction::store(SignalManager &manager)
{
    MultiTrackReader reader(Kwave::SinglePassForward, manager,
	m_track_list, m_offset, m_offset + m_length - 1);

    // encode the data that will be deleted into a Kwave::MimeData container
    if (!m_mime_data.encode(m_parent_widget, reader, manager.metaData())) {
	m_mime_data.clear();
	return false;
    }

    return true;
}

//***************************************************************************
UndoAction *UndoDeleteAction::undo(SignalManager &manager, bool with_redo)
{
    UndoAction *redo_action = 0;

    // store data for redo
    if (with_redo) {
	redo_action = new UndoInsertAction(m_parent_widget, m_track_list,
	    m_offset, m_length);
	Q_ASSERT(redo_action);
	if (!redo_action) return 0;
	redo_action->store(manager);
    }

    // perform the undo operation
    if (!m_mime_data.decode(m_parent_widget, &m_mime_data,
                            manager, m_offset))
    {
	qWarning("UndoDeleteAction::undo() FAILED");
	delete redo_action;
	return 0;
    }

    return redo_action;
}

//***************************************************************************
void UndoDeleteAction::dump(const QString &indent)
{
    qDebug("%sundo delete from [%lu ... %lu] (%lu)",
           indent.toLocal8Bit().data(),
           static_cast<unsigned long int>(m_offset),
           static_cast<unsigned long int>(m_offset + ((m_length) ?
               (m_length - 1) : m_length)),
           static_cast<unsigned long int>(m_length));
}

//***************************************************************************
//***************************************************************************
