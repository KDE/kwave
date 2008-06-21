/***************************************************************************
        UndoSelection.h  -  Undo action for selection
			     -------------------
    begin                : Tue Jun 05 2001
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

#ifndef _UNDO_SELECTION_H_
#define _UNDO_SELECTION_H_

#include "config.h"
#include <QList>

#include "libkwave/undo/UndoAction.h"

class SignalManager;

/**
 * This Undo action simply stores the combination of the currently selected
 * tracks and the currently selected range of samples.
 */
class UndoSelection: public UndoAction
{

public:

    /**
     * Constructor.
     * @param manager reference to the SignalManager
     */
    UndoSelection(SignalManager &manager);

    /** virtual destructor */
    virtual ~UndoSelection();

    /** @see UndoAction::description() */
    virtual QString description();

    /** @see UndoAction::undoSize() */
    virtual unsigned int undoSize();

    /** @see UndoAction::redoSize() */
    virtual int redoSize();

    /** @see UndoAction::store() */
    virtual bool store(SignalManager &manager);

    /** @see UndoAction::undo() */
    virtual UndoAction *undo(SignalManager &manager, bool with_redo);

private:

    /** reference to the SignalManager is needed in redoSize() */
    SignalManager &m_manager;

    /** First selected sample */
    unsigned int m_offset;

    /** Number of selected samples */
    unsigned int m_length;

    /** Array with indices of selected tracks. */
    QList<unsigned int> m_selected_tracks;

};

#endif /* _UNDO_SELECTION_H_ */
