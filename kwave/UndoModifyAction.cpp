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

#include "UndoModifyAction.h"

//***************************************************************************
UndoModifyAction::UndoModifyAction(unsigned int track, unsigned int left,
                                   unsigned int right)
    :UndoAction(), m_track(track), m_left(left), m_right(right),
     m_buffer(0)
{
    debug("UndoModifyAction::UndoModifyAction(%u,%u,%u)", track,
	  left, right); // ###
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
unsigned int UndoModifyAction::size()
{
    return sizeof(*this) + (m_right-m_left+1) * sizeof(sample_t);
}

//***************************************************************************
void UndoModifyAction::store()
{
    /* copy from left to right into our buffer */
    debug("UndoModifyAction::store()");
    // ### TODO ###
}

//***************************************************************************
UndoAction *UndoModifyAction::undo(SignalManager &manager)
{
    /* exchange samples from current signal and buffer and return this */
    // ### TODO ###
    debug("UndoModifyAction::undo()");
    return this;
}

//***************************************************************************
//***************************************************************************
