/***************************************************************************
     ThreadsafX11Guard.h -  guard for using X11 from a worker thread
			     -------------------
    begin                : Sun Jun 03 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#ifndef _THREADSAFE_X11_GUARD_H_
#define _THREADSAFE_X11_GUARD_H_

#include "mt/SignalProxy.h"
#include "mt/Semaphore.h"
#include "mt/TSS_Object.h"
#include "mt/Mutex.h"

/**
 * @class ThreadsafeX11Guard
 * If a thread other than the program's main thread wants to access X11,
 * e.g. for doing some drawing or showing windows or dialog boxes, the
 * main thread has to be suspended until X11 output is done. This is
 * needed because <b>X11 currently is not threadsafe!</b>.
 *
 * @warning Doing X11 output without this using this guard class normally
 *          leads to incomprehensible program crahes like bus errors or
 *          segmentation faults.
 */
class ThreadsafeX11Guard: public TSS_Object
{
    Q_OBJECT

public:
    /** Constructor */
    ThreadsafeX11Guard();

    /** Destructor */
    virtual ~ThreadsafeX11Guard();

private slots:

    /**
     * This slot is called in the context of the X11 thread and locks
     * X11 while the worker thread is active. The handshake process
     * with the worker thread is done via some semaphores.
     * @see m_sem_x11_locked
     * @see m_sem_x11_done
     * @see m_sem_x11_unlocked
     */
    void lockX11();

private:

    /** the global/unique lock for X11 */
    static Mutex m_lock_X11;

    /**
     * Semaphore set by lockX11() to signal that X11 has been locked and
     * the X11 thread will be suspended.
     */
    Semaphore m_sem_x11_locked;

    /**
     * Semaphore set by the worker thread to signal the X11 thread that
     * is currently suspended in lockX11() that it can continue and
     * release X11 again.
     */
    Semaphore m_sem_x11_done;

    /**
     * Semaphore set by lockX11() to signal that X11 has been unlocked
     * again and the X11 thread is active again.
     */
    Semaphore m_sem_x11_unlocked;

    /**
     * The SignalProxy activates the X11 thread and lets it enter lockX11()
     */
    SignalProxy<void> m_spx_X11_request;

};

#endif /* _THREADSAFE_X11_GUARD_H_ */

