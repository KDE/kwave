/**
 *
 * Thread.cpp -- wrapper class for making methods asynchronously callable
 *
 * \file mt/Thread.cpp
 *
 * Thanks to Carlos O'Ryan (coryan@cs.wustl.edu) for elucidating
 * the Daemon_Adapter pattern on which this is closely based.
 *
 * \author  Oliver M. Kellogg <Oliver.Kellogg@vs.dasa.de> (initial)
 * \author  Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de> (ported to Kwave)
 * \date    1998-08-12 (initial)
 * \date    2000-09-30 (ported to the Kwave project)
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
#include <error.h>        // for strerror()
#include <time.h>         // for clock()
#include <qapplication.h> // for warning()

#include "mt/TSS_Object.h"
#include "mt/MutexGuard.h"
#include "mt/Mutex.h"
#include "mt/Thread.h"

/**
 * C wrapper function for Thread::thread_adapter.
 * @internal
 */
extern "C" void* C_thread_adapter(void* arg)
{
    Thread *thread = (Thread *)arg;
    ASSERT(thread);
    if (!thread) return 0;

    /* call the thread's function through a C++ adapter */
    void* result = thread->thread_adapter(arg);
    return result;
}

//***************************************************************************
Thread::Thread(int */*grpid*/, const long /*flags*/)
    :TSS_Object(), m_tid((pthread_t)-1), m_lock("thread"),
    m_thread_running("thread_running")
{
    MutexGuard lock(m_lock);
    int res;
    res = pthread_attr_init(&m_attr);
    if (res)
        warning("Thread::Thread(): initializing thread attributes failed: %s",
	strerror(res));

    res = pthread_attr_setdetachstate(&m_attr, PTHREAD_CREATE_DETACHED);
    if (res)
	warning("Thread::Thread(): setting thread detach state failed: %s",
	strerror(res));
}

//***************************************************************************
Thread::~Thread()
{
    if (this->running()) {
	debug("Thread::~Thread(): waiting for normal shutdown");
	wait(100);
	debug("Thread::~Thread(): stopping");
	stop();
    }

    int res = pthread_attr_destroy(&m_attr);
    if (res)
	warning("Thread::~Thread(): destruction of attributes failed: %s",
	strerror(res));

//    debug("Thread::~Thread(): done.");
}

//***************************************************************************
void *Thread::thread_adapter(void *arg)
{
    MutexGuard lock(m_thread_running);

    Thread *object = (Thread *) arg;
    ASSERT(object);
    if (!object) return (void*)-EINVAL;

    /* execute the thread function */
    object->run();
    return arg;
}

//***************************************************************************
int Thread::start()
{
    MutexGuard lock(m_lock);
    int res = pthread_create(&m_tid, &m_attr, C_thread_adapter, this);
    if (res)
	warning("Thread::start(): thread creation failed: %s",
	strerror(res));
    return res;
}

//***************************************************************************
int Thread::stop()
{
    MutexGuard lock(m_lock);
    if (!running()) return 0; // already down

    debug("Thread::stop(): canceling thread");
    int res = pthread_cancel(m_tid);
    if (res) warning("Thread::stop(): thread cancel failed: %s",
	strerror(res));

    // wait some time until it is really done
    wait(500);

    return res;
}

//***************************************************************************
bool Thread::running()
{
    return m_thread_running.locked();
}

//***************************************************************************
void Thread::wait(unsigned int milliseconds)
{
    double elapsed_ms = 0.0;
    time_t t_start = clock();

    while (running() && (elapsed_ms < milliseconds)) {
	sched_yield();
	elapsed_ms = (((double)(clock()-t_start))/CLOCKS_PER_SEC)*1000.0;
    }

    if (/*still */running()) {
	warning("Thread::wait(): timed out after %d ms!", milliseconds);
    }
}

//***************************************************************************
pthread_t Thread::threadID()
{
    return m_tid;
}

//***************************************************************************
//***************************************************************************
/* end of Thread.cpp */
