/*************************************************************************
    SampleSource.h  -  base class with a generic sample source
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

#ifndef SAMPLE_SOURCE_H
#define SAMPLE_SOURCE_H

#include "config.h"
#include "libkwave_export.h"

#include <QtGlobal>
#include <QObject>

#include "libkwave/SampleArray.h"
#include "libkwave/modules/StreamObject.h"

namespace Kwave
{
    class LIBKWAVE_EXPORT SampleSource: public Kwave::StreamObject
    {
        Q_OBJECT
    public:

        /**
         * Constructor
         *
         * @param parent a parent object, passed to QObject (optional)
         */
        explicit SampleSource(QObject *parent = nullptr);

        /** Destructor */
        virtual ~SampleSource();

        /**
         * Each KwaveSampleSource has to derive this method for producing
         * sample data. It then should emit a signal like this:
         * "output(SampleArray data)"
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
        virtual bool done() const { return false; }

    };
}

#endif /* SAMPLE_SOURCE_H */

//***************************************************************************
//***************************************************************************
