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

#include "libkwave/modules/SampleBuffer.h"

//***************************************************************************
Kwave::SampleBuffer::SampleBuffer(QObject *parent)
    :Kwave::SampleSink(parent), m_data(), m_offset(0)
{
}

//***************************************************************************
Kwave::SampleBuffer::~SampleBuffer()
{
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
    const sample_t *raw = m_data.data() + m_offset;

    // advance the offset
    m_offset += len;

    return raw;
}

//***************************************************************************
void Kwave::SampleBuffer::finished()
{
    emit output(m_data);
}

//***************************************************************************
void Kwave::SampleBuffer::input(Kwave::SampleArray data)
{
    m_data   = data;
    m_offset = 0;
}

//***************************************************************************
#include "SampleBuffer.moc"
//***************************************************************************
//***************************************************************************
