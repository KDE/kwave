/***************************************************************************
       ThreadCondition.h -  interface for POSIX type thread conditions
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

#ifndef _THREAD_CONDITION_H_
#define _THREAD_CONDITION_H_

#include <pthread.h>

/**
 * @class ThreadCondition
 * Implements an interface for POSIX type thread conditions.
 *
 * @see the glibc info page
 *
 * @warning If it might happen that notify() might be called before wait(),
 *          the program will run into a deadlock. In this case you should
 *          consider using a Semaphore instead!
 *
 * @todo some error handling
 */
class ThreadCondition
{
public:
    /** Constructor */
    ThreadCondition();

    /** Destructor */
    virtual ~ThreadCondition();

    /**
     * Waits by setting the calling the calling thread into suspended
     * state until the condition has been set by some other thread.
     */
    void wait();

    /**
     * Sets the condition and wakes up at <b>only one</b> of the threads
     * that are waiting for the condition and suspended.
     */
    void notify();

    /**
     * Sets the condition and wakes up <b>all</em> threads that are currently
     * waiting for the condition and are suspended.
     */
    void notifyAll();

private:

    /** the internal thread condition */
    pthread_cond_t m_condition;

    /** lock used for setting the thread condition to a different value */
    pthread_mutex_t m_lock;

};

#endif /* _THREAD_CONDITION_H_ */
