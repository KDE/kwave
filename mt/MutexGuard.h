/***************************************************************************
    mt/MutexGuard.h  -  Guard class for Synchronization by Mutal Exclusion
                             -------------------
    begin                : Tue Oct 3 2000
    copyright            : (C) 2000 by Thomas Eschenbacher
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

#ifndef _MUTEX_GUARD_H_
#define _MUTEX_GUARD_H_

#include "mt/TSS_Object.h"

class Mutex;

/**
 * \class MutexGuard
 * Implements an easy-to-use guard class for a simple mutal
 * exclusive locking for multithreaded applications. This should be used
 * prior to directly using a Mutex because it also supports asynchronous
 * thread exits through the TSS_Object class.
 *
 * \author Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de>
 * \data 2000-10-03
 */
class MutexGuard: public TSS_Object
{
    Q_OBJECT
public:
    /**
     * Constructor, takes a Mutex object and automatically locks it
     * @param lock the mutex to be locked
     */
    MutexGuard(Mutex &lock);

    /**
     * Destructor, automatically releases (unlocks) the lock.
     */
    virtual ~MutexGuard();

private:
    /** the mutex to be locked */
    Mutex &m_lock;
};

#endif // _MUTEX_GUARD_H_

//***************************************************************************
