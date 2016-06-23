/***************************************************************************
                Mul.cpp  -  multiplier
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

#include <QMutexLocker>
#include <QVariant>

#include "libkwave/Sample.h"
#include "libkwave/SampleArray.h"
#include "libkwave/modules/Mul.h"

/***************************************************************************/
Kwave::Mul::Mul()
    :Kwave::SampleSource(),
     m_queue_a(), m_queue_b(),
     m_sem_a(0), m_sem_b(0),
     m_a(), m_b(),
     m_buffer_x(blockSize()),
     m_a_is_const(false), m_b_is_const(false),
     m_value_a(0), m_value_b(0),
     m_lock()
{
}

/***************************************************************************/
Kwave::Mul::~Mul()
{
}

/***************************************************************************/
void Kwave::Mul::goOn()
{
}

/***************************************************************************/
void Kwave::Mul::multiply()
{
    unsigned int count = blockSize();
    float a = 0;
    float b = 0;
    const sample_t *p_a = 0;
    const sample_t *p_b = 0;
    sample_t       *p_x = 0;

    // get input A
    if (!m_a_is_const) {
	m_sem_a.acquire();
	{
	    QMutexLocker lock(&m_lock);
	    m_a = m_queue_a.dequeue();
	}
	if (m_a.size() < count) count = m_a.size();
    }
    else
    {
	QMutexLocker lock(&m_lock);
	a = m_value_a;
    }

    // get input B
    if (!m_b_is_const) {
	m_sem_b.acquire();
	{
	    QMutexLocker lock(&m_lock);
	    m_b = m_queue_b.dequeue();
	}
	if (m_b.size() < count) count = m_b.size();
    }
    else
    {
	QMutexLocker lock(&m_lock);
	b = m_value_b;
    }

    // check sizes of the buffers
    if (!m_a_is_const && (count > m_a.size()))
	count = m_a.size();
    if (!m_b_is_const && (count > m_b.size()))
	count = m_b.size();

    // special handling for zero length input
    if (!count) {
	emit output(Kwave::SampleArray()); // emit zero length output
	return;                            // and bail out
    }

//     if (!m_a_is_const && !m_b_is_const && (m_a.size() != m_b.size()))
// 	qWarning("Kwave::Mul: block sizes differ: %u x %u -> shrinked to %u",
// 	    m_a.size(), m_b.size(), count);

    bool ok = m_buffer_x.resize(count);
    Q_ASSERT(ok);
    Q_UNUSED(ok);

    // get pointers to the buffer's raw data
    p_a = m_a.constData();
    p_b = m_b.constData();
    p_x = m_buffer_x.data();
    Q_ASSERT((m_a_is_const || p_a) && (m_b_is_const || p_b) && p_x);
    if ((!m_a_is_const && !p_a) || (!m_b_is_const && !p_b) || !p_x)
	return;

    // do the multiplication of the whole buffer
    for (; count; count--)
    {
	if (!m_a_is_const) { a = sample2float(*p_a); ++p_a; }
	if (!m_b_is_const) { b = sample2float(*p_b); ++p_b; }
	float y = a * b;
	if (y > float( 1.0)) y = float( 1.0);
	if (y < float(-1.0)) y = float(-1.0);
	*(p_x++) = float2sample(y);
    }

    // emit the result
    emit output(m_buffer_x);
}

/***************************************************************************/
void Kwave::Mul::input_a(Kwave::SampleArray data)
{
    {
	QMutexLocker lock(&m_lock);

	m_queue_a.enqueue(data);
	m_sem_a.release();
	m_a_is_const = false;
    }
    if (m_b_is_const || !m_queue_b.isEmpty()) multiply();
}

/***************************************************************************/
void Kwave::Mul::input_b(Kwave::SampleArray data)
{
    {
	QMutexLocker lock(&m_lock);

	m_queue_b.enqueue(data);
	m_sem_b.release();
	m_b_is_const = false;
    }
    if (m_a_is_const || !m_queue_a.isEmpty()) multiply();
}

/***************************************************************************/
void Kwave::Mul::set_a(const QVariant &a)
{
    QMutexLocker lock(&m_lock);

    m_value_a = QVariant(a).toFloat();
    m_a_is_const = true;
}

/***************************************************************************/
void Kwave::Mul::set_b(const QVariant &b)
{
    QMutexLocker lock(&m_lock);

    m_value_b = QVariant(b).toFloat();
    m_b_is_const = true;
}

/***************************************************************************/
/***************************************************************************/
