/**
 *
 * Thread.cpp -- wrapper class for making methods asynchronously callable
 *
 * \file mt/Thread.cpp
 *
 * Thanks to Carlos O'Ryan (coryan@cs.wustl.edu) for elucidating
 * the Daemon_Adapter pattern on which this is closely based.
 *
 * \author  Oliver M. Kellogg <Oliver.Kellogg@vs.dasa.de>
 * \author  Thomas Eschenbacher
 * \date    12.08.1998, Oct. 1998
 *
 * \par NOTE:
 *      this code is taken from the following news article 
 *      (found by www.dejanews.com):
 *
 * \verbatim
 * ----------------------------------------------------------------------------
 * Author: Oliver M. Kellogg <Oliver.Kellogg@vs.dasa.de>
 * Date:   1998/08/12
 * Forums: comp.soft-sys.ace
 * ----------------------------------------------------------------------------
 * Subject: Re: [ace-users] Handing non-static methods into ACE_Thread::spawn()
 * From: ace-users@cs.wustl.edu
 * Date: 1998/08/12
 * Newsgroups: comp.soft-sys.ace
 * ----------------------------------------------------------------------------
 * \endverbatim
 *
 * \bug     C++ exception handling does not work across additionally spawned
 *          client threads of this daemon.
 *
 * \bug     C signal handling is not done, a setlongjmp environment has to
 *          be set up.
 */

#include "config.h"
#include <errno.h>

#include "mt/TSS_Object.h"
#include "mt/Thread.h"

/**
 * C wrapper function for Thread::thread_adapter.
 * @internal
 */
extern "C" void* C_thread_adapter(void* arg)
{
    Thread *thread = (Thread *)arg;
    if (!thread) return 0;

    /* call the daemon's function */
    void* result = thread->thread_adapter(arg);
    return result;
}

//***************************************************************************
Thread::Thread(int *grpid, const long flags)
    :TSS_Object(), m_tid(0)
{
    pthread_attr_init(&m_attr);
    pthread_attr_setdetachstate(&m_attr, PTHREAD_CREATE_DETACHED);
}

//***************************************************************************
Thread::~Thread()
{
    pthread_attr_destroy(&m_attr);
}

//***************************************************************************
void *Thread::thread_adapter (void *arg)
{
    Thread *object = (Thread *) arg;
    if (!object) return (void*)-EINVAL;

    /* execute the thread function */
    object->run();
    return arg;
}

//***************************************************************************
void Thread::start()
{
    pthread_create(&m_tid, &m_attr, C_thread_adapter, this);
}

//***************************************************************************
void Thread::stop()
{
    pthread_cancel(m_tid);
}

//***************************************************************************
//***************************************************************************
/* end of Thread.cpp */
