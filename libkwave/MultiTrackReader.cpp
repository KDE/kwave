/***************************************************************************
    MultiTrackReader.cpp - reader for multi-track signals
			     -------------------
    begin                : Sat Jun 30 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#include "libkwave/MultiTrackReader.h"
#include "libkwave/SampleReader.h"

//***************************************************************************
bool MultiTrackReader::eof()
{
    unsigned int c = this->count();
    for (unsigned int r = 0; r < c; r++) {
	SampleReader *reader = this->at(r);
	ASSERT(reader);
	if (!reader) continue;
	if (reader->eof()) return true;
    }
    return false;
}

//***************************************************************************
//***************************************************************************
