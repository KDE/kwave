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
#include <qmutex.h>
#include <signal.h>
#include <sys/time.h>

#include "mt/Thread.h"

/** lock for protecting SIGHUP <-> thread exit */
static QMutex g_lock_sighup;

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
extern "C" void *C_thread_adapter(void* arg)
{
    Thread *thread = (Thread *)arg;
    Q_CHECK_PTR(thread);
    if (!thread) return 0;

    /* install handler for SIGHUP */
    sighandler_t old_handler = signal(SIGHUP, _dummy_SIGHUP_handler);

    /* call the thread's function through a C++ adapter */
    void *result = thread->thread_adapter(arg);

    /* restore previous signal handler */
    old_handler = signal(SIGHUP, old_handler);

    g_lock_sighup.unlock();
    return result;
}

//***************************************************************************
Thread::Thread(int */*grpid*/, const long /*flags*/)
    :QObject(), m_tid((pthread_t)-1), m_lock(),
    m_thread_running(), m_should_stop(false)
{
    QMutexLocker lock(&m_lock);
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
    QMutexLocker lock(&m_thread_running);

    Thread *object = (Thread *) arg;
    Q_CHECK_PTR(object);
    if (!object) {
	g_lock_sighup.lock();
	return (void*)-EINVAL;
    }

    /* execute the thread function */
    object->run();

    g_lock_sighup.lock();
    return arg;
}

//***************************************************************************
int Thread::start()
{
    QMutexLocker lock(&m_lock);

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
    QMutexLocker lock(&m_lock);
    if (!running()) return 0; // already down

    if (timeout < 1000) timeout = 1000;

    // set the "should stop" flag
    m_should_stop = true;

    // send one SIGHUP in advance
    {
	QMutexLocker lock_exit(&g_lock_sighup);
	if (!running()) return 0;
	pthread_kill(m_tid, SIGHUP);
    }

    // try to stop cooperatively
    if (!running()) return 0;
    wait(timeout/10);
    if (!running()) return 0;

    // try to interrupt by INT signal
    qDebug("Thread::stop(): sending SIGHUP");
    for (unsigned int i=0; i < 8; i++) {
	{
	    QMutexLocker lock_exit(&g_lock_sighup);
	    if (!running()) return 0;
	    pthread_kill(m_tid, SIGHUP);
	}
	if (!running()) return 0;
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
    struct timeval t1, t2;
    gettimeofday(&t1, 0);

    while (running() && (elapsed_ms < milliseconds)) {
	// just for fun...
	sched_yield();

	// sleep through select()
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 20*1000;
	select(0, 0, 0, 0, &tv);

	gettimeofday(&t2, 0);
	elapsed_ms = (double)t2.tv_sec  * 1.0E3 +
	             (double)t2.tv_usec / 1.0E3 -
	             (double)t1.tv_sec  * 1.0E3 -
	             (double)t1.tv_usec / 1.0E3;
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
