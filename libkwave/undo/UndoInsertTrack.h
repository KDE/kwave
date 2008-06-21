/***************************************************************************
      UndoInsertTrack.h  -  Undo action for insertion of tracks
			     -------------------
    begin                : Sun Jun 24 2001
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

#ifndef _UNDO_INSERT_TRACK_H_
#define _UNDO_INSERT_TRACK_H_

#include "config.h"

#include <QString>

#include "libkwave/undo/UndoAction.h"

class Signal;

/**
 * Undo action for inserting a track.
 */
class UndoInsertTrack :public UndoAction
{

public:

    /**
     * Constructor
     * @param signal reference to the signal
     * @param track index of the inserted track.
     */
    UndoInsertTrack(Signal &signal, unsigned int track);

    /** Destructor */
    virtual ~UndoInsertTrack();

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

protected:

    /** Reference to the signal */
    Signal &m_signal;

    /** Index of the inserted track */
    unsigned int m_track;

};

#endif /* _UNDO_INSERT_TRACK_H_ */
