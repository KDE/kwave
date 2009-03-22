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

#include <QMutexLocker>
#include <QVariant>

#include "libkwave/modules/KwaveMul.h"
#include "libkwave/KwaveSampleArray.h"
#include "libkwave/Sample.h"

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
    unsigned int count = blockSize();
    float a = 0;
    float b = 0;
    sample_t *p_a = 0;
    sample_t *p_b = 0;
    sample_t *p_x = 0;

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
    Q_ASSERT(m_a_is_const || (m_a.size() == count));
    Q_ASSERT(m_b_is_const || (m_b.size() == count));
    //     Q_ASSERT(m_buffer_x.size() == count);
    if ((!m_a_is_const && (m_a.size() != count)) ||
	(!m_b_is_const && (m_b.size() != count)) ||
	(m_buffer_x.size() != count))
	return;

    if (m_buffer_x.size() != count)
	m_buffer_x.resize(count);
    Q_ASSERT(m_buffer_x.size() == count);

    // get pointers to the buffer's raw data
    p_a = m_a.data();
    p_b = m_b.data();
    p_x = m_buffer_x.data();
    Q_ASSERT((m_a_is_const || p_a) && (m_b_is_const || p_b) && p_x);
    if ((!m_a_is_const && !p_a) || (!m_b_is_const && !p_b) || !p_x)
	return;

    // do the multiplication of the whole buffer
    while (count--)
    {
	if (!m_a_is_const) a = sample2float(*(p_a++));
	if (!m_b_is_const) b = sample2float(*(p_b++));
	float y = a * b;
	if (y >  1.0) y =  1.0;
	if (y < -1.0) y = -1.0;
	*(p_x++) = float2sample(y);
    }

    // emit the result
    emit output(m_buffer_x);
}

/***************************************************************************/
void Kwave::Mul::input_a(Kwave::SampleArray data)
{
    QMutexLocker lock(&m_lock);

    m_queue_a.enqueue(data);
    m_sem_a.release();
    m_a_is_const = false;
}

/***************************************************************************/
void Kwave::Mul::input_b(Kwave::SampleArray data)
{
    QMutexLocker lock(&m_lock);

    m_queue_b.enqueue(data);
    m_sem_b.release();
    m_b_is_const = false;
}

/***************************************************************************/
void Kwave::Mul::set_a(const QVariant &a)
{
    QMutexLocker lock(&m_lock);

    m_value_a = QVariant(a).toDouble();
    m_a_is_const = true;
}

/***************************************************************************/
void Kwave::Mul::set_b(const QVariant &b)
{
    QMutexLocker lock(&m_lock);

    m_value_b = QVariant(b).toDouble();
    m_b_is_const = true;
}

/***************************************************************************/
using namespace Kwave;
#include "KwaveMul.moc"
/***************************************************************************/
/***************************************************************************/
