/***************************************************************************
    libkwave/PluginWorkerThread.h  -  worker thread for Kwave plugins
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

#ifndef _PLUGIN_WORKER_THREAD_H_
#define _PLUGIN_WORKER_THREAD_H_

#include "config.h"
#include <pthread.h>

#include <QMutex>
#include <QObject>
#include <QStringList>
#include <QThread>

#include <kdemacros.h>

namespace Kwave
{
    class Plugin;

    class KDE_EXPORT PluginWorkerThread : public QThread
    {
	Q_OBJECT
    public:

	/** Constructor */
	PluginWorkerThread(Kwave::Plugin *plugin, QStringList params);

	/** Destructor, calls stop() if the thread is still running. */
	virtual ~PluginWorkerThread();

	/** Starts the thread's execution. */
	virtual void start();

	/**
	 * Stops the thread execution. Please note that you <b>MUST</b> call
	 * this function at the end if you derived a class from this one.
	 * @param timeout the timeout in milliseconds, default = 10s
	 * @return zero if successful or an error code if failed
	 * @see errno.h
	 */
	virtual int stop(unsigned int timeout = 10000);

	/**
	 * A wrapper for the run() function, calls the run(...) function
	 * of m_plugin with the parameters stored in m_params.
	 */
	virtual void run();

	/**
	* Returns true if the thread should stop. Should be polled
	* by the thread's run() function to wait for a termination
	* signal.
	*/
	bool shouldStop();

    private:

	/** pointer to the Kwave plugin that has a run() function */
	Kwave::Plugin *m_plugin;

	/** parameter list passed to the m_plugin's run() function */
	QStringList m_params;

	/** Mutex to control access to the thread itself */
	QMutex m_lock;

	/** Mutex for protecting SIGHUP <-> thread exit */
	QMutex m_lock_sighup;

	/** set to signal the thread that it should stop */
	bool m_should_stop;

	/**
	 * POSIX compatible thread ID of the worker thread.
	 * only needed and only valid while the thread is running.
	 * (needs a POSIX 1003.1-2001 system libc)
	 */
	pthread_t m_tid;

	/** POSIX compatible thread ID of the owner thread. */
	pthread_t m_owner_tid;

    };

}

#endif /* _PLUGIN_WORKER_THREAD_H_ */

//***************************************************************************
//***************************************************************************
