/***************************************************************************
    MultiTrackReader.cpp - reader for multi-track signals
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

#include "libkwave/MultiTrackReader.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SignalManager.h"

//***************************************************************************
MultiTrackReader::MultiTrackReader()
    :Kwave::MultiTrackSource<SampleReader, false>(0,0),
     m_first(0), m_last(0), m_canceled(false)
{
}

//***************************************************************************
MultiTrackReader::MultiTrackReader(Kwave::ReaderMode mode,
                                   SignalManager &signal_manager,
                                   const QList<unsigned int> &track_list,
                                   sample_index_t first, sample_index_t last)
    :Kwave::MultiTrackSource<SampleReader, false>(0),
     m_first(first), m_last(last), m_canceled(false)
{
    unsigned int index = 0;

    foreach(unsigned int track, track_list) {
	SampleReader *s = signal_manager.openSampleReader(
	    mode, track, first, last);
	if (!s) break;
	insert(index++, s);
	Q_ASSERT(index == tracks());
    }
}

//***************************************************************************
MultiTrackReader::~MultiTrackReader()
{
    clear();
}

//***************************************************************************
sample_index_t MultiTrackReader::first() const
{
    return m_first;
}

//***************************************************************************
sample_index_t MultiTrackReader::last() const
{
    return m_last;
}

//***************************************************************************
bool MultiTrackReader::eof() const
{
    const unsigned int c = tracks();
    for (unsigned int r = 0; r < c; r++) {
	SampleReader *reader = at(r);
	Q_ASSERT(reader);
	if (!reader) continue;
	if (reader->eof()) return true;
    }
    return false;
}

//***************************************************************************
void MultiTrackReader::proceeded()
{
    qreal sum = 0;
    qreal total = 0;
    unsigned int track;
    const unsigned int n_tracks = tracks();
    for (track = 0; track < n_tracks; ++track) {
	SampleReader *r = at(track);
	if (r) {
	    sum   += (r->pos() - r->first());
	    total += (r->last() - r->first() + 1);
	}
    }

    emit progress(qreal(100.0) * sum / total);
}

//***************************************************************************
void MultiTrackReader::reset()
{
    unsigned int track;
    const unsigned int n_tracks = tracks();
    for (track=0; track < n_tracks; ++track) {
	SampleReader *r = at(track);
	if (r) r->reset();
    }
    emit progress(0);
}

//***************************************************************************
bool MultiTrackReader::insert(unsigned int track, SampleReader *reader)
{
    if (reader) {
        connect(
	    reader, SIGNAL(proceeded()),
	    this, SLOT(proceeded()),
	    Qt::DirectConnection
	);
    }
    return Kwave::MultiTrackSource<SampleReader, false>::insert(
        track, reader);
}

//***************************************************************************
void MultiTrackReader::cancel()
{
    m_canceled = true;
}

//***************************************************************************
using namespace Kwave;
#include "MultiTrackReader.moc"
//***************************************************************************
//***************************************************************************
