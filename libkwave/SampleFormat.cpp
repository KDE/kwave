/***************************************************************************
      SampleFormat.cpp  -  Map for all known sample formats
                             -------------------
    begin                : Sun Jul 28 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
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

#include "SampleFormat.h"

//***************************************************************************
Kwave::SampleFormat::Map::Map()
    :Kwave::TypesMap<int,Kwave::SampleFormat::Format>()
{
    fill();
}

//***************************************************************************
Kwave::SampleFormat::Map::~Map()
{
}

//***************************************************************************
void Kwave::SampleFormat::Map::fill()
{
    append(0, Kwave::SampleFormat::Signed,
           i18n("Linear Two's Complement"), 0);
    append(1, Kwave::SampleFormat::Unsigned,
           i18n("Unsigned Integer"), 0);
    append(2, Kwave::SampleFormat::Float,
           i18n("32-bit IEEE Floating-Point"), 0);
    append(3, Kwave::SampleFormat::Double,
           i18n("64-bit IEEE Double Precision Floating-Point"), 0);
}

//***************************************************************************
void Kwave::SampleFormat::fromInt(int i)
{
    Kwave::SampleFormat::Map map;
    Kwave::SampleFormat::Format format =
	static_cast<Kwave::SampleFormat::Format>(i);
    int index = map.findFromData(format);
    m_format = (index >= 0) ? format : Kwave::SampleFormat::Unknown;
}

//***************************************************************************
//***************************************************************************
