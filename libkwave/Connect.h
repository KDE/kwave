/*************************************************************************
    Connect.h  -  function for connecting Kwave streaming objects
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

#ifndef CONNECT_H
#define CONNECT_H

#include "config.h"
#include "libkwave_export.h"

#include <QtGlobal>


namespace Kwave
{

    class StreamObject;

    /**
     * Connect an output of a Kwave::SampleSource to the input
     * of a Kwave::SampleSink. The following combinations of
     * single-track and multi-track sources/sinks are allowed.:
     * \li single -> single (1:1)
     * \li single -> multi  (1:N)
     * \li multi  -> multi  (N:N)
     *
     * @param source a Kwave::SampleSource that produces data
     * @param output name of the output, must be formed with the
     *               Qt macro \c SIGNAL(...) \c out of
     *               a Qt style signal.
     * @param sink a Kwave::SampleSink that can receive data
     * @param input name of the inputm must be formed with the
     *              Qt macro \c SLOT(...) \c out of a Qt style
     *              (public) slot .
     * @return true if successful or false if either
     *         \li an invalid combination of single/multi track
     *             source/sink has been passed
     *         \li a source or sink's track is NULL (missing)
     *         \li input or output name is zero-length
     */
    bool connect(Kwave::StreamObject &source, const char *output,
                 Kwave::StreamObject &sink,   const char *input)
                 LIBKWAVE_EXPORT;
}

#endif /* CONNECT_H */

//***************************************************************************
//***************************************************************************
