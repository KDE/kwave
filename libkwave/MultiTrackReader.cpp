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
     m_canceled(false)
{
}

//***************************************************************************
MultiTrackReader::MultiTrackReader(SignalManager &signal_manager,
                                   const QList<unsigned int> &track_list,
                                   unsigned int first, unsigned int last)
    :Kwave::MultiTrackSource<SampleReader, false>(0),
     m_canceled(false)
{
    const unsigned int count = track_list.count();
    unsigned int index = 0;

    foreach(unsigned int track, track_list) {
	SampleReader *s = signal_manager.openSampleReader(track, first, last);
	Q_ASSERT(s);
	insert(index++, s);
	Q_ASSERT(index == tracks());
    }

    Q_ASSERT(count == tracks());
    Q_UNUSED(count);
}

//***************************************************************************
MultiTrackReader::~MultiTrackReader()
{
    clear();
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
    unsigned int pos = 0;
    unsigned int track;
    const unsigned int n_tracks = tracks();
    for (track=0; track < n_tracks; ++track) {
	SampleReader *r = at(track);
	if (r) pos += (r->pos() - r->first());
    }
    emit progress(pos);
}

//***************************************************************************
void MultiTrackReader::reset()
{
    unsigned int pos = 0;
    unsigned int track;
    const unsigned int n_tracks = tracks();
    for (track=0; track < n_tracks; ++track) {
	SampleReader *r = at(track);
	if (r) r->reset();
    }
    emit progress(pos);
}

//***************************************************************************
bool MultiTrackReader::insert(unsigned int track, SampleReader *reader)
{
    if (reader) {
        connect(reader, SIGNAL(proceeded()), this, SLOT(proceeded()));
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
