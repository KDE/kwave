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

#include <QVariant>
#include "libkwave/modules/KwaveMul.h"
#include "libkwave/KwaveSampleArray.h"
#include "libkwave/Sample.h"

/***************************************************************************/
Kwave::Mul::Mul()
    :Kwave::SampleSource(),
     m_buffer_a(blockSize()), m_buffer_b(blockSize()),
     m_buffer_x(blockSize()),
     m_count_a(0), m_count_b(0),
     m_a_is_const(false), m_b_is_const(false),
     m_value_a(0), m_value_b(0)
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

    if (!m_a_is_const && !m_b_is_const) {
	Q_ASSERT(m_count_a == m_count_b);
	if (m_count_a != m_count_b) {
	    qWarning("Kwave::Mul::goOn() out of sync, [A]=%u != [B]=%u",
		m_count_a, m_count_b);
	    m_count_a = m_count_b = 0;
	    return;
	}
    }

    // check sizes of the buffers
    Q_ASSERT(m_a_is_const || (m_buffer_a.size() == count));
    Q_ASSERT(m_b_is_const || (m_buffer_b.size() == count));
    Q_ASSERT(m_buffer_x.size() == count);
    if ((!m_a_is_const && (m_buffer_a.size() != count)) ||
        (!m_b_is_const && (m_buffer_b.size() != count)) ||
        (m_buffer_x.size() != count))
	return;

    // get pointers to the buffer's raw data
    sample_t *p_a = m_buffer_a.data();
    sample_t *p_b = m_buffer_b.data();
    sample_t *p_x = m_buffer_x.data();
    Q_ASSERT((m_a_is_const || p_a) && (m_b_is_const || p_b) && p_x);
    if ((!m_a_is_const && !p_a) || (!m_b_is_const && !p_b) || !p_x)
	return;

    // do the multiplication of the whole buffer
    for (unsigned int pos=0; pos < count; pos++, p_a++, p_b++, p_x++)
    {
	float a = (m_a_is_const) ? m_value_a : sample2float(*p_a);
	float b = (m_b_is_const) ? m_value_b : sample2float(*p_b);
	float y = a * b;
	if (y >  1.0) y =  1.0;
	if (y < -1.0) y = -1.0;
	*p_x = float2sample(y);
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
    m_a_is_const = false;
}

/***************************************************************************/
void Kwave::Mul::input_b(Kwave::SampleArray &data)
{
    m_buffer_b = data;
    m_count_b++;
    m_b_is_const = false;
}

/***************************************************************************/
void Kwave::Mul::set_a(const QVariant &a)
{
    m_value_a = QVariant(a).toDouble();
    m_a_is_const = true;
}

/***************************************************************************/
void Kwave::Mul::set_b(const QVariant &b)
{
    m_value_b = QVariant(b).toDouble();
    m_b_is_const = true;
}

/***************************************************************************/
using namespace Kwave;
#include "KwaveMul.moc"
/***************************************************************************/
/***************************************************************************/
