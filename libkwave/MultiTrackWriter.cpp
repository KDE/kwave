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

#include "kwave/SignalManager.h"
#include "kwave/UndoTransactionGuard.h"

//***************************************************************************
MultiTrackWriter::MultiTrackWriter()
    :Kwave::MultiTrackSink<SampleWriter>(0,0), m_cancelled(false)
{
}

//***************************************************************************
MultiTrackWriter::MultiTrackWriter(SignalManager &signal_manager,
    const QList<unsigned int> &track_list, InsertMode mode,
    unsigned int left, unsigned int right)
    :Kwave::MultiTrackSink<SampleWriter>(track_list.count()),
     m_cancelled(false)
{
    UndoTransactionGuard guard(signal_manager, 0);

    unsigned int index = 0;
    foreach (unsigned int track, track_list) {
	// NOTE: this function is *nearly* identical to the one in the
	//       Signal class, except for undo support
	SampleWriter *s = signal_manager.openSampleWriter(
	    track, mode, left, right, true);
	Q_ASSERT(s);
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
    :Kwave::MultiTrackSink<SampleWriter>(0,0), m_cancelled(false)
{
    QList<unsigned int> tracks = signal_manager.selectedTracks();
    unsigned int left = 0;
    unsigned int right = 0;

    if (signal_manager.length()) {
	// default if signal is present: current selection
	left  = signal_manager.selection().first();
	right = signal_manager.selection().last();
	if (left == right) {
	    // if no selection: whole signal
	    left  = 0;
	    right = signal_manager.length();
	}
    }
    MultiTrackWriter(signal_manager, tracks, mode, left, right);
}

//***************************************************************************
MultiTrackWriter::~MultiTrackWriter()
{
    clear();
}

//***************************************************************************
MultiTrackWriter &MultiTrackWriter::operator << (MultiTrackReader &source)
{
    unsigned int src_tracks = source.tracks();
    unsigned int dst_tracks = tracks();

    Q_ASSERT(src_tracks);
    Q_ASSERT(dst_tracks);
    if (!src_tracks || !dst_tracks) return *this;

    if (src_tracks != dst_tracks) {
	// create a mixer matrix and pass everything through

	// ### ALPHA: process sample per sample          ###
	// ### still using the same code as in playback  ###

	// create a translation matrix for mixing up/down to the desired
	// number of output channels
	Matrix<double> matrix(src_tracks, dst_tracks);
	unsigned int x, y;
	for (y=0; y < dst_tracks; y++) {
	    unsigned int m1, m2;
	    m1 = y * src_tracks;
	    m2 = (y+1) * src_tracks;

	    for (x=0; x < src_tracks; x++) {
		unsigned int n1, n2;
		n1 = x * dst_tracks;
		n2 = n1 + dst_tracks;

		// get the common area of [n1..n2] and [m1..m2]
		unsigned int l  = qMax(n1, m1);
		unsigned int r = qMin(n2, m2);

		matrix[x][y] = (r > l) ?
		    (double)(r-l) / (double)src_tracks : 0.0;
	    }
	}

	Kwave::SampleArray in_samples(src_tracks);
	Kwave::SampleArray out_samples(dst_tracks);

	while (!(source.eof())) {
	    // read input vector
	    unsigned int x;
	    for (x=0; x < src_tracks; x++) {
		in_samples[x] = 0;
		SampleReader *stream = source[x];
		Q_ASSERT(stream);
		if (!stream) continue;

		sample_t act;
		(*stream) >> act;
		in_samples[x] = act;
	    }

	    // multiply matrix with input to get output
	    unsigned int y;
	    for (y=0; y < dst_tracks; y++) {
		double sum = 0;
		for (x=0; x < src_tracks; x++) {
		    sum += (double)in_samples[x] * matrix[x][y];
		}
		out_samples[y] = (sample_t)sum;
	    }

	    // write samples to the target stream
	    for (y = 0; y < dst_tracks; y++) {
		if (m_cancelled) break;
		*at(y) << out_samples[y];
	    }

	}

    } else {
	// process 1:1
	unsigned int track;
	for (track = 0; track < src_tracks; ++track) {
	    *at(track) << *(source[track]);
	    if (m_cancelled) break;
	}
    }

    return *this;
}

//***************************************************************************
bool MultiTrackWriter::insert(unsigned int track, SampleWriter *writer)
{
    if (writer) {
        connect(writer, SIGNAL(proceeded()), this, SLOT(proceeded()));
    }
    return Kwave::MultiTrackSink<SampleWriter>::insert(track, writer);
}

//***************************************************************************
void MultiTrackWriter::proceeded()
{
    unsigned int pos = 0;
    unsigned int track;
    for (track=0; track < tracks(); ++track) {
	SampleWriter *w = at(track);
	if (w) pos += (w->position() - w->first());
    }
    emit progress(pos);
}

//***************************************************************************
void MultiTrackWriter::cancel()
{
    m_cancelled = true;
}

//***************************************************************************
unsigned int MultiTrackWriter::last() const
{
    unsigned int last = 0;
    const unsigned int tracks = this->tracks();
    for (unsigned int track=0; track < tracks; ++track) {
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
    for (unsigned int track=0; track < tracks; ++track) {
	SampleWriter *w = (*this)[track];
	if (w) w->flush();
    }
}

//***************************************************************************
using namespace Kwave;
#include "MultiTrackWriter.moc"
//***************************************************************************
//***************************************************************************
