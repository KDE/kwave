/***************************************************************************
    UndoDeleteTrack.cpp  -  Undo action for deletion of tracks
			     -------------------
    begin                : Mon Jun 25 2001
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

#include <config.h>

#include <new>

#include <klocale.h>

#include "libkwave/SignalManager.h"
#include "libkwave/undo/UndoInsertTrack.h"
#include "libkwave/undo/UndoDeleteTrack.h"

//***************************************************************************
Kwave::UndoDeleteTrack::UndoDeleteTrack(Kwave::Signal &signal,
                                        unsigned int track)
    :UndoAction(), m_signal(signal), m_track(track),
     m_length(signal.length()), m_stripes(), m_uuid(signal.uuidOfTrack(track))
{
}

//***************************************************************************
Kwave::UndoDeleteTrack::~UndoDeleteTrack()
{
}

//***************************************************************************
QString Kwave::UndoDeleteTrack::description()
{
    return i18n("Delete Track");
}

//***************************************************************************
qint64 Kwave::UndoDeleteTrack::undoSize()
{
    return sizeof(*this) + m_length * sizeof(sample_t);
}

//***************************************************************************
qint64 Kwave::UndoDeleteTrack::redoSize()
{
    return sizeof(Kwave::UndoInsertTrack);
}

//***************************************************************************
bool Kwave::UndoDeleteTrack::store(Kwave::SignalManager &manager)
{
    if (!m_length) return true; // shortcut: this is an empty action

    // fork off a multi track stripe list for the selected range
    QList<unsigned int> track_list;
    track_list.append(m_track);
    m_stripes = manager.stripes(track_list, 0, m_length - 1);
    if (m_stripes.isEmpty())
	return false; // retrieving the stripes failed

    return true;
}

//***************************************************************************
Kwave::UndoAction *Kwave::UndoDeleteTrack::undo(Kwave::SignalManager &manager,
                                                bool with_redo)
{
    Kwave::UndoAction *redo_action = 0;

    // create a redo action
    if (with_redo) {
	redo_action =
	    new(std::nothrow) Kwave::UndoInsertTrack(m_signal, m_track);
	Q_ASSERT(redo_action);
	if (redo_action) redo_action->store(manager);
    }

    // insert an empty track into the signal
    m_signal.insertTrack(m_track, m_length, &m_uuid);

    // merge the stripes back into the signal
    QList<unsigned int> track_list;
    track_list.append(m_track);
    if (!manager.mergeStripes(m_stripes, track_list)) {
	qWarning("UndoDeleteTrack::undo() FAILED [mergeStripes]");
	delete redo_action;
	return 0;
    }

    return redo_action;
}

//***************************************************************************
//***************************************************************************
