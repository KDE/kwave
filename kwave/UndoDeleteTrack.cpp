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

#include <klocale.h>

#include "libkwave/InsertMode.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SampleWriter.h"
#include "SignalManager.h"
#include "UndoInsertTrack.h"
#include "UndoDeleteTrack.h"

//***************************************************************************
UndoDeleteTrack::UndoDeleteTrack(Signal &signal, unsigned int track)
    :UndoAction(), m_signal(signal), m_track(track),
     m_length(signal.length()), m_buffer()
{
}

//***************************************************************************
UndoDeleteTrack::~UndoDeleteTrack()
{
}

//***************************************************************************
QString UndoDeleteTrack::description()
{
    return i18n("delete track");
}

//***************************************************************************
unsigned int UndoDeleteTrack::undoSize()
{
    return sizeof(*this) + m_length * sizeof(sample_t);
}

//***************************************************************************
int UndoDeleteTrack::redoSize()
{
    return (-1 * undoSize());
}

//***************************************************************************
void UndoDeleteTrack::store(SignalManager &manager)
{
    // allocate the buffer
    m_buffer.resize(m_length);
    Q_ASSERT(m_buffer.size() == m_length);

    // create a reader for the whole range
    SampleReader *reader = manager.openSampleReader(m_track, 0, m_length-1);
    Q_ASSERT(reader);
    if (!reader) return;

    // read into the save buffer
    *reader >> m_buffer;

    // discard the reader
    delete reader;
}

//***************************************************************************
UndoAction *UndoDeleteTrack::undo(SignalManager &manager, bool with_redo)
{
    UndoAction *redo = 0;

    // create a redo action
    if (with_redo) {
	redo = new UndoInsertTrack(m_signal, m_track);
	Q_ASSERT(redo);
	if (redo) redo->store(manager);
    }

    // insert an empty track into the signal
    m_signal.insertTrack(m_track, m_length);

    // restore the sample data from the internal buffer
    SampleWriter *writer = m_signal.openSampleWriter(m_track,
	Overwrite, 0, m_length-1);
    Q_ASSERT(writer);
    if (writer) {
	*writer << m_buffer;
	delete writer;
    }

    return redo;
}

//***************************************************************************
//***************************************************************************
