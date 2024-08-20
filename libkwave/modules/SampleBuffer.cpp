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

#include <QFuture>
#include <QtConcurrentRun>

#include "libkwave/Utils.h"
#include "libkwave/modules/SampleBuffer.h"

//***************************************************************************
//***************************************************************************
Kwave::SampleBuffer::SampleBuffer(QObject *parent)
    :Kwave::SampleSink(parent),
     m_data(), m_offset(0), m_buffered(0), m_sema(1)
{
}

//***************************************************************************
Kwave::SampleBuffer::~SampleBuffer()
{
    m_sema.acquire();
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
const Kwave::SampleArray &Kwave::SampleBuffer::constData() const
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
    if (m_offset > size) return nullptr; // already reached EOF

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
    bool ok = m_data.resize(block_size);
    Q_ASSERT(ok);
    Q_UNUSED(ok)

    m_data[m_buffered++] = sample;
    if (m_buffered >= block_size) finished();
}


//***************************************************************************
void Kwave::SampleBuffer::finished()
{
    bool ok = true;

    if (m_buffered && (m_data.size() != m_buffered))
        ok &= m_data.resize(m_buffered);

    enqueue(m_data);
    m_buffered = 0;
    ok &= m_data.resize(Kwave::StreamObject::blockSize());
    Q_ASSERT(ok);
}

//***************************************************************************
void Kwave::SampleBuffer::input(Kwave::SampleArray data)
{
    // if we have buffered data, flush that first
    if (m_buffered) {
        bool ok = m_data.resize(m_buffered);
        Q_ASSERT(ok);
        Q_UNUSED(ok)
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
    m_sema.acquire();
    auto discard = QtConcurrent::run(&Kwave::SampleBuffer::emitData, this, data);
}

//***************************************************************************
void Kwave::SampleBuffer::emitData(Kwave::SampleArray data)
{
    // NOTE: this signal could be connected to a slot that blocks for a while
    emit output(data);
    m_sema.release();
}

//***************************************************************************
//***************************************************************************

#include "moc_SampleBuffer.cpp"
