/***************************************************************************
                  mt/Thread.h  -  Representation of a POSIX thread
                             -------------------
    begin                : Fri Sep 15 2000
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

#ifndef _THREAD_H_
#define _THREAD_H_

#include "config.h"
#include <pthread.h>       // for POSIX threads, included in libc > 2.0
#include <qmutex.h>
#include <qobject.h>
#include "mt/TSS_Object.h"

class Thread : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor. Initializes the thread's attributes and the thread's
     * group membership.
     * @param grpid pointer to an integer that receives or contains a
     *        group id (optional):
     *        \arg If the pointer is not null and the content of the
     *             integer it points to is -1, a new thread group will
     *             be created and the content will be set to it's id.
     *        \arg If the pointer is not null and the content of the
     *             integer is greater than zero it is assumed to be a
     *             group id and the thread will be appended to it.
     *        \arg If the pointer is null or the content is zero, no
     *             group will be created and the thread will not be
     *             appended to any group.
     *        \bug this parameter is currently ignored !
     *
     * @param flags the flags for thread creation, like defined in
     *              pthread.h.
     *        \bug currently ignored
     */
    Thread(int *grpid = 0, const long flags = PTHREAD_CREATE_DETACHED);

    /**
     * Destructor. Also calls stop() if the thread is currently running.
     */
    virtual ~Thread();

    /**
     * Starts the thread's execution.
     * @return zero if successful or an error code if failed
     * @see errno.h
     */
    int start();

    /**
     * Stops the thread execution. Please note that you <b>MUST</b> call
     * this function at the end if you derived a class from this one.
     * @param timeout the timeout in milliseconds, default = 10s
     * @return zero if successful or an error code if failed
     * @see errno.h
     */
    virtual int stop(unsigned int timeout = 10000);

    /**
     * The "run()" function of the thread. This is the function you
     * should overwrite to perform your thread's action.
     */
    virtual void run() = 0;

    /**
     * Returns true if the thread should stop. Should be polled
     * by the thread's run() function to wait for a termination
     * signal.
     */
    bool shouldStop();

    /**
     * Returns true if the thread is currently running
     */
    bool running();

    /**
     * Waits for termination of the thread with timeout. If the thread
     * is currently not running this function immediately returns.
     * @param milliseconds the time to wait in milliseconds, with
     *        precision of abt. 10ms under Linux. Don't use values
     *        below 200ms, the default is one second.
     */
    void wait(unsigned int milliseconds = 1000);

    /**
     * Returns the id of this thread.
     */
    pthread_t threadID();

    /**
     * wrapper to call the run() function, called internally
     * from the thread function with "C" linkage.
     * @internal
     */
    void *thread_adapter(void *arg);

protected:
    /** thread id */
    pthread_t m_tid;

private:

    /** thread attributes, like defined in pthread.h */
    pthread_attr_t m_attr;

    /** Mutex to control access to the thread itself */
    QMutex m_lock;

    /**
     * This mutex is locked as long as the thread is running and
     * is internally used for the running() function.
     */
    QMutex m_thread_running;

    /** set to signal the thread that it should stop */
    bool m_should_stop;

};

#endif /* _THREAD_H_ */

/* end of mt/Thread.h */
