/*************************************************************************
         SampleFIFO.cpp  -  simple FIFO, tuned for sample_t
                             -------------------
    begin                : Sun Apr 11 2004
    copyright            : (C) 2004 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de
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
#include <string.h> // for memcpy
#include "libkwave/SampleFIFO.h"

//***************************************************************************
SampleFIFO::SampleFIFO(unsigned int size)
    :m_size(0), m_buffer(), m_written(0), m_write_pos(0)
{
    if (size) resize(size);
}

//***************************************************************************
SampleFIFO::~SampleFIFO()
{
    resize(0);
}

//***************************************************************************
void SampleFIFO::resize(unsigned int size)
{
    m_buffer.resize(size);
    m_size      = m_buffer.size();
    m_written   = 0;
    m_write_pos = 0;
}

//***************************************************************************
bool SampleFIFO::put(const QMemArray<sample_t> &source)
{
    const unsigned int size = source.size();

    if (!size) return true;
    Q_ASSERT(m_write_pos < m_size);

    unsigned int rest = size;
    unsigned int to_the_end = m_size - m_write_pos;
    unsigned int offset = 0;

    if (rest > to_the_end) {
	// copy only up to the end
	memcpy(m_buffer.data() + m_write_pos,
	       source.data(),
	       to_the_end * sizeof(sample_t));

	rest        -= to_the_end;
	offset      += to_the_end;
	m_written   += to_the_end;
	Q_ASSERT(m_write_pos + to_the_end == m_size);
	m_write_pos = 0;
    }

    if (rest) {
	// copy the whole (remaining) source
	memcpy(m_buffer.data() + m_write_pos,
	       source.data() + offset,
	       rest * sizeof(sample_t));

	m_write_pos += rest;
	m_written   += rest;
    }
    Q_ASSERT(m_write_pos <= m_size);
    if (m_write_pos >= m_size) m_write_pos = 0;
    if (m_written > m_size) m_written = m_size;

    return true;
}

//***************************************************************************
void SampleFIFO::align()
{
    if (m_written < m_size) {
	// shrink the buffer to contain only used samples
	m_buffer.resize(m_written);
	m_size = m_buffer.size();
    } else if (m_written != m_size) {
	// we have to do more: rotate the buffer content
	// so that it starts at zero, like this:
	// [0 ... wp-1, wp .... size-1 ] ->  [wp ... size-1, 0 ... wp-1 ]
	unsigned int x1 = 0;
	unsigned int x2 = m_write_pos;
	const unsigned int n = m_size - 2 + ((m_size + x2) & 1);
	Q_ASSERT(m_size >= 2);

	sample_t *buf = m_buffer.data();
	for (x1=0; x1 < n; ++x1)
	{
	    sample_t s = buf[x1];
	    buf[x1] = buf[x2];
	    buf[x2] = s;
	    if (x2 < m_size-1) x2++;
	}
    }
}

//***************************************************************************
unsigned int SampleFIFO::length()
{
    return m_written;
}

//***************************************************************************
QMemArray<sample_t> &SampleFIFO::data()
{
    return m_buffer;
}

//***************************************************************************
//***************************************************************************
