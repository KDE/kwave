/***************************************************************************
     UndoModifyAction.h  -  UndoAction for modifications on samples
			     -------------------
    begin                : May 25 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "config.h"
#include <KLocalizedString>

#include "libkwave/Sample.h"
#include "libkwave/SampleArray.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SignalManager.h"
#include "libkwave/Writer.h"
#include "libkwave/undo/UndoModifyAction.h"

//***************************************************************************
Kwave::UndoModifyAction::UndoModifyAction(unsigned int track,
                                          sample_index_t offset,
                                          sample_index_t length)
    :UndoAction(), m_track(track), m_offset(offset), m_length(length),
     m_stripes()
{
}

//***************************************************************************
Kwave::UndoModifyAction::~UndoModifyAction()
{
}

//***************************************************************************
QString Kwave::UndoModifyAction::description()
{
    return i18n("Modify Samples");
}

//***************************************************************************
qint64 Kwave::UndoModifyAction::undoSize()
{
    return sizeof(*this) + (m_length * sizeof(sample_t));
}

//***************************************************************************
bool Kwave::UndoModifyAction::store(Kwave::SignalManager &manager)
{
    if (!m_length) return true; // shortcut: this is an empty action

    // fork off a multi track stripe list for the selected range
    QList<unsigned int> track_list;
    track_list.append(m_track);
    const sample_index_t left  = m_offset;
    const sample_index_t right = m_offset + m_length - 1;
    m_stripes = manager.stripes(track_list, left, right);
    if (m_stripes.isEmpty())
	return false; // retrieving the stripes failed

    return true;
}

//***************************************************************************
Kwave::UndoAction *Kwave::UndoModifyAction::undo(
    Kwave::SignalManager &manager, bool with_redo)
{
    const sample_index_t left  = m_offset;
    const sample_index_t right = m_offset + m_length - 1;
    QList<Kwave::Stripe::List> redo_data;
    QList<unsigned int> track_list;
    track_list.append(m_track);
    bool ok = true;

    if (m_length && with_redo) {
	// save the current stripes for later redo
	redo_data = manager.stripes(track_list, left, right);
	ok &= !redo_data.isEmpty();
    }

    // merge the stripes back into the signal
    if (m_length && ok) {
	if (!manager.mergeStripes(m_stripes, track_list)) {
	    qWarning("UndoModifyAction::undo() FAILED [mergeStripes]");
	    return 0;
	}
    }

    if (ok && with_redo) {
	// now store the redo data in this object
	m_stripes = redo_data;
    }

    return (with_redo && ok) ? this : 0;
}

//***************************************************************************
//***************************************************************************
