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

#include "libkwave/MultiTrackReader.h"
#include "libkwave/SampleReader.h"
#include "kwave/SignalManager.h"

//***************************************************************************
MultiTrackReader::MultiTrackReader()
    :QObject(), QPtrVector<SampleReader>(), m_cancelled(false)
{
    setAutoDelete(true);
}

//***************************************************************************
MultiTrackReader::MultiTrackReader(SignalManager &signal_manager,
    const QMemArray<unsigned int> &track_list,
    unsigned int first, unsigned int last)
    :QObject(),
     QPtrVector<SampleReader>(),
     m_cancelled(false)
{
    unsigned int count = track_list.count();
    resize(count);

    for (unsigned int i=0; i < count; i++) {
	unsigned int track = track_list[i];
	SampleReader *s =
	    signal_manager.openSampleReader(track, first, last);
	Q_ASSERT(s);
	insert(i, s);
    }

    Q_ASSERT(count == tracks());
}

//***************************************************************************
MultiTrackReader::~MultiTrackReader()
{
    clear();
}

//***************************************************************************
bool MultiTrackReader::eof() const
{
    unsigned int c = this->count();
    for (unsigned int r = 0; r < c; r++) {
	SampleReader *reader = this->at(r);
	Q_ASSERT(reader);
	if (!reader) continue;
	if (reader->eof()) return true;
    }
    return false;
}

//***************************************************************************
bool MultiTrackReader::insert(unsigned int track, const SampleReader *reader)
{
    if (reader) {
        connect(reader, SIGNAL(proceeded()), this, SLOT(proceeded()));
    }
    return QPtrVector<SampleReader>::insert(track, reader);
}

//***************************************************************************
void MultiTrackReader::proceeded()
{
    unsigned int pos = 0;
    unsigned int track;
    const unsigned int tracks = count();
    for (track=0; track < tracks; ++track) {
	SampleReader *r = (*this)[track];
	if (r) pos += (r->pos() - r->first());
    }
    emit progress(pos);
}

//***************************************************************************
void MultiTrackReader::reset()
{
    unsigned int pos = 0;
    unsigned int track;
    const unsigned int tracks = count();
    for (track=0; track < tracks; ++track) {
	SampleReader *r = (*this)[track];
	if (r) r->reset();
    }
    emit progress(pos);
}

//***************************************************************************
void MultiTrackReader::clear()
{
    setAutoDelete(false);
    while (!isEmpty()) {
	unsigned int last = count()-1;
	SampleReader *reader = at(last);
	remove(last);
	resize(last);
	if (reader) delete reader;
    }
}

//***************************************************************************
void MultiTrackReader::cancel()
{
    m_cancelled = true;
}

//***************************************************************************
#include "MultiTrackReader.moc"
//***************************************************************************
//***************************************************************************
