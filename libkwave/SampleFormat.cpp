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
SampleFormat::Map::Map()
    :TypesMap<int,SampleFormat::Format>()
{
}

//***************************************************************************
SampleFormat::Map::~Map()
{
}

//***************************************************************************
void SampleFormat::Map::fill()
{
    append(0, SampleFormat::Signed,
           i18n("Linear Two's Complement"), 0);
    append(1, SampleFormat::Unsigned,
           i18n("Unsigned Integer"), 0);
    append(2, SampleFormat::Float,
           i18n("32-bit IEEE Floating-Point"), 0);
    append(3, SampleFormat::Double,
           i18n("64-bit IEEE Double-Precision Floating-Point"), 0);
}

//***************************************************************************
void SampleFormat::fromInt(int i)
{
    SampleFormat::Map map;
    SampleFormat::Format format = static_cast<SampleFormat::Format>(i);
    int index = map.findFromData(format);
    m_format = (index >= 0) ? format : SampleFormat::Unknown;
}

//***************************************************************************
//***************************************************************************
