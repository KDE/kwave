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

#include <pthread.h>
#include "mt/TSS_Object.h"

class Thread : public TSS_Object
{
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

    /** starts the thread's execution */
    void start();

    /**
     * Stops the thread execution. Please note that you <b>MUST</b> call
     * this function at the end if you derived a class from this one.
     */
    virtual void stop();

    /**
     * the "run()" function of the thread
     */
    virtual void run() = 0;


    /** Returns true if the thread runns in "standalone" mode */
    bool standalone() { return m_standalone; }

    void *thread_adapter (void *arg);

protected:
//    protected:                // only for class ModuleList
//
//    static map<ACE_thread_t, Daemon *> DaemonList; /*!< list of all daemons */
//    static ACE_Thread_Mutex lock_daemon_list_; /*! lock for the daemon list */
//    jmp_buf jmp_buffer;       /*!< buffer for longjmp */

    /** thread id */
    pthread_t m_tid;

private:

    /** thread attributes */
    pthread_attr_t m_attr;
    bool m_respawn;             /*!< true=restart if crashed */
    bool m_standalone;          /*!< true=no parent, self-deletion on exit */
//    int  fail_count_;         /*!< number of crashs [0...n] */
//    amr_time_t last_failed_;  /*!< time of last crash */
//
//    String name_;             /*!< the thread's name (normally owner module) */
};

#endif /* _THREAD_H_ */

/* end of mt/Thread.h */
