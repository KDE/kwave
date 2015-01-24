/***************************************************************************
            Decoder.cpp  -  abstract base class of all decoders
			     -------------------
    begin                : Jun 01 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
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

#include <config.h>
#include "libkwave/Decoder.h"

//***************************************************************************
Kwave::Decoder::Decoder()
    :QObject(), Kwave::CodecBase()
{
}

//***************************************************************************
Kwave::Decoder::~Decoder()
{
}

//***************************************************************************
#include "Decoder.moc"
//***************************************************************************
//***************************************************************************
