/***************************************************************************
     UndoModifyAction.h  -  UndoAction for modifications on samples
			     -------------------
    begin                : May 25 2001
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

#include <klocale.h>

#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SampleWriter.h"

#include "kwave/SignalManager.h"

#include "UndoModifyAction.h"

//***************************************************************************
UndoModifyAction::UndoModifyAction(unsigned int track, unsigned int offset,
                                   unsigned int length)
    :UndoAction(), m_track(track), m_offset(offset), m_length(length),
     m_buffer(0)
{
}

//***************************************************************************
UndoModifyAction::~UndoModifyAction()
{
    m_buffer.resize(0);
}

//***************************************************************************
QString UndoModifyAction::description()
{
    return i18n("modify samples");
}

//***************************************************************************
unsigned int UndoModifyAction::undoSize()
{
    return sizeof(*this) + (m_length * sizeof(sample_t));
}

//***************************************************************************
void UndoModifyAction::store(SignalManager &manager)
{
    /* copy from left to right into our buffer */
    m_buffer.resize(m_length);
    SampleReader *reader = manager.openSampleReader(
	m_track, m_offset, m_offset+m_length-1);
    ASSERT(reader);
    if (reader) reader->read(m_buffer, 0, m_length);
}

//***************************************************************************
UndoAction *UndoModifyAction::undo(SignalManager &manager, bool with_redo)
{
    SampleWriter *writer = manager.openSampleWriter(
	m_track, Overwrite, m_offset, m_offset+m_length-1);
    SampleReader *reader = (with_redo) ? manager.openSampleReader(
	m_track, m_offset, m_offset+m_length-1) : 0;
    unsigned int ofs = 0;
    unsigned int len = m_length;
    sample_t s;
    while (reader && writer && len--) {
	if (with_redo) {
	    *reader >> s;
	    *writer << m_buffer[ofs];
	    m_buffer[ofs] = s;
	    ofs++;
	} else {
	    *writer << m_buffer[ofs++];
	}
    }
    if (writer) delete writer;
    if (reader) delete reader;

    return (with_redo) ? this : 0;
}

//***************************************************************************
//***************************************************************************
