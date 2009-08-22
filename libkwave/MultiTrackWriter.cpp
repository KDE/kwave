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
#include "libkwave/Matrix.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SampleWriter.h"
#include "libkwave/SignalManager.h"
#include "libkwave/undo/UndoTransactionGuard.h"

//***************************************************************************
MultiTrackWriter::MultiTrackWriter()
    :Kwave::MultiTrackSink<SampleWriter>(0,0), m_canceled(false)
{
}

//***************************************************************************
MultiTrackWriter::MultiTrackWriter(SignalManager &signal_manager,
    const QList<unsigned int> &track_list, InsertMode mode,
    unsigned int left, unsigned int right)
    :Kwave::MultiTrackSink<SampleWriter>(0),
     m_canceled(false)
{
    UndoTransactionGuard guard(signal_manager, 0);

    unsigned int index = 0;
    foreach (unsigned int track, track_list) {
	// NOTE: this function is *nearly* identical to the one in the
	//       Signal class, except for undo support
	SampleWriter *s = signal_manager.openSampleWriter(
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
MultiTrackWriter::MultiTrackWriter(SignalManager &signal_manager,
    InsertMode mode)
    :Kwave::MultiTrackSink<SampleWriter>(0,0), m_canceled(false)
{
    UndoTransactionGuard guard(signal_manager, 0);

    QList<unsigned int> track_list = signal_manager.selectedTracks();
    unsigned int left = 0;
    unsigned int right = 0;

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
	SampleWriter *s = signal_manager.openSampleWriter(
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
MultiTrackWriter::~MultiTrackWriter()
{
    clear();
}

//***************************************************************************
bool MultiTrackWriter::insert(unsigned int track, SampleWriter *writer)
{
    if (writer) {
	connect(
	    writer, SIGNAL(proceeded()),
	    this, SLOT(proceeded()),
	    Qt::DirectConnection
	);
    }
    return Kwave::MultiTrackSink<SampleWriter>::insert(track, writer);
}

//***************************************************************************
void MultiTrackWriter::proceeded()
{
    unsigned int pos = 0;
    unsigned int track;
    const unsigned int tracks = this->tracks();
    for (track = 0; track < tracks; ++track) {
	SampleWriter *w = at(track);
	if (w) pos += (w->position() - w->first());
    }
    emit progress(pos);
}

//***************************************************************************
void MultiTrackWriter::cancel()
{
    m_canceled = true;
}

//***************************************************************************
unsigned int MultiTrackWriter::last() const
{
    unsigned int last = 0;
    const unsigned int tracks = this->tracks();
    for (unsigned int track = 0; track < tracks; ++track) {
	const SampleWriter *w = at(track);
	if (w && w->last() > last) last = w->last();
    }
    return last;
}

//***************************************************************************
void MultiTrackWriter::clear()
{
    flush();
    Kwave::MultiTrackSink<SampleWriter>::clear();
}

//***************************************************************************
void MultiTrackWriter::flush()
{
    const unsigned int tracks = this->tracks();
    for (unsigned int track = 0; track < tracks; ++track) {
	SampleWriter *w = (*this)[track];
	if (w) w->flush();
    }
}

//***************************************************************************
using namespace Kwave;
#include "MultiTrackWriter.moc"
//***************************************************************************
//***************************************************************************
