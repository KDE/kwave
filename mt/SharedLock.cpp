/***************************************************************************
         SharedLock.cpp  -  Threadsafe lock for shared and exclusive access
			     -------------------
    begin                : Jun 14 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mt/SharedLock.h"

//***************************************************************************
//***************************************************************************
SharedLock::SharedLock()
    :TSS_Object(), m_shared_count(0), m_lock_exclusive(), m_lock_changed()
{
}

//***************************************************************************
SharedLock::~SharedLock()
{
    ASSERT(!m_shared_count);
}

//***************************************************************************
void SharedLock::lock_exclusive()
{
    // get exclusive access to m_shared_count, also makes sure
    // that we are not in an exclusively locked region
    m_lock_exclusive.lock();

    while (m_shared_count) {
	// still locked for shared access, unlock again and
	// wait for a change
	m_lock_exclusive.unlock();
	m_lock_changed.wait();
	
	// lock exclusively, will be unlocked in unlock_exclusive()
	m_lock_exclusive.lock();
    }
}

//***************************************************************************
void SharedLock::unlock_exclusive()
{
    // unlock from exclusive access and wake up all waiting threads
    m_lock_exclusive.unlock();
    m_lock_changed.notifyAll();
}

//***************************************************************************
void SharedLock::lock_shared()
{
    // get exclusive access to m_shared_count, also makes sure
    // that we are not in an exclusively locked region
    m_lock_exclusive.lock();

    // increase the number of concurrent shared locks
    m_shared_count++;

    // unlock m_shared_count again and allow further lock attempts
    m_lock_exclusive.unlock();
}

//***************************************************************************
void SharedLock::unlock_shared()
{
    // get exclusive access to m_shared_count, also makes sure
    // that we are not in an exclusively locked region
    m_lock_exclusive.lock();

    // reduce the number of concurrent shared locks
    ASSERT(m_shared_count);
    if (m_shared_count) m_shared_count--;

    // wake up all threads that are waiting for exclusive access
    // if this was the last active shared lock
    if (!m_shared_count) m_lock_changed.notifyAll();

    // unlock m_shared_count again and allow further lock attempts
    m_lock_exclusive.unlock();
}

//***************************************************************************
//***************************************************************************
