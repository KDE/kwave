/***************************************************************************
   UndoDeleteAction.cpp  -  UndoAction for deletion of a range of samples
			     -------------------
    begin                : Jun 08 2001
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

#include "libkwave/SampleWriter.h"
#include "SignalManager.h"
#include "UndoAction.h"
#include "UndoDeleteAction.h"
#include "UndoInsertAction.h"

//***************************************************************************
UndoDeleteAction::UndoDeleteAction(unsigned int track,
	unsigned int offset, unsigned int length)
    :UndoModifyAction(track, offset, length)
{
}

//***************************************************************************
QString UndoDeleteAction::description()
{
    return i18n("delete");
}

//***************************************************************************
int UndoDeleteAction::redoSize()
{
    return sizeof(*this) - undoSize();
}

//***************************************************************************
UndoAction *UndoDeleteAction::undo(SignalManager &manager, bool with_redo)
{
    // open a SampleWriter for reading back the data from the buffer
    SampleWriter *writer = manager.openSampleWriter(m_track, Insert,
        m_offset, m_offset+m_length-1);
    Q_ASSERT(writer);
    if (!writer) return 0;

    // read back the buffer
    *writer << m_buffer;

    // now the writer and the buffer are no longer needed
    delete writer;
    m_buffer.resize(0);

    if (with_redo) {
	return new UndoInsertAction(m_track, m_offset, m_length);
    } else {
	return 0;
    }
}

//***************************************************************************
//***************************************************************************
