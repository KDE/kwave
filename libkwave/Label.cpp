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
    :Kwave::MetaData()
{
}

//***************************************************************************
Label::Label(sample_index_t position, const QString &name)
    :Kwave::MetaData()
{
    setProperty(Kwave::MetaData::STDPROP_POS, position);
    setProperty(Kwave::MetaData::STDPROP_DESCRIPTION, name);
}

//***************************************************************************
Label::~Label()
{
}

//***************************************************************************
void Label::moveTo(sample_index_t position)
{
    setProperty(Kwave::MetaData::STDPROP_POS, position);
}

//***************************************************************************
sample_index_t Label::pos() const
{
    return static_cast<sample_index_t>(
	property(Kwave::MetaData::STDPROP_POS).toULongLong()
    );
}

//***************************************************************************
void Label::rename(const QString &name)
{
    setProperty(Kwave::MetaData::STDPROP_DESCRIPTION, name);
}

//***************************************************************************
QString Label::name() const
{
    return property(Kwave::MetaData::STDPROP_DESCRIPTION).toString();
}

//***************************************************************************
//***************************************************************************
