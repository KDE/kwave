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

#include "mt/SharedLock.h"
#include "mt/SharedLockGuard.h"

#include "libkwave/MultiTrackWriter.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SampleWriter.h"
#include "libkwave/Signal.h"
#include "libkwave/Track.h"

#include "ClipBoard.h"

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
                     const QMemArray<unsigned int> &track_list,
                     unsigned int offset, unsigned int length,
                     double rate)
{
    SharedLockGuard lock(m_lock, true); // lock exclusive

    // first get rid of the previous content
    m_buffer.setAutoDelete(true);
    m_buffer.clear();

    // remember the sample rate
    m_rate = rate;

    // break if nothing to do
    if ((!length) || (!track_list.count())) return;

    // allocate buffers for the signals and fill them with data
    unsigned int i;
    for (i = 0; i < track_list.count(); i++) {
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
    SharedLockGuard lock(m_lock, false); // lock read-only

    Q_ASSERT(length());
    if (!length()) return; // clipboard is empty ?

    unsigned int tracks = m_buffer.count();
    Q_ASSERT(tracks == writers.tracks());
    if (tracks != writers.tracks()) return; // track count mismatch

    QPtrListIterator<Track> it(m_buffer);
    unsigned int i = 0;
    for ( ; it.current(); ++it ) {
	Track *track = it.current();
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
    SharedLockGuard lock(m_lock, true); // lock exclusive

    m_buffer.setAutoDelete(true);
    m_buffer.clear();
    m_rate = 0;
}

//***************************************************************************
unsigned int ClipBoard::length()
{
    SharedLockGuard lock(m_lock, false); // lock read-only

    unsigned int count = m_buffer.count();
    if (!count) return 0;

    QPtrListIterator<Track> it(m_buffer);
    unsigned int max_len = 0;
    for (; it.current(); ++it) {
	Track *track = it.current();
	Q_ASSERT(track);
	if (!track) continue;

	unsigned int len = track->length();
	if (len > max_len) max_len = len;
    }

    return max_len;
}

//***************************************************************************
double ClipBoard::rate()
{
    SharedLockGuard lock(m_lock, false); // lock read-only
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
    SharedLockGuard lock(m_lock, false); // lock read-only
    return m_buffer.count();
}

//***************************************************************************
//***************************************************************************
