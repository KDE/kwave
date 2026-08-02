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

#include "Encoder.h"
#include "config.h"

#include <QObject>

#include "libkwave/Connect.h"
#include "libkwave/SampleSink.h"
#include "libkwave/SampleSource.h"

//***************************************************************************
namespace Kwave {

    //***********************************************************************
    bool connect(Kwave::StreamObject &source,
                 Kwave::StreamObject &sink,
                 unsigned int port)
    {
        unsigned int src_tracks = source.tracksOut();
        unsigned int dst_tracks = sink.tracksIn();

        // if (tracks == 0) -> use in() / out() instead
        unsigned int outputs = (src_tracks != 0) ?
                                src_tracks : source.outputs();
        unsigned int inputs  = (dst_tracks != 0) ?
                                dst_tracks : source.inputs();

        if ((outputs == 1) && (inputs > 1)) {
            // 1 output  -> N inputs
        } else if (outputs == inputs) {
            // N outputs -> N inputs
        } else {
            qWarning("invalid source/sink combination, %u:%u tracks",
                inputs, outputs);
            return false;
        }

        for (unsigned int i = 0; i < inputs; i++) {
            unsigned int src_idx = (outputs == inputs) ? i : 0;
            unsigned int dst_idx = (dst_tracks != 0)   ? i : 0;
            StreamObject *s = source.out(src_idx);
            StreamObject *d = sink.in(dst_idx);
            Q_ASSERT(s != nullptr);
            Q_ASSERT(d != nullptr);
            if ((s == nullptr) || (d == nullptr)) return false;
            s->connectTo(d, (dst_tracks != 0) ? port : i);
        }

        return true;
    }
}

//***************************************************************************
//***************************************************************************
