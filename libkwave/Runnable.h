/***************************************************************************
             Runnable.h  -  interface for classes using a WorkerThread
                             -------------------
    begin                : Sun Dec 30 2012
    copyright            : (C) 2012 by Thomas Eschenbacher
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

#ifndef RUNNABLE_H
#define RUNNABLE_H

class QVariant;

namespace Kwave
{
    class Runnable
    {
    public:

	/** Destructor */
	virtual ~Runnable() {}

	/** "run function" */
	virtual void run_wrapper(const QVariant &params) = 0;

    };
}

#endif /* RUNNABLE_H */

//***************************************************************************
//***************************************************************************
