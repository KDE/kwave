/***************************************************************************
                  mt/Thread.h  -  Representation of a POSIX thread
                             -------------------
    begin                : Fri Sep 15 2000
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

#ifndef _THREAD_H_
#define _THREAD_H_

#include "config.h"

#include <QMutex>
#include <QObject>
#include <QThread>

class Thread : public QThread
{
    Q_OBJECT
public:

    /** Constructor */
    Thread(QObject *parent = 0);

    /** Destructor, calls stop() if the thread is still running. */
    virtual ~Thread();

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
     * The "run()" function of the thread. This is the function you
     * should overwrite to perform your thread's action.
     */
    virtual void run() = 0;

    /**
     * Returns true if the thread should stop. Should be polled
     * by the thread's run() function to wait for a termination
     * signal.
     */
    bool shouldStop();

private:

    /** Mutex to control access to the thread itself */
    QMutex m_lock;

    /** set to signal the thread that it should stop */
    bool m_should_stop;

};

#endif /* _THREAD_H_ */

/* end of mt/Thread.h */
