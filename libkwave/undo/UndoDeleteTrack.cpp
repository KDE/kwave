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

#include "config.h"

#include <klocale.h>

#include "libkwave/InsertMode.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SignalManager.h"
#include "libkwave/Writer.h"
#include "libkwave/undo/UndoInsertTrack.h"
#include "libkwave/undo/UndoDeleteTrack.h"

//***************************************************************************
UndoDeleteTrack::UndoDeleteTrack(Kwave::Signal &signal, unsigned int track)
    :UndoAction(), m_signal(signal), m_track(track),
     m_length(signal.length()), m_buffer_track()
{
}

//***************************************************************************
UndoDeleteTrack::~UndoDeleteTrack()
{
}

//***************************************************************************
QString UndoDeleteTrack::description()
{
    return i18n("Delete Track");
}

//***************************************************************************
unsigned int UndoDeleteTrack::undoSize()
{
    return sizeof(*this) + m_length * sizeof(sample_t);
}

//***************************************************************************
int UndoDeleteTrack::redoSize()
{
    return sizeof(UndoInsertTrack);
}

//***************************************************************************
bool UndoDeleteTrack::store(SignalManager &manager)
{
    SampleReader *reader = manager.openSampleReader(
	Kwave::SinglePassForward, m_track, 0, m_length-1);
    Q_ASSERT(reader);
    if (!reader) return false;

    Kwave::Writer *writer = m_buffer_track.openWriter(Append, 0, m_length - 1);
    Q_ASSERT(writer);
    if (!writer) {
	delete reader;
	return false;
    }

    // copy the data
    (*writer) << (*reader);

    delete reader;
    delete writer;
    return (m_buffer_track.length() == m_length);
}

//***************************************************************************
UndoAction *UndoDeleteTrack::undo(SignalManager &manager, bool with_redo)
{
    UndoAction *redo_action = 0;

    // create a redo action
    if (with_redo) {
	redo_action = new UndoInsertTrack(m_signal, m_track);
	Q_ASSERT(redo_action);
	if (redo_action) redo_action->store(manager);
    }

    // insert an empty track into the signal
    m_signal.insertTrack(m_track, m_length);

    // restore the sample data from the internal buffer
    Kwave::Writer *writer = m_signal.openWriter(m_track,
	Overwrite, 0, m_length-1);
    Q_ASSERT(writer);

    SampleReader *reader = m_buffer_track.openSampleReader(
	Kwave::SinglePassForward, 0, m_length-1);
    Q_ASSERT(reader);
    if (reader && writer) (*writer) << (*reader);

    if (reader) delete reader;
    if (writer) delete writer;

    return redo_action;
}

//***************************************************************************
//***************************************************************************
