/***************************************************************************
    Writer.cpp - base class for writers, providing a C++ stream interface
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

#include <QtCore/QObject>

#include "libkwave/memcpy.h"
#include "libkwave/SampleReader.h"
#include "libkwave/Utils.h"
#include "libkwave/Writer.h"

/** size of m_buffer in samples */
#define BUFFER_SIZE (1024 * 1024)

/** minimum block size used for direct block i/o */
#define MIN_DIRECT_IO_BLOCK_SIZE (BUFFER_SIZE / 2)

//***************************************************************************
Kwave::Writer::Writer()
    :Kwave::SampleSink(0),
     m_first(0), m_last(0), m_mode(Kwave::Insert), m_position(0),
     m_buffer(BUFFER_SIZE), m_buffer_size(BUFFER_SIZE), m_buffer_used(0)
{
}

//***************************************************************************
Kwave::Writer::Writer(Kwave::InsertMode mode,
                      sample_index_t left, sample_index_t right)
    :Kwave::SampleSink(0),
     m_first(left), m_last(right), m_mode(mode), m_position(left),
     m_buffer(BUFFER_SIZE), m_buffer_size(BUFFER_SIZE), m_buffer_used(0)
{
}

//***************************************************************************
Kwave::Writer::~Writer()
{
    // If this assert gets hit, you deleted a writer without calling
    // flush() before. Flushing in the destructor is problematic and
    // might be too late when the derived classes' destructor was
    // already done and signal/slot connections were already released!
    Q_ASSERT(!m_buffer_used);
    if (m_buffer_used) qWarning("Writer was not flushed!?");

    Q_ASSERT((m_mode != Kwave::Overwrite) || (m_position <= m_last + 1));

    // inform others that we proceeded
    emit sigSamplesWritten(m_position - m_first);
}

//***************************************************************************
Kwave::Writer &Kwave::Writer::operator << (const Kwave::SampleArray &samples)
{
    unsigned int count = samples.size();

    if ( (m_buffer_used || (count < MIN_DIRECT_IO_BLOCK_SIZE)) &&
	 (m_buffer_used + count <= m_buffer_size) )
    {
	// append to the internal buffer if it is already in use
	// and if there is still some room,
	// or if the buffer so small that it would make too much overhead
	// to process it by block operation
	MEMCPY(&(m_buffer[m_buffer_used]), &(samples[0]),
	       count * sizeof(sample_t));
	m_buffer_used += count;
	if (m_buffer_used >= m_buffer_size) flush();
    } else {
	// first flush the single-sample buffer before doing block operation
	if (m_buffer_used) flush();

	// now flush the block that we received as parameter (pass-through)
	write(samples, count);
	Q_ASSERT(!count);
    }

    return *this;
}

//***************************************************************************
Kwave::Writer &Kwave::Writer::operator << (const sample_t &sample)
{
    m_buffer[m_buffer_used++] = sample;
    if (m_buffer_used >= m_buffer_size) flush();
    return *this;
}

//***************************************************************************
Kwave::Writer &Kwave::Writer::operator << (Kwave::SampleReader &reader)
{
    if (m_buffer_used) flush();

    // transfer data, using our internal buffer
    unsigned int buflen = m_buffer_size;
    while (!reader.eof() && !eof()) {

	// overwrite mode -> clip at right border
	if (m_mode == Kwave::Overwrite) {
	    if (m_position + buflen - 1 > m_last)
		buflen = Kwave::toUint(m_last - m_position + 1);
	}

	m_buffer_used = reader.read(m_buffer, 0, buflen);
	Q_ASSERT(m_buffer_used);
	if (!m_buffer_used) break;

	if (!flush()) return *this; // out of memory
    }

    // pad the rest with zeroes
    if (m_mode == Kwave::Overwrite) {
	Q_ASSERT(m_position <= m_last + 1);
	while (m_buffer_used + m_position <= m_last) {
	    *this << static_cast<sample_t>(0);
	    m_position++;
	}
	Q_ASSERT(m_position <= m_last + 1);
    }

    return *this;
}

//***************************************************************************
bool Kwave::Writer::eof() const
{
    return (m_mode == Kwave::Overwrite) ? (m_position > m_last) : false;
}

//***************************************************************************
Kwave::Writer &flush(Kwave::Writer &s)
{
    s.flush();
    return s;
}

//***************************************************************************
void Kwave::Writer::input(Kwave::SampleArray data)
{
    if (data.size()) (*this) << data;
}

//***************************************************************************
//***************************************************************************
