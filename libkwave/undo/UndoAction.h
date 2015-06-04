/***************************************************************************
           UndoAction.h  -  Abstract base class for undo actions
			     -------------------
    begin                : Sat May 26 2001
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

#ifndef UNDO_ACTION_H
#define UNDO_ACTION_H

#include "config.h"

#include <QtGlobal>
#include <QString>

#include "libkwave/String.h"

namespace Kwave
{

    class SignalManager;

    /**
     * This is an abstract base class used for implementation of undo and redo.
     * The constructor should take all information needed for storing the
     * undo data and for being able to guess the amount of the needed memory.
     * After construction, the caller can call size() for retaining the
     * amount of needed memory and o provide enough space. The store() function
     * can then save the data for further undo. Calling the undo() function
     * also creates a new UndoAction object that is responsible for holding
     * all information for undoing that undo (redo).
     */
    class Q_DECL_EXPORT UndoAction
    {

    public:

	/** Destructor */
	virtual ~UndoAction()
	{
	}

	/**
	 * Returns a verbose short description of the action.
	 */
	virtual QString description() = 0;

	/**
	 * Returns the required amount of memory that is needed for storing
	 * undo data for the operation. This will be called to determine the
	 * free memory to be reserved.
	 * @note this is the first step (after the constructor)
	 */
	virtual qint64 undoSize() = 0;

	/**
	 * Returns the difference of needed memory that is needed for
	 * redo.
	 */
	virtual qint64 redoSize() = 0;

	/**
	 * Stores the data needed for undo.
	 * @param manager the SignalManager for modifying the signal
	 * @note this is the second step, after size() has been called
	 * @return true if successful, false if failed (e.g. out of memory)
	 */
	virtual bool store(Kwave::SignalManager &manager) = 0;

	/**
	 * Takes back an action by creating a new undo action (for further
	 * redo) and restoring the previous state.
	 * @param manager the SignalManager for modifying the signal
	 * @param with_redo if true a UndoAction for redo will be created
	 * @note The return value is allowed to be the same object. This
	 *       is useful for objects that can re-use their data for
	 *       undo/redo. You have to check for this when deleting an
	 *       UndoAction object after undo.
	 */
	virtual Kwave::UndoAction *undo(Kwave::SignalManager &manager,
	                                bool with_redo) = 0;

	/**
	 * Determines whether a undo action has to do with a modification
	 * of the signal or meta information and leads to a "modified"
	 * file. The default implementation returns "true". An example for
	 * a implementation that returns "false" is the UndoSelection class
	 * which contains only information about a change in selected samples.
	 * @return true if a modification is contained, false if not.
	 */
	virtual bool containsModification() const { return true; }

	/** dump, for debugging purposes */
	virtual void dump(const QString &indent) {
	    qDebug("%s%s", DBG(indent), DBG(description()));
	}

    };
}

#endif /* UNDO_ACTION_H */

//***************************************************************************
//***************************************************************************
