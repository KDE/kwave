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

//#include <setjmp.h>
//#include <map>
//#include <String.h>
//#include "sys/TimeBase.h"
//#include "Thread_Manager.h"
//#include "TSS_Object.h"

#include <pthread.h>

//class ModuleList; // forward to include/ModuleList.h
//class ModSyscon;  // forward to modules/sys/Syscon.h

// still a dummy implementation...
class Thread_Manager
{
};

class Thread //: virtual public TSS_Object
{
//    friend class ModuleList;
//    friend class modSyscon;

    public:

    Thread(bool respawn=false, bool standalone=false);
    virtual ~Thread();
    virtual void execute () = 0;

    // create a new thread and activate it
    pthread_t activate(Thread_Manager &thr_mgr, int &grpid,
		       const char *Name = 0,
                       long flags = PTHREAD_CREATE_DETACHED |
				    PTHREAD_CANCEL_ENABLE
		       );

    void* thread_adapter (void* arg);
    bool standalone() { return standalone_; }

//    protected:                // only for class ModuleList
//
//    static map<ACE_thread_t, Daemon *> DaemonList; /*!< list of all daemons */
//    static ACE_Thread_Mutex lock_daemon_list_; /*! lock for the daemon list */
//    jmp_buf jmp_buffer;       /*!< buffer for longjmp */
    pthread_t tid_;             /*!< thread id */

private:

    /** thread attributes */
    pthread_attr_t attr_;
    bool respawn_;            /*!< true=restart if crashed */
    bool standalone_;         /*!< true=no parent, self-deletion on exit */
//    int  fail_count_;         /*!< number of crashs [0...n] */
//    amr_time_t last_failed_;  /*!< time of last crash */
//
//    String name_;             /*!< the thread's name (normally owner module) */
};

#endif /* _THREAD_H_ */

/* end of mt/Thread.h */
