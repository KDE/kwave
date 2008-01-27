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

#include "SignalManager.h"
#include "UndoSelection.h"

//***************************************************************************
UndoSelection::UndoSelection(SignalManager &manager)
    :UndoAction(),
     m_manager(manager), m_offset(0), m_length(0), m_selected_tracks()
{
}

//***************************************************************************
UndoSelection::~UndoSelection()
{
}

//***************************************************************************
QString UndoSelection::description()
{
    return i18n("selection");
}

//***************************************************************************
unsigned int UndoSelection::undoSize()
{
    return sizeof(*this) + m_selected_tracks.count() * sizeof(unsigned int);
}

//***************************************************************************
int UndoSelection::redoSize()
{
    return (m_manager.selectedTracks().count() -
            m_selected_tracks.count()) * sizeof(unsigned int);
}

//***************************************************************************
void UndoSelection::store(SignalManager &manager)
{
    m_offset = manager.selection().offset();
    m_length = manager.selection().length();
    m_selected_tracks = manager.selectedTracks();
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
//***************************************************************************
