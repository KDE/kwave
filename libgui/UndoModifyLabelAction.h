/***************************************************************************
    UndoModifyLabelAction.h  -  Undo action for modifying labels
			     -------------------
    begin                : Sun Sep 03 2006
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

#ifndef _UNDO_MODIFY_LABEL_ACTION_H_
#define _UNDO_MODIFY_LABEL_ACTION_H_

#include "config.h"

#include <QString>

#include "libkwave/undo/UndoAction.h"

class Label;
class SignalWidget;

/**
 * Undo action for deleting a label.
 */
class UndoModifyLabelAction: public UndoAction
{
public:

    /**
     * Constructor
     * @param signal_widget reference to the SignalWidget
     * @param label reference to the label that has been changed
     */
    UndoModifyLabelAction(SignalWidget &signal_widget, Label &label);

    /** Destructor */
    virtual ~UndoModifyLabelAction();

    /**
     * Sets the last known position of the label, for finding it
     * at the time where the undo() happens (in case it has moved).
     * @param pos last known position
     */
    virtual void setLastPosition(unsigned int pos);

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
    virtual UndoGroupID group() const { return LabelEdit; };

protected:

    /** reference to the signal widget */
    SignalWidget &m_signal_widget;

    /** the modified label */
    Label *m_label;

    /** the last known location, for finding it again in undo() */
    unsigned int m_last_position;

};

#endif /* _UNDO_MODIFY_LABEL_ACTION_H_ */
