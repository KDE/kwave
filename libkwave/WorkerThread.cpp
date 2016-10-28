/***************************************************************************
    libkwave/WorkerThread.cpp  -  worker thread for Kwave
                             -------------------
    begin                : Sun Apr 06 2008
    copyright            : (C) 2008 by Thomas Eschenbacher
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

#include "config.h"

#include <signal.h>
#include <stdio.h>

#include <QApplication>
#include <QtGlobal> // for qWarning()

#undef DEBUG_FIND_DEADLOCKS
#ifdef DEBUG_FIND_DEADLOCKS
#include <execinfo.h> // for backtrace()
#endif
#include <errno.h>

#include "libkwave/Runnable.h"
#include "libkwave/WorkerThread.h"

/** maximum number of attempts to stop the worker thread */
#define MAX_ATTEMPTS_TO_STOP 8

/** set to true once a handler for SIGHUP is in place */
static bool g_signal_handler_is_in_place = false;

//***************************************************************************
extern "C" void _dummy_SIGHUP_handler(int);

//***************************************************************************
void _dummy_SIGHUP_handler(int)
{
    printf("\r\n--- SIGHUP ---\r\n");
}

//***************************************************************************
Kwave::WorkerThread::WorkerThread(Kwave::Runnable *runnable, QVariant params)
    :QThread(0),
     m_runnable(runnable),
     m_params(params),
     m_lock(), m_lock_sighup(),
     m_should_stop(0),
     m_tid(pthread_self()),
     m_owner_tid(pthread_self())
{
    /* NOTE: we assume that this gets called from the GUI thread only */
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(this->thread() == qApp->thread());

    /* install handler for SIGHUP */
    if (!g_signal_handler_is_in_place) {
	/*
	 * NOTE: the old signal handler is lost. But as long as we do not use
	 *       SIGHUP in any other place of the application and we need to
	 *       have a dummy handler only, that should not matter
	 */
	signal(SIGHUP, _dummy_SIGHUP_handler);
	g_signal_handler_is_in_place = true;
    }
}

//***************************************************************************
Kwave::WorkerThread::~WorkerThread()
{
    if (isRunning()) {
	qDebug("WorkerThread::~WorkerThread(): waiting for normal shutdown");
	wait(2000);
	qDebug("WorkerThread::~WorkerThread(): stopping");
	stop(2000);
    }
    Q_ASSERT(!isRunning());
}

//***************************************************************************
void Kwave::WorkerThread::start()
{
    QMutexLocker lock(&m_lock);

    // reset the "should stop" command flag
    m_should_stop = 0;

    QThread::start();
}

//***************************************************************************
int Kwave::WorkerThread::stop(unsigned int timeout)
{
    QMutexLocker lock(&m_lock);
    if (!isRunning()) return 0; // already down

    if (timeout < 1000) timeout = 1000;

    // set the "should stop" flag
    m_should_stop = 1;

    // send one SIGHUP in advance
    {
	QMutexLocker _lock(&m_lock_sighup);
	if (!pthread_equal(m_tid, m_owner_tid))
	    pthread_kill(m_tid, SIGHUP);
	if (!isRunning()) return 0;
    }

    // try to stop cooperatively
    if (!isRunning()) return 0;
    wait(timeout/10);
    if (!isRunning()) return 0;

    // try to interrupt by HUP signal
    qWarning("WorkerThread::stop(): sending SIGHUP");
    for (unsigned int i = 0; i < MAX_ATTEMPTS_TO_STOP; i++) {
	{
	    QMutexLocker _lock(&m_lock_sighup);
	    if (!isRunning()) return 0;
	    if (!pthread_equal(m_tid, m_owner_tid))
		pthread_kill(m_tid, SIGHUP);
	}
	if (!isRunning()) return 0;
	wait(timeout/10);
	if (!isRunning()) return 0;
    }

#ifdef DEBUG_FIND_DEADLOCKS
    if (running()) {
	qDebug("WorkerThread::stop(): pthread_self()=%08X",
	       (unsigned int)pthread_self());
	void *buf[256];
	size_t n = backtrace(buf, 256);
	backtrace_symbols_fd(buf, n, 2);
    }
#endif

    qDebug("WorkerThread::stop(): canceling thread");
    terminate();

    return -1;
}

//***************************************************************************
void Kwave::WorkerThread::run()
{
    Q_ASSERT(m_runnable);
    if (!m_runnable) return;

    /* get the POSIX thread ID, needed for sending SIGHUP */
    {
	QMutexLocker _lock(&m_lock_sighup);
	m_tid = pthread_self();
    }

    /* call the run(...) function */
    m_runnable->run_wrapper(m_params);

    /* avoid sending any SIGHUP by setting the m_tid to "invalid" */
    {
	QMutexLocker _lock(&m_lock_sighup);
	m_tid = m_owner_tid;
    }
}

//***************************************************************************
void Kwave::WorkerThread::cancel()
{
    m_should_stop = 1;
}

//***************************************************************************
bool Kwave::WorkerThread::shouldStop()
{
    return (m_should_stop);
}

//***************************************************************************
//***************************************************************************
