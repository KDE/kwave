/*************************************************************************
    KwaveConnect.cpp  -  function for connecting Kwave streaming objects
                             -------------------
    begin                : Sat Oct 27 2007
    copyright            : (C) 2007 by Thomas Eschenbacher
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

#include "config.h"

#include <QObject>
#include <QString>

#include "libkwave/KwaveConnect.h"
#include "libkwave/KwaveSampleSink.h"
#include "libkwave/SampleSource.h"

//***************************************************************************
namespace Kwave {

    //***********************************************************************
    static bool _connect_one_by_one(
	Kwave::StreamObject &src, const QString &output, unsigned int src_idx,
	Kwave::StreamObject &dst, const QString &input,  unsigned int dst_idx)
    {
	Kwave::StreamObject *s = src.port(output, src_idx);
	Kwave::StreamObject *d = dst.port(input,  dst_idx);
	Q_ASSERT(s);
	Q_ASSERT(d);
	if (!s || !d) return false;

	QObject::connect(
	    s, output.toAscii(),
	    d, input.toAscii(),
	    Qt::DirectConnection);

	return true;
    }

    //***********************************************************************
    bool connect(Kwave::StreamObject &source, const QString &output,
	         Kwave::StreamObject &sink,   const QString &input)
    {
	unsigned int src_tracks = source.tracksOfPort(output);
	unsigned int dst_tracks = sink.tracksOfPort(input);

	if (!src_tracks || !dst_tracks)
	    return false;

	Q_ASSERT(output.length());
	Q_ASSERT(input.length());
	if (!output.length() || !input.length())
	    return false;

	if ((src_tracks == 1) && (dst_tracks > 1)) {
	    // 1 output -> N inputs
	    for (unsigned int track = 0; track < dst_tracks; track++) {
		if (!_connect_one_by_one(
		    source, output, 0,
		    sink,   input,  track)) return false;
	    }
	} else if (src_tracks == dst_tracks) {
	    // N outputs -> N inputs
	    for (unsigned int track=0; track < dst_tracks; track++) {
		if (!_connect_one_by_one(
		    source, output, track,
		    sink,   input,  track)) return false;
	    }
	} else {
	    qWarning("invalid source/sink combination, %d:%d tracks",
		src_tracks, dst_tracks);
	    return false;
	}
	return true;
    }
}

//***************************************************************************
//***************************************************************************
