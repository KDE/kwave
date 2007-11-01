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

#include <qobject.h>
#include <qstring.h>

#include "libkwave/KwaveConnect.h"
#include "libkwave/KwaveSampleSink.h"
#include "libkwave/KwaveSampleSource.h"

//***************************************************************************
namespace Kwave {
    bool connect(Kwave::StreamObject &source, const QString &output,
	         Kwave::StreamObject &sink,   const QString &input)
    {
	unsigned int src_tracks = source.tracks();
	unsigned int dst_tracks = sink.tracks();

	Q_ASSERT(src_tracks);
	Q_ASSERT(dst_tracks);
	if (!src_tracks || !dst_tracks)
	    return false;

	Q_ASSERT(output.length());
	Q_ASSERT(input.length());
	if (!output.length() || !input.length())
	    return false;

	if ((src_tracks == 1) && (dst_tracks == 1)) {
	    // 1 output -> 1 input
	    QObject::connect(&source, output, &sink, input);
	} else if ((src_tracks == 1) && (dst_tracks > 1)) {
	    // 1 output -> N inputs
	    for (unsigned int track=0; track < dst_tracks; track++) {
		Kwave::StreamObject *sink_n = sink[track];
		Q_ASSERT(sink_n);
		if (!sink_n) return false;
		QObject::connect(&source, output, sink_n, input);
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
		QObject::connect(source_n, output, sink_n, input);
	    }
	} else {
	    qWarning("invalid source/sink combination, M:N tracks");
	    return false;
	}
	return true;
    }
}

//***************************************************************************
//***************************************************************************
