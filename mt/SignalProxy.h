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
class SignalProxy: protected AsyncSync
{
public:
    SignalProxy(QObject *owner, const char *slot);
};

//***************************************************************************
template <class T>
SignalProxy<T>::SignalProxy(QObject *owner, const char *slot)
    :AsyncSync(owner)
{
    QObject::connect(this, SIGNAL(Activated()), owner, slot);
}

//***************************************************************************
//***************************************************************************
template <class T>
class SignalProxy1: protected AsyncSync
{
public:
    SignalProxy1(QObject *owner, const char *slot);
    virtual ~SignalProxy1();
    virtual void enqueue(T &param);
    virtual T *dequeue();
    virtual unsigned int count();
private:
    QQueue<T> m_queue;
    Mutex m_lock;
};

//***************************************************************************
template <class T>
SignalProxy1<T>::SignalProxy1(QObject *owner,
    const char *slot)
    :AsyncSync(), m_lock("SignalProxy1")
{
    m_queue.setAutoDelete(false);
    QObject::connect(this, SIGNAL(Activated()), owner, slot);
}

//***************************************************************************
template <class T>
SignalProxy1<T>::~SignalProxy1()
{
    MutexGuard lock(m_lock);

    m_queue.setAutoDelete(true);
    m_queue.clear();
}

//***************************************************************************
template <class T>
void SignalProxy1<T>::enqueue(T &param)
{
    MutexGuard lock(m_lock);

    T *copy = new T(param);
    ASSERT(copy);
    m_queue.enqueue(copy);
    AsyncHandler();
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

#endif //  _SIGNAL_PROXY_H_

//***************************************************************************
//***************************************************************************
