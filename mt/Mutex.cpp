/***************************************************************************
                mt/Mutex.cpp  -  Synchronization by Mutal Exclusion
                             -------------------
    begin                : Tue Oct 3 2000
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

#include "config.h"
#include <pthread.h>
#include <errno.h>
#include <error.h>
#include <string.h>  // for strerror()
#include <qglobal.h> // for qWarning()

#ifdef DEBUG
#include <execinfo.h> // for backtrace()
#endif

#include "mt/Mutex.h"

//***************************************************************************
Mutex::Mutex(const char *name)
    :m_name(name), m_locked_by(0)
{
    int ret = pthread_mutex_init(&m_mutex, 0);
    if (ret) qWarning("Mutex::Mutex(): mutex creation failed: %s",
	strerror(ret));
}

//***************************************************************************
Mutex::~Mutex()
{
    if (locked()) {
	qWarning("Mutex::~Mutex(): destroying locked mutex!");

#ifdef DEBUG
	qDebug("pthread_self()=%08X", (unsigned int)pthread_self());
	void *buf[256];
	size_t n = backtrace(buf, 256);
	backtrace_symbols_fd(buf, n, 2);
#endif

	unlock();
    }

    int ret = pthread_mutex_destroy(&m_mutex);
    if (ret) qWarning("Mutex::~Mutex(): mutex destruction failed: %s",
	strerror(ret));
}

//***************************************************************************
int Mutex::lock()
{
    if (pthread_self() == m_locked_by) {
	qWarning("Mutex::lock() RECURSIVE CALL => DANGER OF DEADLOCK !!!");
#ifdef DEBUG
	qDebug("pthread_self()=%08X", (unsigned int)pthread_self());
	void *buf[256];
	size_t n = backtrace(buf, 256);
	backtrace_symbols_fd(buf, n, 2);
#endif
	return -EBUSY;
    }

    int ret = pthread_mutex_lock(&m_mutex);
    if (ret) qWarning("Mutex::lock(): lock of mutex failed: %s",
	strerror(ret));
    m_locked_by = pthread_self();
    return ret;
}

//***************************************************************************
int Mutex::unlock()
{
    int ret = pthread_mutex_unlock(&m_mutex);
    if (ret) qWarning("Mutex::unlock(): unlock of mutex failed: %s",
	strerror(ret));
    m_locked_by = (pthread_t)-1;
    return ret;
}

//***************************************************************************
bool Mutex::locked()
{
    // try to lock the mutex
    int ret = pthread_mutex_trylock(&m_mutex);

    // if it failed, it is busy
    if (ret == EBUSY) return true;
    if (ret) qWarning("Mutex::locked(): getting mutex state failed: %s",
	strerror(ret));

    // if it succeeded, unlock the mutex, we didn't really want to
    // lock it !
    ret = pthread_mutex_unlock(&m_mutex);
    if (ret) qWarning("Mutex::locked(): unlock of mutex failed: %s",
	strerror(ret));

    return false;
}

//***************************************************************************
void Mutex::setName(const char *name)
{
    m_name = name;
}

//***************************************************************************
const char *Mutex::name()
{
    return m_name;
}

//***************************************************************************
//***************************************************************************
