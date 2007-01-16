/***************************************************************************
    UndoDeleteLabelAction.h  -  Undo action for deleting labels
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

#ifndef _UNDO_DELETE_LABEL_ACTION_H_
#define _UNDO_DELETE_LABEL_ACTION_H_

#include "config.h"
#include <qstring.h>
#include "UndoAction.h"

class Label;
class SignalWidget;

/**
 * Undo action for deleting a label.
 */
class UndoDeleteLabelAction :public UndoAction
{
public:

    /**
     * Constructor
     * @param signal_widget reference to the SignalWidget
     * @param label reference to the label that should be deleted
     */
    UndoDeleteLabelAction(SignalWidget &signal_widget, Label &label);

    /** Destructor */
    virtual ~UndoDeleteLabelAction();

    /**
     * Returns a verbose short description of the action.
     */
    virtual QString description();

    /** @see UndoAction::undoSize() */
    virtual unsigned int undoSize();

    /** @see UndoAction::redoSize() */
    virtual int redoSize();

    /** @see UndoAction::store() */
    virtual void store(SignalManager &manager);

    /** @see UndoAction::undo() */
    virtual UndoAction *undo(SignalManager &manager, bool with_redo);

    /** @see UndoAction::group() */
    virtual UndoGroupID group() const { return LabelCommands; };

protected:

    /** reference to the signal widget */
    SignalWidget &m_signal_widget;

    /** the deleted label */
    Label *m_label;

};

#endif /* _UNDO_DELETE_LABEL_ACTION_H_ */
