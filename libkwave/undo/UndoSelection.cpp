/***************************************************************************
      UndoSelection.cpp  -  Undo action for selection
			     -------------------
    begin                : Tue Jun 05 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#include "libkwave/SignalManager.h"
#include "libkwave/undo/UndoSelection.h"

//***************************************************************************
UndoSelection::UndoSelection(SignalManager &manager)
    :UndoAction(),
     m_manager(manager), m_offset(0), m_length(0), m_selected_tracks()
{
}

//***************************************************************************
UndoSelection::UndoSelection(SignalManager &manager,
                             QList<unsigned int> selected_tracks,
                             unsigned int offset,
                             unsigned int length)
    :UndoAction(),
     m_manager(manager), m_offset(offset), m_length(length),
     m_selected_tracks(selected_tracks)
{
}

//***************************************************************************
UndoSelection::~UndoSelection()
{
}

//***************************************************************************
QString UndoSelection::description()
{
    return i18n("Selection");
}

//***************************************************************************
unsigned int UndoSelection::undoSize()
{
    return sizeof(*this) +
	(m_selected_tracks.count() * sizeof(unsigned int));
}

//***************************************************************************
int UndoSelection::redoSize()
{
    return sizeof(*this) +
	(m_manager.selectedTracks().count() * sizeof(unsigned int));
}

//***************************************************************************
bool UndoSelection::store(SignalManager &manager)
{
    m_offset = manager.selection().offset();
    m_length = manager.selection().length();
    m_selected_tracks = manager.selectedTracks();

    return true;
}

//***************************************************************************
UndoAction *UndoSelection::undo(SignalManager &manager, bool with_redo)
{
    // store current selection for later redo
    unsigned int old_offset = manager.selection().offset();
    unsigned int old_length = manager.selection().length();
    QList<unsigned int> old_selected_tracks = manager.selectedTracks();

    // restore the previous selection
    manager.selectRange(m_offset, m_length);
    manager.selectTracks(m_selected_tracks);

    // store data for redo
    if (with_redo) {
	m_offset = old_offset;
	m_length = old_length;
	m_selected_tracks = old_selected_tracks;
	return this;
    } else {
	return 0;
    }
}

//***************************************************************************
void UndoSelection::dump(const QString &indent)
{
    qDebug("%sselect from [%u ... %u] (%d)", indent.toLocal8Bit().data(),
           m_offset,
           m_offset + ((m_length) ? (m_length - 1) : m_length),
           m_length);
}

//***************************************************************************
//***************************************************************************
