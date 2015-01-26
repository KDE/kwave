/*************************************************************************
     StandardBitrates.h  -  well known bitrates from ogg/vorbis + MP3
                             -------------------
    begin                : Tue Jun 17 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
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

#include <config.h>
#include <limits.h>
#include "libkwave/StandardBitrates.h"

/** the global static list of standard bitrates */
static Kwave::StandardBitrates g_bitrates;

//***************************************************************************
Kwave::StandardBitrates::StandardBitrates()
    :QList<int>()
{
        // use well-known bitrates from MP3
        append(  8000);
        append( 16000);
        append( 24000);
        append( 32000);
        append( 40000);
        append( 56000);
        append( 64000);
        append( 80000);
        append( 96000);
        append(112000);
        append(128000);
        append(144000);
        append(160000);
        append(176000);
        append(192000);
        append(224000);
        append(256000);
        append(288000);
        append(320000);
        append(352000);
        append(384000);
        append(416000);
        append(448000);
}

//***************************************************************************
Kwave::StandardBitrates::~StandardBitrates()
{
}

//***************************************************************************
const Kwave::StandardBitrates &Kwave::StandardBitrates::instance()
{
    return g_bitrates;
}

//***************************************************************************
int Kwave::StandardBitrates::nearest(int rate) const
{
    int best = rate;
    int min_delta = INT_MAX;

    foreach (int value, *this) {
	int delta = (value > rate) ? (value-rate) : (rate-value);
	if (!delta) return rate; // complete match, easy case

	if (delta < min_delta) {
	    // found a better alternative
	    min_delta = delta;
	    best      = value;
	}
    }

    return best;
}

//***************************************************************************
//***************************************************************************
