/***************************************************************************
      UndoDeleteTrack.h  -  Undo action for deletion of tracks
			     -------------------
    begin                : Mon Jun 25 2001
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

#ifndef _UNDO_DELETE_TRACK_H_
#define _UNDO_DELETE_TRACK_H_

#include "config.h"
#include <qarray.h>
#include <qstring.h>
#include "libkwave/Sample.h"
#include "kwave/UndoAction.h"

class Signal;

/**
 * Undo action for deleting a track.
 */
class UndoDeleteTrack :public UndoAction
{

public:

    /**
     * Constructor
     * @param signal reference to the signal
     * @param track index of the deleted track.
     */
    UndoDeleteTrack(Signal &signal, unsigned int track);

    /** Destructor */
    virtual ~UndoDeleteTrack();

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

protected:

    /** Reference to the signal */
    Signal &m_signal;

    /** Index of the deleted track */
    unsigned int m_track;

    /** Length of the track in samples */
    unsigned int m_length;

    /** Buffer with undo data */
    QArray<sample_t> m_buffer;

};

#endif /* _UNDO_DELETE_TRACK_H_ */
