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
#include "libkwave/Sample.h"
#include "libkwave/SampleFIFO.h"
#include "libkwave/memcpy.h"

//***************************************************************************
SampleFIFO::SampleFIFO()
    :m_buffer(), m_read_offset(0), m_lock(true)
{
}

//***************************************************************************
SampleFIFO::~SampleFIFO()
{
    QMutexLocker _lock(&m_lock);
    flush();
}

//***************************************************************************
void SampleFIFO::flush()
{
    QMutexLocker _lock(&m_lock);

    m_buffer.clear();
    m_read_offset = 0;
}

//***************************************************************************
void SampleFIFO::put(const Kwave::SampleArray &buffer)
{
    QMutexLocker _lock(&m_lock);
    if (!buffer.isEmpty()) m_buffer.append(buffer.copy());
}

//***************************************************************************
unsigned int SampleFIFO::get(Kwave::SampleArray &buffer)
{
    QMutexLocker _lock(&m_lock);

    if (m_buffer.isEmpty()) return 0;

    unsigned int rest = buffer.size();
    const unsigned int available = length();
    if (rest > available) rest = available;

    QValueVector<Kwave::SampleArray>::iterator it = m_buffer.begin();
    sample_t *dst = buffer.data();
    unsigned int read = 0;
    while (rest && (it != m_buffer.end())) {
	sample_t *src        = (*it).data();
	unsigned int src_len = (*it).count();
	Q_ASSERT(src_len > m_read_offset);
	if (m_read_offset) src_len -= m_read_offset;

	if (src_len <= rest) {
	    // use the whole buffer up to it's end
	    MEMCPY(dst, src + m_read_offset, src_len * sizeof(sample_t));
	    rest  -= src_len;
	    read  += src_len;
	    dst   += src_len;
	    m_read_offset = 0;
	    // remove the buffer from the queue
	    it = m_buffer.erase(it);
	} else {
	    // use only a portion of the buffer
	    MEMCPY(dst, src + m_read_offset, rest * sizeof(sample_t));
	    read          += rest;
	    m_read_offset += rest;
	    Q_ASSERT(m_read_offset < (*it).count());
	    rest = 0;
	}
    }

    return read;
}

//***************************************************************************
unsigned int SampleFIFO::length()
{
    QMutexLocker _lock(&m_lock);

    unsigned int len = 0;
    QValueVector<Kwave::SampleArray>::iterator it;
    for (it = m_buffer.begin(); it != m_buffer.end(); ++it)
	len += (*it).count();
    return len;
}

//***************************************************************************
//***************************************************************************
