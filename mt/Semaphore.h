/***************************************************************************
            Semaphore.h -  interface for POSIX 1003.b semaphores
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

#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

#include "config.h"
#include <pthread.h>
#include <semaphore.h>    // from POSIX threads / glibc
#include "mt/TSS_Object.h"

/**
 * @class Semaphore
 * Implements an interface for POSIX 1003.1b semaphores in glibc.
 *
 * @see the glibc info page
 *
 * @todo some error handling
 */
class Semaphore: public TSS_Object
{

public:

    /** Constructor */
    Semaphore();

    /** Destructor */
    virtual ~Semaphore();

    /**
     * Atomically increases the count of the semaphore. This function
     * never blocks. If a thread that is suspended in wait() it will
     * continue. If several threads are suspended, each call to this
     * function will wake up only one waiting thread.
     */
    void post();

    /**
     * Suspends the calling thread and waits until the semaphore has
     * non-zero count (someone calls post(). It then atomically decreases
     * the semaphore count and continues execution.
     * @note this is a cancellation point
     */
    void wait();

    /**
     * This is the non-blocking variant of wait(). If the semaphore
     * has non-zero count, the count is atomically decreased and the
     * function immediately returns 0. If the semaphore count is zero,
     * it immediately returns -1 and sets errno to `EAGAIN'.
     */
    int trywait();

private:

    /** the native semaphore */
    sem_t m_semaphore;

};

#endif /* _SEMAPHORE_H_ */
