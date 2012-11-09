/*************************************************************************
           SampleSink.h  -  base class with a generic sample sink
                             -------------------
    begin                : Sun Oct 07 2007
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

#ifndef _SAMPLE_SINK_H_
#define _SAMPLE_SINK_H_

#include "config.h"
#include <QObject>

#include "libkwave/SampleArray.h"
#include "libkwave/modules/StreamObject.h"

namespace Kwave {
    class SampleSink: public Kwave::StreamObject
    {
        Q_OBJECT
    public:
        /**
         * Constructor
         *
         * @param parent a parent object, passed to QObject (optional)
         */
        SampleSink(QObject *parent = 0);

        /** Destructor */
        virtual ~SampleSink();

        /**
         * Returns true if the end of the destination has been reached,
         * e.g. EOF on the input stream, and it is unable to receive
         * more data.
         *
         * @return true if the sink can't receive more data, otherwise false
         */
        virtual bool done() const { return false; }

    };
}

#endif /* _SAMPLE_SINK_H_ */
