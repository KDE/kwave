/***************************************************************************
         MultiWriter.cpp - writer for multi-track processing
                             -------------------
    begin                : Sun Aug 23 2009
    copyright            : (C) 2009 by Thomas Eschenbacher
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

#include "libkwave/MultiWriter.h"

//***************************************************************************
Kwave::MultiWriter::MultiWriter()
    :Kwave::MultiTrackSink<Kwave::Writer, false>(0, nullptr)
{
}

//***************************************************************************
Kwave::MultiWriter::~MultiWriter()
{
    clear();
}

//***************************************************************************
bool Kwave::MultiWriter::insert(unsigned int track, Kwave::Writer *writer)
{
    if (writer) {
        connect(
            writer, SIGNAL(proceeded()),
            this, SLOT(proceeded()),
            Qt::DirectConnection
        );
    }
    return Kwave::MultiTrackSink<Kwave::Writer, false>::insert(track, writer);
}

//***************************************************************************
void Kwave::MultiWriter::proceeded()
{
    bool overwrite = tracks() && at(0) && (at(0)->mode() == Kwave::Overwrite);
    if (overwrite) {
        qreal sum   = 0;
        qreal total = 0;
        unsigned int track;
        const unsigned int tracks = this->tracks();
        for (track = 0; track < tracks; ++track) {
            const Kwave::Writer *w = at(track);
            if (w) {
                sum   += static_cast<qreal>(w->position() - w->first());
                total += static_cast<qreal>(w->last()     - w->first());
            }
        }
        emit progress(qreal(100.0) * sum / total);
    } else {
        quint64 sum   = 0;
        unsigned int track;
        const unsigned int tracks = this->tracks();
        for (track = 0; track < tracks; ++track) {
            const Kwave::Writer *w = at(track);
            if (w) sum += w->position() - w->first();
        }
        emit written(sum);
    }
}

//***************************************************************************
sample_index_t Kwave::MultiWriter::last() const
{
    sample_index_t last = 0;
    const unsigned int tracks = this->tracks();
    for (unsigned int track = 0; track < tracks; ++track) {
        const Kwave::Writer *w = at(track);
        if (w && w->last() > last) last = w->last();
    }
    return last;
}

//***************************************************************************
void Kwave::MultiWriter::clear()
{
    Kwave::MultiTrackSink<Kwave::Writer, false>::clear();
}

//***************************************************************************
void Kwave::MultiWriter::flush()
{
    const unsigned int tracks = this->tracks();
    for (unsigned int track = 0; track < tracks; ++track) {
        Kwave::Writer *w = (*this)[track];
        if (w) w->flush();
    }
}

//***************************************************************************
//***************************************************************************

#include "moc_MultiWriter.cpp"
