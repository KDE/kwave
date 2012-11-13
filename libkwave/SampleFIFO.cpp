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
Kwave::SampleFIFO::SampleFIFO()
    :m_buffer(), m_size(0), m_read_offset(0), m_lock(QMutex::Recursive)
{
}

//***************************************************************************
Kwave::SampleFIFO::SampleFIFO(const Kwave::SampleFIFO &other)
    :m_buffer(other.m_buffer), m_size(other.m_size),
     m_read_offset(other.m_read_offset),
     m_lock(QMutex::Recursive)
{
}

//***************************************************************************
Kwave::SampleFIFO::~SampleFIFO()
{
    QMutexLocker _lock(&m_lock);
    flush();
}

//***************************************************************************
void Kwave::SampleFIFO::flush()
{
    QMutexLocker _lock(&m_lock);

    m_buffer.clear();
    m_read_offset = 0;
}

//***************************************************************************
void Kwave::SampleFIFO::put(const Kwave::SampleArray &buffer)
{
    if (buffer.isEmpty()) return;
    QMutexLocker _lock(&m_lock);

    // always enqueue the new buffer
    m_buffer.enqueue(buffer);

    if (!m_size) return; // no limit set

    // crop away whole unneeded buffers
    while ((unlockedLength() - m_buffer.head().size()) > m_size)
	m_buffer.dequeue();
}

//***************************************************************************
unsigned int Kwave::SampleFIFO::get(Kwave::SampleArray &buffer)
{
    QMutexLocker _lock(&m_lock);

    if (m_buffer.isEmpty()) return 0;

    unsigned int rest = buffer.size();
    const unsigned int available = length();
    if (rest > available) rest = available;

    sample_t *dst = buffer.data();
    unsigned int read = 0;
    while (rest && !m_buffer.isEmpty()) {
	Kwave::SampleArray head = m_buffer.head();
	sample_t *src        = head.data();
	unsigned int src_len = head.size();
	Q_ASSERT(src_len > m_read_offset);

	if (m_read_offset + rest >= src_len) {
	    // use the whole buffer up to it's end
	    unsigned int len = src_len - m_read_offset;
	    MEMCPY(dst, src + m_read_offset, len * sizeof(sample_t));
	    rest  -= len;
	    read  += len;
	    dst   += len;
	    m_read_offset = 0;

	    // remove the buffer from the queue
	    m_buffer.dequeue();
	} else {
	    // use only a portion of the buffer
	    MEMCPY(dst, src + m_read_offset, rest * sizeof(sample_t));
	    read          += rest;
	    m_read_offset += rest;
	    Q_ASSERT(m_read_offset < src_len);
	    rest = 0;
	}
    }

    return read;
}

//***************************************************************************
unsigned int Kwave::SampleFIFO::unlockedLength()
{
    unsigned int len = 0;
    foreach (Kwave::SampleArray buf, m_buffer)
	len += buf.size();
    return len;
}

//***************************************************************************
unsigned int Kwave::SampleFIFO::length()
{
    QMutexLocker _lock(&m_lock);
    return unlockedLength();
}

//***************************************************************************
void Kwave::SampleFIFO::setSize(unsigned int size)
{
    QMutexLocker _lock(&m_lock);
    m_size = size;
}

//***************************************************************************
void Kwave::SampleFIFO::crop()
{
    QMutexLocker _lock(&m_lock);

    if (!m_size) return; // no limit set
    if (unlockedLength() <= m_size) return; // nothing to do

    // we have to throw away some samples
    while ((unlockedLength() - m_buffer.head().size()) > m_size)
	m_buffer.dequeue();
    m_read_offset = 0;
    if (unlockedLength() <= m_size) return; // nothing more to do

    // put the read offset into the next buffer
    Q_ASSERT(unlockedLength() > m_size);
    m_read_offset = unlockedLength() - m_size;
    Q_ASSERT(unlockedLength() - m_read_offset == m_size);
}

//***************************************************************************
//***************************************************************************
