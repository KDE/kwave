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

#include <QString>

#include "libkwave/Sample.h"
#include "libkwave/Track.h"
#include "libkwave/undo/UndoAction.h"

namespace Kwave
{

    class Signal;

    /**
     * Undo action for deleting a track.
     */
    class UndoDeleteTrack: public Kwave::UndoAction
    {
    public:

	/**
	 * Constructor
	 * @param signal reference to the signal
	 * @param track index of the deleted track.
	 */
	UndoDeleteTrack(Kwave::Signal &signal, unsigned int track);

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
	virtual bool store(SignalManager &manager);

	/** @see UndoAction::undo() */
	virtual Kwave::UndoAction *undo(Kwave::SignalManager &manager,
	                                bool with_redo);

    protected:

	/** Reference to the signal */
	Kwave::Signal &m_signal;

	/** Index of the deleted track */
	unsigned int m_track;

	/** Length of the track in samples */
	sample_index_t m_length;

	/** track that serves as buffer with undo data */
	Kwave::Track m_buffer_track;

    };
}

#endif /* _UNDO_DELETE_TRACK_H_ */

//***************************************************************************
//***************************************************************************
