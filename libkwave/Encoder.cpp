/***************************************************************************
            Encoder.cpp  -  abstract base class of all encoders
			     -------------------
    begin                : May 08 2007
    copyright            : (C) 2007 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "libkwave/Encoder.h"

//***************************************************************************
Kwave::Encoder::Encoder()
    :QObject(), Kwave::CodecBase()
{
}

//***************************************************************************
#include "Encoder.moc"
//***************************************************************************
//***************************************************************************
