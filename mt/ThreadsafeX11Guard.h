/***************************************************************************
    ThreadsafeX11Guard.h -  guard for using X11 from a worker thread
			     -------------------
    begin                : Sun Jun 03 2001
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

#ifndef _THREADSAFE_X11_GUARD_H_
#define _THREADSAFE_X11_GUARD_H_

#include "config.h"
#include <qmutex.h>
#include <qobject.h>

/**
 * @class ThreadsafeX11Guard
 * If a thread other than the program's main thread wants to access X11,
 * e.g. for doing some drawing or showing windows or dialog boxes, the
 * main thread has to be suspended until X11 output is done. This is
 * needed because <b>X11 currently is not threadsafe!</b>.
 *
 * @note This guard can be used multiple times in different contexts, it
 *       is able to handle recursion within the same thread. So you can
 *       use it as often as you want without having the danger of a
 *       deadlock.
 *
 * @warning Doing X11 output without this using this guard class normally
 *          leads to incomprehensible program crahes like bus errors or
 *          segmentation faults.
 */
class ThreadsafeX11Guard: public QObject
{
    Q_OBJECT

public:
    /** Constructor */
    ThreadsafeX11Guard();

    /** Destructor */
    virtual ~ThreadsafeX11Guard();

};

#endif /* _THREADSAFE_X11_GUARD_H_ */
