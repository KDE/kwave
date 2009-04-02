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
#include "libkwave/KwaveSampleSource.h"

//***************************************************************************
namespace Kwave {

    //***********************************************************************
    static void _connect_one_by_one(
	Kwave::StreamObject *source, const char *output,
	Kwave::StreamObject *sink,   const char *input)
    {
	QObject::connect(
	    source, output,
	    sink,   input,
	    Qt::DirectConnection);
    }

    //***********************************************************************
    bool connect(Kwave::StreamObject &source, const QString &output,
	         Kwave::StreamObject &sink,   const QString &input)
    {
	unsigned int src_tracks = source.tracks();
	unsigned int dst_tracks = sink.tracks();

	if (!src_tracks || !dst_tracks)
	    return false;

	Q_ASSERT(output.length());
	Q_ASSERT(input.length());
	if (!output.length() || !input.length())
	    return false;

	if ((src_tracks == 1) && (dst_tracks == 1)) {
	    // 1 output -> 1 input
	    _connect_one_by_one(
		source[0], output.toAscii(),
		sink[0], input.toAscii());
	} else if ((src_tracks == 1) && (dst_tracks > 1)) {
	    // 1 output -> N inputs
	    for (unsigned int track=0; track < dst_tracks; track++) {
		Kwave::StreamObject *sink_n = sink[track];
		Q_ASSERT(sink_n);
		if (!sink_n) return false;
		_connect_one_by_one(
		    source[0], output.toAscii(),
		    sink_n, input.toAscii());
	    }
	} else if (src_tracks == dst_tracks) {
	    // N outputs -> N inputs
	    for (unsigned int track=0; track < dst_tracks; track++) {
		Kwave::StreamObject *source_n = source[track];
		Kwave::StreamObject *sink_n   = sink[track];
		Q_ASSERT(source_n);
		Q_ASSERT(sink_n);
		if (!source_n) return false;
		if (!sink_n)   return false;
		_connect_one_by_one(
		    source_n, output.toAscii(),
		    sink_n, input.toAscii());
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
