/*************************************************************************
    KwaveSampleArray.h  -  array with Kwave's internal sample_t
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

#ifndef _KWAVE_SAMPLE_ARRAY_H_
#define _KWAVE_SAMPLE_ARRAY_H_

#include "config.h"

#include <qmemarray.h>
#include "libkwave/Sample.h"

namespace Kwave {
    /**
     * array with sample_t, for use in KwaveSampleSource, KwaveSampleSink
     * and other streaming classes.
     */
    typedef QMemArray<sample_t> SampleArray;
}

#endif /* _KWAVE_SAMPLE_ARRAY_H_ */
