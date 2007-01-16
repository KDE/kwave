/***************************************************************************
    SharedLockGuard.cpp  -  Simple guard class for shared/exclusive locks
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
#include "mt/SharedLockGuard.h"

//***************************************************************************
SharedLockGuard::SharedLockGuard(SharedLock &lock, bool exclusive)
    :m_lock(lock), m_exclusive(exclusive)
{
    m_lock.lock(m_exclusive);
}

//***************************************************************************
SharedLockGuard::~SharedLockGuard()
{
    m_lock.unlock(m_exclusive);
}

//***************************************************************************
//***************************************************************************
