/***************************************************************************
           MouseMark.cpp -  Handling of mouse selection
			     -------------------
    begin                : Sun Nov 12 2000
    copyright            : (C) 2000 by Thomas Eschenbacher
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

#include <config.h>
#include <QtCore/QtGlobal>
#include "libgui/MouseMark.h"

//****************************************************************************
Kwave::MouseMark::MouseMark()
    :m_initial(0), m_last(0)
{
}

//****************************************************************************
void Kwave::MouseMark::set(sample_index_t l, sample_index_t r)
{
    m_initial = l;
    m_last    = r;
}

//****************************************************************************
sample_index_t Kwave::MouseMark::left() const
{
    return (m_initial < m_last) ? m_initial : m_last;
}

//****************************************************************************
sample_index_t Kwave::MouseMark::right() const
{
    return (m_initial > m_last) ? m_initial : m_last;
}

//****************************************************************************
void Kwave::MouseMark::grep(sample_index_t x)
{
    const sample_index_t d_last  =
	(x > m_last)    ? (x - m_last)    : (m_last    - x);
    const sample_index_t d_first =
	(x > m_initial) ? (x - m_initial) : (m_initial - x);
    if (d_last > d_first) {
	m_initial = m_last;
    }
    m_last = x;
}

//****************************************************************************
void Kwave::MouseMark::update(sample_index_t x)
{
    m_last = x;
}

//****************************************************************************
//****************************************************************************
