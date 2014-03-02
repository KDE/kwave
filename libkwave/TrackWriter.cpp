/***************************************************************************
        TrackWriter.cpp  -  stream for inserting samples into a track
			     -------------------
    begin                : Feb 11 2001
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

#include <QtGui/QApplication>

#include "libkwave/memcpy.h"
#include "libkwave/InsertMode.h"
#include "libkwave/Track.h"
#include "libkwave/TrackWriter.h"
#include "libkwave/Utils.h"

/** minimum time between emitting the "progress()" signal [ms] */
#define MIN_PROGRESS_INTERVAL 500

//***************************************************************************
Kwave::TrackWriter::TrackWriter(Kwave::Track &track, Kwave::InsertMode mode,
                                sample_index_t left, sample_index_t right)
    :Kwave::Writer(mode, left, right),
     m_track(track), m_progress_time()
{
    m_track.use();
    m_progress_time.start();
}

//***************************************************************************
Kwave::TrackWriter::~TrackWriter()
{
    flush();
    m_track.release();
}

//***************************************************************************
bool Kwave::TrackWriter::write(const Kwave::SampleArray &buffer,
                               unsigned int &count)
{
    if ((m_mode == Kwave::Overwrite) && (m_position + count > m_last + 1)) {
	// need clipping
	count = Kwave::toUint(m_last + 1 - m_position);
// 	qDebug("TrackWriter::write() clipped to count=%u", count);
    }

    if (count == 0) return true; // nothing to write

    Q_ASSERT(count <= buffer.size());
//     qDebug("TrackWriter[%p,%u...%u]::write(%u ... %u) (total=%u)",
// 	   static_cast<void *>(this),
// 	   m_first, m_last,
// 	   m_position, m_position + count - 1,
// 	   m_position + count - m_first);

    if (!m_track.writeSamples(m_mode, m_position, buffer, 0, count))
	return false; /* out of memory */

    m_position += count;

    // fix m_last, this might be needed in Append and Insert mode
    Q_ASSERT(m_position >= 1);
    if ((m_mode == Kwave::Append) || (m_mode == Kwave::Insert)) {
	if (m_position - 1 > m_last) m_last = m_position - 1;
    }
    count = 0;

    // inform others that we proceeded
    if (m_progress_time.elapsed() > MIN_PROGRESS_INTERVAL) {
	m_progress_time.restart();
	emit proceeded();
	QApplication::sendPostedEvents();
    }

    return true;
}

//***************************************************************************
#include "TrackWriter.moc"
//***************************************************************************
//***************************************************************************
