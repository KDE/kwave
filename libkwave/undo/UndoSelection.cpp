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
#include <KLocalizedString>

#include "libkwave/SignalManager.h"
#include "libkwave/undo/UndoSelection.h"

//***************************************************************************
Kwave::UndoSelection::UndoSelection(Kwave::SignalManager &manager)
    :UndoAction(),
     m_manager(manager), m_offset(0), m_length(0), m_selected_tracks()
{
}

//***************************************************************************
Kwave::UndoSelection::UndoSelection(Kwave::SignalManager &manager,
                                    QVector<unsigned int> selected_tracks,
                                    sample_index_t offset,
                                    sample_index_t length)
    :UndoAction(),
     m_manager(manager), m_offset(offset), m_length(length),
     m_selected_tracks(selected_tracks)
{
}

//***************************************************************************
Kwave::UndoSelection::~UndoSelection()
{
}

//***************************************************************************
QString Kwave::UndoSelection::description()
{
    return i18n("Selection");
}

//***************************************************************************
qint64 Kwave::UndoSelection::undoSize()
{
    return sizeof(*this) +
        (m_selected_tracks.count() * sizeof(unsigned int));
}

//***************************************************************************
qint64 Kwave::UndoSelection::redoSize()
{
    return sizeof(*this) +
        (m_manager.selectedTracks().count() * sizeof(unsigned int));
}

//***************************************************************************
bool Kwave::UndoSelection::store(Kwave::SignalManager &manager)
{
    m_offset = manager.selection().offset();
    m_length = manager.selection().length();
    m_selected_tracks = manager.selectedTracks();

    return true;
}

//***************************************************************************
Kwave::UndoAction *Kwave::UndoSelection::undo(Kwave::SignalManager &manager,
                                              bool with_redo)
{
    // store current selection for later redo
    sample_index_t old_offset = manager.selection().offset();
    sample_index_t old_length = manager.selection().length();
    QVector<unsigned int> old_selected_tracks = manager.selectedTracks();

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
        return Q_NULLPTR;
    }
}

//***************************************************************************
void Kwave::UndoSelection::dump(const QString &indent)
{
    qDebug("%sselect from [%lu ... %lu] (%lu)", DBG(indent),
           static_cast<unsigned long int>(m_offset),
           static_cast<unsigned long int>(m_offset + ((m_length) ?
               (m_length - 1) : m_length)),
           static_cast<unsigned long int>(m_length));
}

//***************************************************************************
//***************************************************************************
