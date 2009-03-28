/***************************************************************************
  SampleWriter.cpp  -  stream for inserting samples into a track
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

#include <QApplication>

#include "libkwave/memcpy.h"
#include "libkwave/InsertMode.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SampleWriter.h"
#include "libkwave/Stripe.h"
#include "libkwave/Track.h"

/** size of m_buffer in samples */
#define BUFFER_SIZE (256*1024)

/** minimum time between emitting the "progress()" signal [ms] */
#define MIN_PROGRESS_INTERVAL 500

//***************************************************************************
SampleWriter::SampleWriter(Track &track, InsertMode mode,
    unsigned int left, unsigned int right)
    :Kwave::SampleSink(),
     m_first(left), m_last(right), m_mode(mode), m_track(track),
     m_position(left),
     m_buffer(BUFFER_SIZE), m_buffer_size(BUFFER_SIZE), m_buffer_used(0),
     m_progress_time()
{
    m_track.use();
    m_progress_time.start();
}

//***************************************************************************
SampleWriter::~SampleWriter()
{
    flush();
    m_track.release();
    Q_ASSERT(m_position <= m_last+1);

    // inform others that we proceeded
    emit sigSamplesWritten(m_position - m_first);
}

//***************************************************************************
SampleWriter &SampleWriter::operator << (const Kwave::SampleArray &samples)
{
    unsigned int count = samples.size();

    if (m_buffer_used + count < m_buffer_size) {
	// append to the internal buffer if there is still some room
	MEMCPY(&(m_buffer[m_buffer_used]), &(samples[0]),
	       count * sizeof(sample_t));
	m_buffer_used += count;
	if (m_buffer_used >= m_buffer_size) flush();
    } else {
	// first flush the single-sample buffer before doing block operation
	if (m_buffer_used) flush();

	// now flush the block that we received as parameter (pass-through)
	flush(samples, count);
	Q_ASSERT(!count);
    }

    return *this;
}

//***************************************************************************
SampleWriter &SampleWriter::operator << (const sample_t &sample)
{
    m_buffer[m_buffer_used++] = sample;
    if (m_buffer_used >= m_buffer_size) flush();
    return *this;
}

//***************************************************************************
SampleWriter &SampleWriter::operator << (SampleReader &reader)
{
    if (m_buffer_used) flush();

    // transfer data, using our internal buffer
    unsigned int buflen = m_buffer_size;
    while (!reader.eof() && (m_position <= m_last)) {
	if (m_position+buflen-1 > m_last) buflen = (m_last-m_position)+1;

	m_buffer_used = reader.read(m_buffer, 0, buflen);
	Q_ASSERT(m_buffer_used);
	if (!m_buffer_used) break;

	if (!flush()) return *this; // out of memory
    }

    // pad the rest with zeroes
    Q_ASSERT(m_position <= m_last+1);
    while (m_buffer_used + m_position <= m_last) {
	*this << static_cast<sample_t>(0);
	m_position++;
    }
    Q_ASSERT(m_position <= m_last+1);

    return *this;
}

//***************************************************************************
bool SampleWriter::flush(const Kwave::SampleArray &buffer,
                         unsigned int &count)
{
    if ((m_mode == Overwrite) && (m_position + count > m_last + 1)) {
	// need clipping
	count = m_last + 1 - m_position;
// 	qDebug("SampleWriter::flush() clipped to count=%u", count);
    }

    if (count == 0) return true; // nothing to flush

    Q_ASSERT(count <= buffer.size());
    if (!m_track.writeSamples(m_mode, m_position, buffer, 0, count))
	return false; /* out of memory */

    m_position += count;

    // fix m_last, this might be needed in Append and Insert mode
    if (m_position - 1 > m_last) m_last = m_position - 1;
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
bool SampleWriter::eof() const
{
    return (m_mode == Overwrite) ? (m_position > m_last) : false;
}

//***************************************************************************
SampleWriter &flush(SampleWriter &s)
{
    s.flush();
    return s;
}

//***************************************************************************
void SampleWriter::input(Kwave::SampleArray data)
{
    if (data.size()) (*this) << data;
}

//***************************************************************************
#include "SampleWriter.moc"
//***************************************************************************
//***************************************************************************
