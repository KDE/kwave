/***************************************************************************
           SharedLock.h  -  Threadsafe lock for shared and exclusive access
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

#ifndef _SHARED_LOCK_H_
#define _SHARED_LOCK_H_

#include "config.h"
#include "mt/TSS_Object.h"
#include "mt/Mutex.h"
#include "mt/ThreadCondition.h"

/**
 * @class SharedLock
 * A SharedLock allows controlling the access to a resource in shared and
 * exclusive mode. In comparism to a Mutex, it is allowed to lock the
 * resource concurrently multiple times with shared access, but only once
 * at a time with exclusive access.
 * This is useful for implementing read/write access control - use shared
 * mode for read-only access and exclusive access for read/write access.
 * @note This class should not be used directly, it is safet to use it
 *       via a SharedLockGuard !
 * @see SharedLockGuard
 * @see Mutex
 */
class SharedLock: public TSS_Object
{
public:

    /** Constructor. */
    SharedLock();

    /** Destructor. */
    virtual ~SharedLock();

    /**
     * Locks in shared or in exclusive mode. This blocks the current
     * thread until the resource is available / unlocked.
     * @param exclusive if true locks in exclusive mode,
     *                  or shared mode if false
     */
    inline void lock(bool exclusive) {
        exclusive ? lock_exclusive() : lock_shared();
    };

    /**
     * Unlocks the resource from shared or exclusive mode.
     * @param exclusive if true unlocks from exclusive mode,
     *                  or shared mode if false
     */
    inline void unlock(bool exclusive) {
        exclusive ? unlock_exclusive() : unlock_shared();
    };

private:

    /**
     * Locks in exclusive mode.
     * @internal
     */
    void lock_exclusive();

    /**
     * Unlocks from shared mode.
     * @internal
     */
    void lock_shared();

    /**
     * Unlocks from exclusive mode.
     * @internal
     */
    void unlock_exclusive();

    /**
     * Unlocks from shared mode.
     * @internal
     */
    void unlock_shared();

    /** Number of concurrent shared locks */
    unsigned int m_shared_count;

    /** Mutex for exclusively locking and some internal critical regions */
    Mutex m_lock_exclusive;

    /**
     * ThreadCondition for synchronization of threads that are waiting
     * for shared or exclusive access.
     */
    ThreadCondition m_lock_changed;

};

#endif /* _SHARED_LOCK_H_ */
