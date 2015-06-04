/***************************************************************************
    UndoInsertTrack.cpp  -  Undo action for insertion of tracks
			     -------------------
    begin                : Sun Jun 24 2001
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

#include <KI18n/KLocalizedString>

#include <new>

#include "libkwave/SignalManager.h"
#include "libkwave/undo/UndoDeleteTrack.h"
#include "libkwave/undo/UndoInsertTrack.h"

//***************************************************************************
Kwave::UndoInsertTrack::UndoInsertTrack(Kwave::Signal &signal,
                                        unsigned int track)
    :UndoAction(), m_signal(signal), m_track(track)
{
}

//***************************************************************************
Kwave::UndoInsertTrack::~UndoInsertTrack()
{
}

//***************************************************************************
QString Kwave::UndoInsertTrack::description()
{
    return i18n("Insert Track");
}

//***************************************************************************
qint64 Kwave::UndoInsertTrack::undoSize()
{
    return sizeof(*this);
}

//***************************************************************************
qint64 Kwave::UndoInsertTrack::redoSize()
{
    return (m_signal.length() * sizeof(sample_t)) + sizeof(UndoDeleteTrack);
}

//***************************************************************************
bool Kwave::UndoInsertTrack::store(Kwave::SignalManager &)
{
    // nothing to do, the track number has already
    // been stored in the constructor
    return true;
}

//***************************************************************************
Kwave::UndoAction *Kwave::UndoInsertTrack::undo(
    Kwave::SignalManager &manager, bool with_redo)
{
    Kwave::UndoAction *redo = 0;

    // store data for redo
    if (with_redo) {
	redo = new(std::nothrow) Kwave::UndoDeleteTrack(m_signal, m_track);
	Q_ASSERT(redo);
	if (redo) redo->store(manager);
    }

    // remove the track from the signal
    m_signal.deleteTrack(m_track);

    return redo;
}

//***************************************************************************
//***************************************************************************
