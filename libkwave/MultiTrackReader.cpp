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
Kwave::MultiTrackReader::MultiTrackReader()
:Kwave::MultiTrackSource<Kwave::SampleReader, false>(0, Q_NULLPTR),
     m_first(0), m_last(0)
{
}

//***************************************************************************
Kwave::MultiTrackReader::MultiTrackReader(
    Kwave::ReaderMode mode,
    Kwave::SignalManager &signal_manager,
    const QVector<unsigned int> &track_list,
    sample_index_t first,
    sample_index_t last)
    :Kwave::MultiTrackSource<Kwave::SampleReader, false>(0, Q_NULLPTR),
     m_first(first), m_last(last)
{
    unsigned int index = 0;

    foreach(unsigned int track, track_list) {
        Kwave::SampleReader *s = signal_manager.openReader(
            mode, track, first, last);
        if (!s) break;
        insert(index++, s);
        Q_ASSERT(index == tracks());
    }
}

//***************************************************************************
Kwave::MultiTrackReader::~MultiTrackReader()
{
    clear();
}

//***************************************************************************
sample_index_t Kwave::MultiTrackReader::first() const
{
    return m_first;
}

//***************************************************************************
sample_index_t Kwave::MultiTrackReader::last() const
{
    return m_last;
}

//***************************************************************************
bool Kwave::MultiTrackReader::eof() const
{
    const unsigned int c = tracks();
    for (unsigned int r = 0; r < c; r++) {
        Kwave::SampleReader *reader = at(r);
        Q_ASSERT(reader);
        if (!reader) continue;
        if (!reader->eof()) return false;
    }
    return true;
}

//***************************************************************************
void Kwave::MultiTrackReader::proceeded()
{
    qreal sum = 0;
    qreal total = 0;
    unsigned int track;
    const unsigned int n_tracks = tracks();
    for (track = 0; track < n_tracks; ++track) {
        Kwave::SampleReader *r = at(track);
        if (r) {
            sum   += static_cast<qreal>(r->pos()  - r->first());
            total += static_cast<qreal>(r->last() - r->first() + 1);
        }
    }

    emit progress(qreal(100.0) * sum / total);
}

//***************************************************************************
void Kwave::MultiTrackReader::reset()
{
    unsigned int track;
    const unsigned int n_tracks = tracks();
    for (track=0; track < n_tracks; ++track) {
        Kwave::SampleReader *r = at(track);
        if (r) r->reset();
    }
    emit progress(0);
}

//***************************************************************************
bool Kwave::MultiTrackReader::insert(unsigned int track,
                                     Kwave::SampleReader *reader)
{
    if (reader) {
        connect(
            reader, SIGNAL(proceeded()),
            this, SLOT(proceeded()),
            Qt::DirectConnection
        );
    }
    return Kwave::MultiTrackSource<Kwave::SampleReader, false>::insert(
        track, reader);
}

//***************************************************************************
void Kwave::MultiTrackReader::skip(sample_index_t count)
{
    unsigned int track;
    const unsigned int n_tracks = tracks();
    for (track=0; track < n_tracks; ++track) {
        Kwave::SampleReader *r = at(track);
        if (r) r->skip(count);
    }
}

//***************************************************************************
void Kwave::MultiTrackReader::seek(sample_index_t pos)
{
    unsigned int track;
    const unsigned int n_tracks = tracks();
    for (track=0; track < n_tracks; ++track) {
        Kwave::SampleReader *r = at(track);
        if (r) r->seek(pos);
    }
}

//***************************************************************************
//***************************************************************************

#include "moc_MultiTrackReader.cpp"
