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
    debug("UndoModifyAction::UndoModifyAction(%u,%u,%u)", track,
	  offset, length); // ###
}

//***************************************************************************
UndoModifyAction::~UndoModifyAction()
{
    debug("UndoModifyAction::~UndoModifyAction()"); // ###
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
    debug("UndoModifyAction::store(), offset=%u, length=%u",
	m_offset, m_length);
    // ### TODO ###

    m_buffer.resize(m_length);
    SampleReader *reader = manager.openSampleReader(
	0, m_offset, m_offset+m_length-1);
    unsigned int ofs = 0;
    unsigned int len = m_length;
    while (reader && len--) {
	*reader >> m_buffer[ofs++];
    }

}

//***************************************************************************
UndoAction *UndoModifyAction::undo(SignalManager &manager)
{
    /* exchange samples from current signal and buffer and return this */
    // ### TODO ###
    debug("UndoModifyAction::undo(), offset=%u, length=%u",
	m_offset, m_length);

    SampleWriter *writer = manager.openSampleWriter(
	0, Overwrite, m_offset, m_offset+m_length-1);
    SampleReader *reader = manager.openSampleReader(
	0, m_offset, m_offset+m_length-1);
    unsigned int ofs = 0;
    unsigned int len = m_length;
    sample_t s;
    while (reader && writer && len--) {
	*reader >> s;
	*writer << m_buffer[ofs];
	m_buffer[ofs] = s;
	ofs++;
    }
    if (writer) delete writer;
    if (reader) delete reader;

    return this;
}

//***************************************************************************
//***************************************************************************
