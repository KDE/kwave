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
#include "libgui/AsyncSync.h"

//***************************************************************************

//template <class T> class SignalProxy: protected AsyncSync
//{
//public:
//    SignalProxy(QObject *owner, const char *slot);
//};
//
//template <class T> SignalProxy<T>::SignalProxy(QObject *owner, const char *slot)
//    :AsyncSync(owner)
//{
//    QObject::connect(this, SIGNAL(Activated()), owner, slot);
//}

//***************************************************************************

template <class T> class SignalProxy1: protected AsyncSync
{
public:
    SignalProxy1(QObject *owner, const char *slot);
    virtual ~SignalProxy1();
    virtual void enqueue(T param);
    virtual T *dequeue();
private:
    QQueue<T> queue;
};

template <class T> SignalProxy1<T>::SignalProxy1(QObject *owner, const char *slot)
{
    queue.setAutoDelete(false);
    QObject::connect(this, SIGNAL(Activated()), owner, slot);
}

template <class T> SignalProxy1<T>::~SignalProxy1()
{
    queue.setAutoDelete(true);
    queue.clear();
}

template <class T> void SignalProxy1<T>::enqueue(T param)
{
    T *copy = new T(param);
    queue.enqueue(copy);
    AsyncHandler();
}

template <class T> T *SignalProxy1<T>::dequeue()
{
    return queue.dequeue();
}

#endif //  _SIGNAL_PROXY_H_

//***************************************************************************
//***************************************************************************
