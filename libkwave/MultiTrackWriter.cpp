/***************************************************************************
    MultiTrackWriter.cpp - writer for multi-track signals
			     -------------------
    begin                : Sat Jun 30 2001
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

#include "libkwave/KwaveSampleArray.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/SignalManager.h"
#include "libkwave/undo/UndoTransactionGuard.h"

//***************************************************************************
Kwave::MultiTrackWriter::MultiTrackWriter()
    :Kwave::MultiWriter()
{
}

//***************************************************************************
Kwave::MultiTrackWriter::MultiTrackWriter(SignalManager &signal_manager,
                                          const QList<unsigned int> &track_list,
                                          InsertMode mode,
                                          sample_index_t left,
                                          sample_index_t right)
    :Kwave::MultiWriter()
{
    UndoTransactionGuard guard(signal_manager, 0);

    unsigned int index = 0;
    foreach (unsigned int track, track_list) {
	// NOTE: this function is *nearly* identical to the one in the
	//       Signal class, except for undo support
	Kwave::Writer *s = signal_manager.openWriter(
	    track, mode, left, right, true);
	if (s) {
	    insert(index++, s);
	} else {
	    // out of memory or aborted
	    qWarning("MultiTrackWriter constructor: "\
	             "out of memory or aborted");
	    clear();
	    break;
	}
    }
}

//***************************************************************************
Kwave::MultiTrackWriter::MultiTrackWriter(SignalManager &signal_manager,
                                          InsertMode mode)
    :Kwave::MultiWriter()
{
    UndoTransactionGuard guard(signal_manager, 0);

    QList<unsigned int> track_list = signal_manager.selectedTracks();
    sample_index_t left = 0;
    sample_index_t right = 0;

    if (signal_manager.length()) {
	// default if signal is present: current selection
	left  = signal_manager.selection().first();
	right = signal_manager.selection().last();
	if (left == right) {
	    // if no selection: whole signal
	    left  = 0;
	    right = signal_manager.length() - 1;
	}
    }

    unsigned int index = 0;
    foreach (unsigned int track, track_list) {
	// NOTE: this function is *nearly* identical to the one in the
	//       Signal class, except for undo support
	Kwave::Writer *s = signal_manager.openWriter(
	    track, mode, left, right, true);
	if (s) {
	    insert(index++, s);
	} else {
	    // out of memory or aborted
	    qWarning("MultiTrackWriter constructor: "\
	             "out of memory or aborted");
	    clear();
	    break;
	}
    }

}

//***************************************************************************
Kwave::MultiTrackWriter::~MultiTrackWriter()
{
    flush();
    clear();
}

//***************************************************************************
#include "MultiTrackWriter.moc"
//***************************************************************************
//***************************************************************************
