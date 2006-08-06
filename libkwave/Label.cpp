/***************************************************************************
              Label.cpp  -  representation of a label within a signal
                             -------------------
    begin                : Mon Jul 31 2006
    copyright            : (C) 2006 by Thomas Eschenbacher
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

#include "Label.h"

//***************************************************************************
Label::Label(unsigned int position, const QString &name)
    :m_position(position), m_name(name)
{
}

//***************************************************************************
Label::~Label()
{
}

//***************************************************************************
void Label::moveTo(unsigned int position)
{
    m_position = position;
}

//***************************************************************************
unsigned int Label::pos() const
{
    return m_position;
}

//***************************************************************************
void Label::rename(const QString &name)
{
    m_name = name;
}

//***************************************************************************
QString Label::name() const
{
    return m_name;
}

//***************************************************************************
//***************************************************************************
