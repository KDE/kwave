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

#ifndef UNDO_SELECTION_H
#define UNDO_SELECTION_H

#include "config.h"
#include <QList>

#include "libkwave/Sample.h"
#include "libkwave/undo/UndoAction.h"

namespace Kwave
{

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
	explicit UndoSelection(Kwave::SignalManager &manager);

	/**
	 * Constructor.
	 * @param manager reference to the SignalManager
	 * @param selected_tracks list of selected tracks
	 * @param offset start of the selection
	 * @param length number of selected samples
	 */
	UndoSelection(Kwave::SignalManager &manager,
	              QList<unsigned int> selected_tracks,
	              sample_index_t offset,
	              sample_index_t length);

	/** virtual destructor */
	virtual ~UndoSelection();

	/** @see UndoAction::description() */
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

	/** @see UndoAction::containsModification() */
	bool containsModification() const Q_DECL_OVERRIDE { return false; }

	/** dump, for debugging purposes */
	void dump(const QString &indent) Q_DECL_OVERRIDE;

    private:

	/** reference to the SignalManager is needed in redoSize() */
	Kwave::SignalManager &m_manager;

	/** First selected sample */
	sample_index_t m_offset;

	/** Number of selected samples */
	sample_index_t m_length;

	/** Array with indices of selected tracks. */
	QList<unsigned int> m_selected_tracks;

    };
}

#endif /* UNDO_SELECTION_H */

//***************************************************************************
//***************************************************************************
