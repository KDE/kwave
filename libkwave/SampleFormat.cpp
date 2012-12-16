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

#include "libkwave/SampleFormat.h"
#include "libkwave/String.h"

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
    append(0, Kwave::SampleFormat::Signed,   _("SIGNED"),
              _(I18N_NOOP("Linear Two's Complement")));
    append(1, Kwave::SampleFormat::Unsigned, _("UNSIGNED"),
              _(I18N_NOOP("Unsigned Integer")));
    append(2, Kwave::SampleFormat::Float,    _("FLOAT"),
              _(I18N_NOOP("32-bit IEEE Floating-Point")));
    append(3, Kwave::SampleFormat::Double,   _("DOUBLE"),
              _(I18N_NOOP("64-bit IEEE Double Precision Floating-Point")));
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
