/*************************************************************************
       SampleBuffer.cpp  -  simple buffer for sample arrays
                             -------------------
    begin                : Sun Oct 17 2010
    copyright            : (C) 2010 by Thomas Eschenbacher
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

#include <threadweaver/DebuggingAids.h>

#include "libkwave/Utils.h"
#include "libkwave/modules/SampleBuffer.h"

//***************************************************************************
Kwave::SampleBuffer::BufferJob::BufferJob(Kwave::SampleBuffer *buffer)
    :ThreadWeaver::Job(), m_buffer(buffer), m_data(), m_sema(1)
{
}

//***************************************************************************
Kwave::SampleBuffer::BufferJob::~BufferJob()
{
    m_sema.acquire();

    int i = 0;
    while (!isFinished()) {
	qDebug("buffer job %p waiting... #%u", static_cast<void *>(this), i++);
	Kwave::yield();
    }
    Q_ASSERT(isFinished());
}

//***************************************************************************
void Kwave::SampleBuffer::BufferJob::enqueue(Kwave::SampleArray data)
{
    m_sema.acquire();
    m_data = data;
}

//***************************************************************************
void Kwave::SampleBuffer::BufferJob::run()
{
    if (!m_buffer) return;
    m_buffer->emitData(m_data);
    m_sema.release();
}

//***************************************************************************
//***************************************************************************
Kwave::SampleBuffer::SampleBuffer(QObject *parent)
    :Kwave::SampleSink(parent),
     m_data(), m_offset(0), m_buffered(0), m_weaver(), m_job(this)
{
}

//***************************************************************************
Kwave::SampleBuffer::~SampleBuffer()
{
    m_weaver.finish();
}

//***************************************************************************
bool Kwave::SampleBuffer::isEmpty() const
{
    return m_data.isEmpty();
}

//***************************************************************************
Kwave::SampleArray &Kwave::SampleBuffer::data()
{
    return m_data;
}

//***************************************************************************
const Kwave::SampleArray &Kwave::SampleBuffer::data() const
{
    return m_data;
}

//***************************************************************************
unsigned int Kwave::SampleBuffer::available() const
{
    unsigned int size = m_data.size();
    return (size > m_offset) ? (size - m_offset) : 0;
}

//***************************************************************************
const sample_t *Kwave::SampleBuffer::get(unsigned int len)
{
    const unsigned int size = m_data.size();
    if (m_offset > size) return 0; // already reached EOF

    // limit read length to the size of the buffer
    if (len > size) len = size;

    // get a pointer to the raw data
    const sample_t *raw = m_data.constData() + m_offset;

    // advance the offset
    m_offset += len;

    return raw;
}

//***************************************************************************
void Kwave::SampleBuffer::put(sample_t sample)
{
    const unsigned int block_size = Kwave::StreamObject::blockSize();

    if (m_buffered >= block_size) finished();

    // initial fill of the buffer?
    if (m_data.size() < block_size) m_data.resize(block_size);

    m_data[m_buffered++] = sample;
    if (m_buffered >= block_size) finished();
}


//***************************************************************************
void Kwave::SampleBuffer::finished()
{
    if (m_buffered && (m_data.size() != m_buffered))
	m_data.resize(m_buffered);
    enqueue(m_data);

    m_buffered = 0;
    m_data.resize(Kwave::StreamObject::blockSize());
}

//***************************************************************************
void Kwave::SampleBuffer::input(Kwave::SampleArray data)
{
    // if we have buffered data, flush that first
    if (m_buffered) {
	m_data.resize(m_buffered);
	m_buffered = 0;
	enqueue(m_data);
    }

    // take over the input array directly
    m_data     = data;
    m_offset   = 0;
    m_buffered = data.size();
}

//***************************************************************************
void Kwave::SampleBuffer::enqueue(Kwave::SampleArray data)
{
    m_job.enqueue(data);
    m_weaver.enqueue(&m_job);
}

//***************************************************************************
void Kwave::SampleBuffer::emitData(Kwave::SampleArray data)
{
    emit output(data);
}

//***************************************************************************
#include "SampleBuffer.moc"
//***************************************************************************
//***************************************************************************
