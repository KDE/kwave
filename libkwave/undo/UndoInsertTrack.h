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

#ifndef UNDO_INSERT_TRACK_H
#define UNDO_INSERT_TRACK_H

#include "config.h"

#include <QString>

#include "libkwave/undo/UndoAction.h"

namespace Kwave
{

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
	UndoInsertTrack(Kwave::Signal &signal, unsigned int track);

	/** Destructor */
	virtual ~UndoInsertTrack();

	/**
	 * Returns a verbose short description of the action.
	 */
	QString description() Q_DECL_OVERRIDE;

	/** @see UndoAction::undoSize() */
	qint64 undoSize() Q_DECL_OVERRIDE;

	/** @see UndoAction::redoSize() */
	qint64 redoSize() Q_DECL_OVERRIDE;

	/** @see UndoAction::store() */
	bool store(Kwave::SignalManager &manager) Q_DECL_OVERRIDE;

	/** @see UndoAction::undo() */
	Kwave::UndoAction *undo(Kwave::SignalManager &manager,
	                                bool with_redo) Q_DECL_OVERRIDE;

    protected:

	/** Reference to the signal */
	Kwave::Signal &m_signal;

	/** Index of the inserted track */
	unsigned int m_track;

    };
}

#endif /* UNDO_INSERT_TRACK_H */

//***************************************************************************
//***************************************************************************
