/***************************************************************************
		  SignalProxy.h  - threadsafe proxy for signals/slots
			     -------------------
    begin                : Mon Sep 11 2000
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

#ifndef _SIGNAL_PROXY_H_
#define _SIGNAL_PROXY_H_

#include <qqueue.h>
#include "mt/AsyncSync.h"
#include "mt/Mutex.h"
#include "mt/MutexGuard.h"

//***************************************************************************
template <class T>
class SignalProxy: public AsyncSync
{
public:
    SignalProxy(QObject *owner, const char *slot);
};

//***************************************************************************
template <class T>
SignalProxy<T>::SignalProxy(QObject *owner, const char *slot)
    :AsyncSync()
{
    QObject::connect(this, SIGNAL(Activated()), owner, slot);
}

//***************************************************************************
//***************************************************************************
/**
 * \class SignalProxy1
 * Provides a way of emitting signals that contain some data from inside
 * a thread out to the GUI thread (X11). Each event will be put into a
 * queue from within a thread and read out from the X11 side.
 * If a limit is specified, the content will be limited to the specified
 * number of entries, the oldest entry will be removed and deleted.
 *
 * This class is completely threadsafe !
 */
template <class T>
class SignalProxy1: public AsyncSync
{
public:
    SignalProxy1(QObject *owner, const char *slot, unsigned int limit=0);
    virtual ~SignalProxy1();
    virtual void enqueue(const T &param);
    virtual T *dequeue();
    virtual unsigned int count();
    virtual void setLimit(unsigned int limit);
    virtual unsigned int limit();
private:
    QQueue<T> m_queue;
    Mutex m_lock;
    unsigned int m_limit;
};

//***************************************************************************
template <class T>
SignalProxy1<T>::SignalProxy1(QObject *owner,
    const char *slot, unsigned int limit)
    :AsyncSync(), m_lock("SignalProxy1")
{
    m_limit = limit;
    m_queue.setAutoDelete(false);
    QObject::connect(this, SIGNAL(Activated()), owner, slot);
}

//***************************************************************************
template <class T>
SignalProxy1<T>::~SignalProxy1()
{
    m_queue.setAutoDelete(true);
    m_queue.clear();
}

//***************************************************************************
template <class T>
void SignalProxy1<T>::enqueue(const T &param)
{
    MutexGuard lock(m_lock);
    bool call_async = true;

    // dequeue the first (oldest) object of the queue if the limit
    // has been reached.
    if ((m_limit) && (m_queue.count() >= m_limit)) {
	T *p1 = m_queue.dequeue();
	ASSERT(p1);
	if (p1) delete p1;

	// if the queue already was full, don't call the AsyncHandler()
	// once more, the X11 synchronization pipe is already filled
	// with the correct number of events
	call_async = false;
    }

    // enqueue a copy of the object
    T *copy = new T(param);
    ASSERT(copy);
    m_queue.enqueue(copy);

    if (call_async) AsyncHandler();
}

//***************************************************************************
template <class T>
T *SignalProxy1<T>::dequeue()
{
    MutexGuard lock(m_lock);
    T *p1 = m_queue.dequeue();
    ASSERT(p1);
    T *copy = 0;
    if (p1) {
	copy = new T(*p1);
	ASSERT(copy);
	delete p1;
    }
    return copy;
}

//***************************************************************************
template <class T>
unsigned int SignalProxy1<T>::count()
{
    MutexGuard lock(m_lock);
    return m_queue.count();
}

//***************************************************************************
template <class T>
void SignalProxy1<T>::setLimit(unsigned int limit)
{
    volatile MutexGuard lock(m_lock);
    m_limit = limit;
}

//***************************************************************************
template <class T>
unsigned int SignalProxy1<T>::limit()
{
    MutexGuard lock(m_lock);
    return m_limit;
}

#endif //  _SIGNAL_PROXY_H_

//***************************************************************************
//***************************************************************************
