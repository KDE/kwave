/***************************************************************************
     UndoDeleteAction.h  -  UndoAction for deletion of a range of samples
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

#ifndef _UNDO_DELETE_ACTION_H_
#define _UNDO_DELETE_ACTION_H_

#include "config.h"
#include <qstring.h>
#include "UndoModifyAction.h"

class SignalManager;

class UndoDeleteAction: public UndoModifyAction
{
public:

    /**
     * Constructor.
     * @param track index of the track
     * @param offset index of the first sample to be deleted
     * @param length number of samples to delete
     */
    UndoDeleteAction(unsigned int track, unsigned int offset,
                     unsigned int length);

    /** Destructor */
    virtual ~UndoDeleteAction() { };

    /** @see UndoAction::description() */
    virtual QString description();

    /** @see UndoAction::redoSize() */
    virtual int redoSize();

    /**
     * Copies the samples to be deleted to the internal buffer.
     * @see UndoAction::undo()
     */
    virtual UndoAction *undo(SignalManager &manager, bool with_redo);

};

#endif /* _UNDO_DELETE_ACTION_H_ */
