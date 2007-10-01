/***************************************************************************
  ArtsMultiSource.h  -  base class for multi-track aRts compatible sources
                             -------------------
    begin                : Sun Dec 9 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#ifndef _ARTS_MULTI_SOURCE_H_
#define _ARTS_MULTI_SOURCE_H_

#include "config.h"
#ifdef HAVE_ARTS_SUPPORT

#include <arts/artsflow.h>

class ArtsMultiSource
{
public:

    /** Destructor */
    virtual ~ArtsMultiSource() {};

    /**
     * Returns a pointer to one of the aRts sample sources. (Used for
     * connecting to the internal aRts streams)
     * @param i index of the track [0...count-1]
     * @return pointer to an Arts::Object
     */
    virtual Arts::Object *operator [] (unsigned int i) = 0;

    /**
     * Should be overwritten in derived classes that act like sources
     * to return true if end of input has been reached.
     */
    virtual bool done() { return false; };

    /** @see Arts::StdSynthModule::start() */
    virtual void start() {};

    /** @see Arts::StdSynthModule::stop() */
    virtual void stop() {};
};

#else /* HAVE_ARTS_SUPPORT */
#warning aRts support is disabled
#endif /* HAVE_ARTS_SUPPORT */

#endif /* _ARTS_MULTI_SOURCE_H_ */
