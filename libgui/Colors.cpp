/***************************************************************************
             Colors.cpp  -  Kwave color sets
                             -------------------
    begin                : Sat Oct 05 2013
    copyright            : (C) 2013 by Thomas Eschenbacher
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

#include "libgui/Colors.h"

Kwave::Colors::ColorSet Kwave::Colors::Normal =
{
    /* background   : */ Qt::black,
    /* sample       : */ Qt::white,
    /* interpolated : */ Qt::lightGray,
    /* zero         : */ Qt::green,
    /* zero_unused  : */ Qt::darkGray
};

Kwave::Colors::ColorSet Kwave::Colors::Disabled =
{
    /* background   : */ QColor(Qt::darkGray).darker(300),
    /* sample       : */ Kwave::Colors::Normal.sample.dark(),
    /* interpolated : */ Kwave::Colors::Normal.interpolated.dark(),
    /* zero         : */ Qt::darkGreen,
    /* zero_unused  : */ Qt::black
};

//***************************************************************************
//***************************************************************************
