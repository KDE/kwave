/***************************************************************************
    MultiTrackWriter.cpp - writer for multi-track signals
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

#include <qlist.h>

#include "libkwave/MultiTrackReader.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SampleWriter.h"

//***************************************************************************

MultiTrackWriter &MultiTrackWriter::operator << (
	const MultiTrackReader &source)
{
    unsigned int src_tracks = source.count();
    unsigned int dst_tracks = count();

    if (src_tracks != dst_tracks) {
        // create a mixer matrix and pass everything through
        warning("MultiTrackWriter << : transfer with mixing not implemented yet!");
    } else {
	// process 1:1
	unsigned int track;
	for (track = 0; track < src_tracks; ++track) {
	    *at(track) << *source.at(track);
	}
    }

    return *this;
};

//***************************************************************************
//***************************************************************************
