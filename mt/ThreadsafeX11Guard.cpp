/***************************************************************************
   ThreadsafX11Guard.cpp -  guard for using X11 from a worker thread
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

#include <qapplication.h> // for flushX and syncX

#include "mt/Mutex.h"
#include "mt/MutexGuard.h"
#include "mt/Semaphore.h"
#include "mt/ThreadsafeX11Guard.h"

//***************************************************************************

/** Static initializer for the global/static X11 lock */
Mutex ThreadsafeX11Guard::m_lock_X11;

//***************************************************************************
ThreadsafeX11Guard::ThreadsafeX11Guard()
    :TSS_Object(), m_sem_x11_locked(), m_sem_x11_done(),
     m_sem_x11_unlocked(), m_spx_X11_request(this, SLOT(lockX11()))
{
    // activate the X11 thread
    m_spx_X11_request.AsyncHandler();

    // wait until the X11 thread is locked and suspended
    m_sem_x11_locked.wait();
}

//***************************************************************************
ThreadsafeX11Guard::~ThreadsafeX11Guard()
{
    // let the X11 thread continue
    m_sem_x11_done.post();

    // wait until X11 is unlocked again
    m_sem_x11_unlocked.wait();
}

//***************************************************************************
void ThreadsafeX11Guard::lockX11()
{
    // lock the global/static X11 mutex
    m_lock_X11.lock();

    // be sure there are no other X11 transactions we interrupt
    // so better give them a chance to complete within the
    // context of our thread
    QApplication::flushX();
    QApplication::syncX();

    // notify the waiting worker thread that X11 is locked now
    m_sem_x11_locked.post();

    // suspend until the worker thread is done with X11
    m_sem_x11_done.wait();

    // unlock X11 again
    m_lock_X11.unlock();

    // notify the worker thread that X11 is available/unlocked again
    m_sem_x11_unlocked.post();
}

//***************************************************************************
//***************************************************************************
