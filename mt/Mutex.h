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

#include "config.h"
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
    Mutex(const char *name = 0);

    /** Destructor, destroys the lock */
    virtual ~Mutex();

    /**
     * Locks the mutex.
     * @return zero if successful or an error code if failed
     * @see errno.h
     */
    virtual int lock();

    /**
     * Unlocks the mutex.
     * @return zero if successful or an error code if failed
     * @see errno.h
     */
    virtual int unlock();

    /**
     * returns the lock state of the mutex
     * @return true if the mutex is locked, false if it is not locked or
     *         an error has occurred
     */
    virtual bool locked();

    /**
     * Sets a (new) name for this mutex. <b>NOTE:</b> this function is
     * not threadsafe and should only be called on initialization !
     * @param name pointer to a const string, 0 is allowed
     */
    void setName(const char *name);

    /** returns the name of the mutex */
    const char *name();

private:

    /** The mutex handle internally used */
    pthread_mutex_t m_mutex;

    /** Name of the mutex, optional but fine for debugging */
    const char *m_name;

    /** ID of the thread that currently owns the mutex */
    pthread_t m_locked_by;

};

#endif /* _MUTEX_H_ */
