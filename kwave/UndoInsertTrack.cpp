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

#include <klocale.h>

#include "SignalManager.h"
#include "UndoInsertTrack.h"
#include "UndoDeleteTrack.h"

//***************************************************************************
UndoInsertTrack::UndoInsertTrack(Signal &signal, unsigned int track)
    :UndoAction(), m_signal(signal), m_track(track)
{
}

//***************************************************************************
UndoInsertTrack::~UndoInsertTrack()
{
}

//***************************************************************************
QString UndoInsertTrack::description()
{
    return i18n("insert track");
}

//***************************************************************************
unsigned int UndoInsertTrack::undoSize()
{
    return sizeof(*this);
}

//***************************************************************************
int UndoInsertTrack::redoSize()
{
    return (m_signal.length() * sizeof(sample_t));
}

//***************************************************************************
void UndoInsertTrack::store(SignalManager &)
{
    // nothing to do, the track number has already
    // been stored in the constructor
}

//***************************************************************************
UndoAction *UndoInsertTrack::undo(SignalManager &manager, bool with_redo)
{
    UndoAction *redo = 0;

    // store data for redo
    if (with_redo) {
	redo = new UndoDeleteTrack(m_signal, m_track);
	ASSERT(redo);
	if (redo) redo->store(manager);
    }

    // remove the track from the signal
    m_signal.deleteTrack(m_track);

    return redo;
}

//***************************************************************************
//***************************************************************************
