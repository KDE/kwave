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
SampleFormat::SampleFormat()
    :TypesMap<int,int>()
{
}

//***************************************************************************
SampleFormat::~SampleFormat()
{
}

//***************************************************************************
void SampleFormat::fill()
{
    append(0, Signed,
           i18n("linear two's complement"), 0);
    append(1, Unsigned,
           i18n("unsigned integer"), 0);
    append(2, Float,
           i18n("32-bit IEEE floating-point"), 0);
    append(3, Double,
           i18n("64-bit IEEE double-precision floating-point"), 0);
}

//***************************************************************************
//***************************************************************************
