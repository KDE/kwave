/***************************************************************************
      SharedLockGuard.h  -  Simple guard class for shared/exclusive locks
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

#ifndef _SHARED_LOCK_GUARD_H_
#define _SHARED_LOCK_GUARD_H_

#include "mt/SharedLock.h"

class SharedLock;

/**
 * @class SharedLockGuard
 * Simple guard class for shared/exclusive locks.
 * @see SharedLock
 */
class SharedLockGuard: public TSS_Object
{
public:

    /**
     * Constructor. Locks a SharedLock in shared or exclusive mode.
     * @param lock reference to the SharedLock
     * @param exclusive if true, the lock will be obtained in exclusive mode,
     *                  if false in shared mode (default) [optional]
     */
    SharedLockGuard(SharedLock &lock, bool exclusive = false);

    /** Destructor, unlocks. */
    virtual ~SharedLockGuard();

private:

    /** reference to the SharedLock, stored for use in the destructor */
    SharedLock &m_lock;

    /** shared/exclusive mode, needed in the destructor for unlocking */
    bool m_exclusive;

};

#endif /* _SHARED_LOCK_GUARD_H_ */
