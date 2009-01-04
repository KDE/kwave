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

#include "Selection.h"

//***************************************************************************
Selection::Selection(unsigned int offset, unsigned int length)
    :QObject(), m_offset(offset), m_length(length)
{
}

//***************************************************************************
Selection::Selection(const Selection &other)
    :QObject(), m_offset(other.offset()), m_length(other.length())
{
}

//***************************************************************************
Selection::~Selection()
{
}

//***************************************************************************
#include "Selection.moc"
//***************************************************************************
//***************************************************************************
