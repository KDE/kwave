/***************************************************************************
                  mt/Mutex.h  -  Synchronization by Mutal Exclusion
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

#ifndef _MUTEX_H_
#define _MUTEX_H_

#include <pthread.h> // for POSIX threads, included in libc > 2.0

/**
 * \class Mutex
 * Implements simple mutal exclusive locking for multithreaded
 * applications. The best way to use this class is through a
 * MutexGuard that also uses TSS and thus is much safer.
 *
 * \author Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de>
 * \data 2000-10-03
 */
class Mutex
{
public:
    /** Constructor, initializes the lock */
    Mutex();

    /** Destructor, destroys the lock */
    virtual ~Mutex();

    /** locks the mutex */
    virtual void lock();

    /** unlocks the mutex */
    virtual void unlock();

private:
    /** the mutex handle internally used */
    pthread_mutex_t m_mutex;
};

#endif _MUTEX_H_
