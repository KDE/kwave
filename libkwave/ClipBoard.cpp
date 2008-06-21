/***************************************************************************
          ClipBoard.cpp  -  the Kwave clipboard
			     -------------------
    begin                : Tue Jun 26, 2001
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

#include <QList>
#include <QReadLocker>
#include <QWriteLocker>

#include "libkwave/ClipBoard.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SampleWriter.h"
#include "libkwave/Signal.h"
#include "libkwave/Track.h"

/** static instance of Kwave's clipboard */
static ClipBoard g_clipboard;

//***************************************************************************
ClipBoard &ClipBoard::instance()
{
    return g_clipboard;
}

//***************************************************************************
ClipBoard::ClipBoard()
    :m_lock(), m_rate(0), m_buffer()
{
}

//***************************************************************************
ClipBoard::~ClipBoard()
{
    // clear() must have been before, e.g. in the application's destructor !
}

//***************************************************************************
void ClipBoard::copy(Signal &signal,
                     const QList<unsigned int> &track_list,
                     unsigned int offset, unsigned int length,
                     double rate)
{
    QWriteLocker lock(&m_lock); // lock exclusive

    // first get rid of the previous content
    clear();

    // remember the sample rate
    m_rate = rate;

    // break if nothing to do
    if ((!length) || (!track_list.count())) return;

    // allocate buffers for the signals and fill them with data
    foreach (unsigned int i, track_list) {
	Track *t = new Track(length);
	Q_ASSERT(t);
	if (!t) continue;

	// transfer with sample reader and writer
	SampleWriter *writer = t->openSampleWriter(Overwrite, 0, length-1);
	Q_ASSERT(writer);
	if (!writer) continue;

	SampleReader *reader = signal.openSampleReader(track_list[i],
	                       offset, offset+length-1);
	Q_ASSERT(reader);
	if (reader) {
	    // transfer the samples from the source track out buffer track
	    *writer << *reader;
	    delete reader;
	}
	delete writer;

	// append the track to the buffer
	m_buffer.append(t);
    }
}

//***************************************************************************
void ClipBoard::paste(MultiTrackWriter &writers)
{
    QReadLocker lock(&m_lock); // lock read-only

    Q_ASSERT(length());
    if (!length()) return; // clipboard is empty ?

    unsigned int tracks = m_buffer.count();
    Q_ASSERT(tracks == writers.tracks());
    if (tracks != writers.tracks()) return; // track count mismatch

    unsigned int i=0;
    foreach (Track *track, m_buffer) {
	Q_ASSERT(track);
	if (!track) continue;

	SampleReader *reader = track->openSampleReader(0, length()-1);
	SampleWriter *writer = writers[i++];
	Q_ASSERT(reader);
	Q_ASSERT(writer);
	if (reader && writer) *writer << *reader;
    }
}

//***************************************************************************
void ClipBoard::clear()
{
    QWriteLocker lock(&m_lock); // lock exclusive

    foreach (Track *track, m_buffer)
	if (track) delete track;
    m_buffer.clear();
    m_rate = 0;
}

//***************************************************************************
unsigned int ClipBoard::length()
{
    QReadLocker lock(&m_lock); // lock read-only

    unsigned int count = m_buffer.count();
    if (!count) return 0;

    unsigned int max_len = 0;
    foreach (Track *track, m_buffer) {
	if (!track) continue;
	unsigned int len = track->length();
	if (len > max_len) max_len = len;
    }

    return max_len;
}

//***************************************************************************
double ClipBoard::rate()
{
    QReadLocker lock(&m_lock); // lock read-only
    return m_rate;
}

//***************************************************************************
bool ClipBoard::isEmpty()
{
    return (tracks() == 0);
}

//***************************************************************************
unsigned int ClipBoard::tracks()
{
    QReadLocker lock(&m_lock); // lock read-only
    return m_buffer.count();
}

//***************************************************************************
//***************************************************************************
