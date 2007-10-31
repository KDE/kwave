/*************************************************************************
    KwaveSampleSource.h  -  base class with a generic sample source
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

#ifndef _KWAVE_SAMPLE_SOURCE_H_
#define _KWAVE_SAMPLE_SOURCE_H_

#include "config.h"
#include <qobject.h>

#include "libkwave/KwaveSampleArray.h"

namespace Kwave {
    class SampleSource: public QObject
    {
        Q_OBJECT
    public:

        /**
         * Constructor
         *
         * @param parent a parent object, passed to QObject (optional)
         * @param name a free name, for identifying this object,
         *             will be passed to the QObject (optional)
         */
        SampleSource(QObject *parent=0, const char *name=0);

        /** Destructor */
        virtual ~SampleSource();

        /**
         * Each KwaveSampleSource has to derive this method for producing
         * sample data. It then should emit a signal like this:
         * "output(SampleArray &data)"
         */
        virtual void goOn() = 0;

        /**
         * Returns true if the end of the source has been reached,
         * e.g. at EOF of an input stream. The default implementation
         * always returns false, which means that the source is always
         * able to produce data (useful for signal generators).
         *
         * @return true if it can produce more sample data, otherwise false
         */
        virtual bool done() { return false; };

        /**
         * Returns the number of tracks that the source provides
         * @return number of tracks, default is 1
         */
        virtual unsigned int tracks() const { return 1; };

        /**
         * Returns the source that corresponds to one specific track
         * if the object has multiple tracks. For single-track objects
         * it returns "this" for the first index and 0 for all others
         */
        virtual Kwave::SampleSource * operator [] (unsigned int track)
        {
            return (track == 0) ? this : 0;
        };
    };
}

#endif /* _KWAVE_SAMPLE_SOURCE_H_ */
