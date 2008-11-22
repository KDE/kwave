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

#include "config.h"
#include "Label.h"

//***************************************************************************
Label::Label()
    :m_data(0)
{
}

//***************************************************************************
Label::Label(unsigned int position, const QString &name)
    :m_data(new LabelData)
{
    moveTo(position);
    rename(name);
}

//***************************************************************************
Label::~Label()
{
}

//***************************************************************************
void Label::moveTo(unsigned int position)
{
    if (m_data) m_data->m_position = position;
}

//***************************************************************************
unsigned int Label::pos() const
{
    return (m_data) ? m_data->m_position : -1;
}

//***************************************************************************
void Label::rename(const QString &name)
{
    if (m_data) m_data->m_name = name;
}

//***************************************************************************
QString Label::name() const
{
    return (m_data) ? m_data->m_name : QString();
}

//***************************************************************************
bool Label::isNull() const
{
    return (m_data == 0);
}

//***************************************************************************
//***************************************************************************
Label::LabelData::LabelData()
    :QSharedData(), m_position(-1), m_name()
{
}

//***************************************************************************
Label::LabelData::LabelData(const LabelData &other)
    :QSharedData(), m_position(other.m_position), m_name(other.m_name)
{
}

//***************************************************************************
Label::LabelData::~LabelData()
{
}

//***************************************************************************
//***************************************************************************
