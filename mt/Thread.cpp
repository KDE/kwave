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
#include <error.h>   // for strerror()
#include <time.h>    // for clock()
#include <qglobal.h> // for qWarning()
#include <signal.h>

#include "mt/TSS_Object.h"
#include "mt/MutexGuard.h"
#include "mt/Mutex.h"
#include "mt/Thread.h"

//***************************************************************************
extern "C" void _dummy_SIGHUP_handler(int)
{
    printf("\r\n--- SIGHUP ---\r\n");
}

//***************************************************************************
/**
 * C wrapper function for Thread::thread_adapter.
 * @internal
 */
extern "C" void* C_thread_adapter(void* arg)
{
    Thread *thread = (Thread *)arg;
    Q_CHECK_PTR(thread);
    if (!thread) return 0;

    /* install handler for SIGHUP */
    sighandler_t old_handler = signal(SIGHUP, _dummy_SIGHUP_handler);

    /* call the thread's function through a C++ adapter */
    void* result = thread->thread_adapter(arg);

    /* restore previous signal handler */
    old_handler = signal(SIGHUP, old_handler);

    return result;
}

//***************************************************************************
Thread::Thread(int */*grpid*/, const long /*flags*/)
    :QObject(), TSS_Object(), m_tid((pthread_t)-1), m_lock("thread"),
    m_thread_running("thread_running"), m_should_stop(false)
{
    MutexGuard lock(m_lock);
    int res;
    res = pthread_attr_init(&m_attr);
    if (res)
        qWarning("Thread::Thread(): initializing thread attributes failed: %s",
	strerror(res));

    res = pthread_attr_setdetachstate(&m_attr, PTHREAD_CREATE_DETACHED);
    if (res)
	qWarning("Thread::Thread(): setting thread detach state failed: %s",
	strerror(res));
}

//***************************************************************************
Thread::~Thread()
{
    if (running()) {
	qDebug("Thread::~Thread(): waiting for normal shutdown");
	wait(2000);
	qDebug("Thread::~Thread(): stopping");
	stop(2000);
    }
    Q_ASSERT(!running());

    int res = pthread_attr_destroy(&m_attr);
    if (res)
	qWarning("Thread::~Thread(): destruction of attributes failed: %s",
	strerror(res));
}

//***************************************************************************
void *Thread::thread_adapter(void *arg)
{
    MutexGuard lock(m_thread_running);

    Thread *object = (Thread *) arg;
    Q_CHECK_PTR(object);
    if (!object) return (void*)-EINVAL;

    /* execute the thread function */
    object->run();
    return arg;
}

//***************************************************************************
int Thread::start()
{
    MutexGuard lock(m_lock);

    // reset the "should stop" command flag
    m_should_stop = false;

    int res = pthread_create(&m_tid, &m_attr, C_thread_adapter, this);
    if (res)
	qWarning("Thread::start(): thread creation failed: %s",
	strerror(res));
    return res;
}

//***************************************************************************
int Thread::stop(unsigned int timeout)
{
    MutexGuard lock(m_lock);
    if (!running()) return 0; // already down

    if (timeout < 1000) timeout = 1000;

    // try to stop cooperatively
    m_should_stop = true;
    wait(timeout/10);
    if (!running()) return 0;

    // try to interrupt by INT signal
    qDebug("Thread::stop(): sending SIGHUP");
    for (unsigned int i=0; i < 8; i++) {
	pthread_kill(m_tid, SIGHUP);
	wait(timeout/10);
	if (!running()) return 0;
    }

    qDebug("Thread::stop(): canceling thread");
    int res = pthread_cancel(m_tid);
    if (res && (res != ESRCH))
	qWarning("Thread::stop(): thread cancel failed: %s", strerror(res));

    // wait some time until it is really done
    wait(timeout/10);

    return res;
}

//***************************************************************************
bool Thread::shouldStop()
{
    return (m_should_stop);
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
//    if (/*still */running()) {
//	qWarning("Thread::wait(): timed out after %d ms!", milliseconds);
//    }
}

//***************************************************************************
pthread_t Thread::threadID()
{
    return m_tid;
}

//***************************************************************************
//***************************************************************************
/* end of Thread.cpp */
