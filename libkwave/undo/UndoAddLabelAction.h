/***************************************************************************
   UndoAddLabelAction.h  -  Undo action for insertion of labels
			     -------------------
    begin                : Wed Aug 16 2006
    copyright            : (C) 2006 by Thomas Eschenbacher
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

#ifndef _UNDO_ADD_LABEL_ACTION_H_
#define _UNDO_ADD_LABEL_ACTION_H_

#include "config.h"
#include <QString>

#include "libkwave/undo/UndoAction.h"

class Label;
class SignalManager;

/**
 * Undo action for inserting a label.
 */
class UndoAddLabelAction: public UndoAction
{
public:

    /**
     * Constructor
     * @param index numerical index of the inserted label
     */
    UndoAddLabelAction(int index);

    /** Destructor */
    virtual ~UndoAddLabelAction();

    /**
     * Returns a verbose short description of the action.
     */
    virtual QString description();

    /** @see UndoAction::undoSize() */
    virtual unsigned int undoSize();

    /** @see UndoAction::redoSize() */
    virtual int redoSize();

    /** @see UndoAction::store() */
    virtual bool store(SignalManager &manager);

    /** @see UndoAction::undo() */
    virtual UndoAction *undo(SignalManager &manager, bool with_redo);

    /** @see UndoAction::group() */
    virtual UndoGroupID group() const { return LabelCommands; };

protected:

    /** index of the inserted label */
    unsigned int m_index;

};

#endif /* _UNDO_ADD_LABEL_ACTION_H_ */
