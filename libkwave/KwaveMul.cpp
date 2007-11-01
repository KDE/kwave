/***************************************************************************
           KwaveMul.cpp  -  multiplier
                             -------------------
    begin                : Thu Nov 01 2007
    copyright            : (C) 2007 by Thomas Eschenbacher
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
#include "libkwave/KwaveMul.h"
#include "libkwave/KwaveSampleArray.h"
#include "libkwave/Sample.h"

/***************************************************************************/
Kwave::Mul::Mul()
    :Kwave::SampleSource(0, "Kwave::Mul"),
     m_buffer_a(blockSize()), m_buffer_b(blockSize()),
     m_buffer_x(blockSize()),
     m_count_a(0), m_count_b(0)
{
}

/***************************************************************************/
Kwave::Mul::~Mul()
{
}

/***************************************************************************/
void Kwave::Mul::goOn()
{
    const unsigned int count = blockSize();
    Q_ASSERT(m_count_a == m_count_b);
    if (m_count_a != m_count_b) {
	qWarning("Kwave::Mul::goOn() out of sync, count[A]=%u != count[b]=%u",
	    m_count_a, m_count_b);
	m_count_a = m_count_b = 0;
	return;
    }

    // check sizes of the buffers
    Q_ASSERT(m_buffer_a.size() == count);
    Q_ASSERT(m_buffer_b.size() == count);
    Q_ASSERT(m_buffer_x.size() == count);
    if ((m_buffer_a.size() != count) ||
        (m_buffer_b.size() != count) ||
        (m_buffer_x.size() != count))
	return;

    // get pointers to the buffer's raw data
    sample_t *p_a = m_buffer_a.data();
    sample_t *p_b = m_buffer_b.data();
    sample_t *p_x = m_buffer_x.data();
    Q_ASSERT(p_a && p_b && p_x);
    if (!p_a || !p_b || !p_x)
	return;

    // do the multiplication of the whole buffer
    for (unsigned int pos=0; pos < count; pos++, p_a++, p_b++, p_x++)
    {
	*p_x = float2sample(sample2float(*p_a) * sample2float(*p_b));
    }

    // if we were in sync, reset counters
    if (m_count_a == m_count_b)
	m_count_a = m_count_b = 0;

    // emit the result
    emit output(m_buffer_x);
}

/***************************************************************************/
void Kwave::Mul::input_a(Kwave::SampleArray &data)
{
    m_buffer_a = data;
    m_count_a++;
}

/***************************************************************************/
void Kwave::Mul::input_b(Kwave::SampleArray &data)
{
    m_buffer_b = data;
    m_count_b++;
}

/***************************************************************************/
using namespace Kwave;
#include "KwaveMul.moc"
/***************************************************************************/
/***************************************************************************/
