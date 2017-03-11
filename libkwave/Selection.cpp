/***************************************************************************
           Selection.cpp - Simple class for a selection
			     -------------------
    begin                : Tue May 08 2007
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

#include "libkwave/Selection.h"

//***************************************************************************
Kwave::Selection::Selection(sample_index_t offset, sample_index_t length)
    :QObject(), m_offset(offset), m_length(length)
{
}

//***************************************************************************
Kwave::Selection::Selection(const Kwave::Selection &other)
    :QObject(), m_offset(other.offset()), m_length(other.length())
{
}

//***************************************************************************
Kwave::Selection::~Selection()
{
}

//***************************************************************************
void Kwave::Selection::select(sample_index_t offset, sample_index_t length)
{
    m_offset = offset;
    m_length = length;
    emit changed(m_offset, m_length);
}

//***************************************************************************
//***************************************************************************
