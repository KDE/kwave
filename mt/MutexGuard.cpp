/***************************************************************************
    mt/MutexGuard.h  -  Guard class for Synchronization by Mutal Exclusion
                             -------------------
    begin                : Tue Oct 3 2000
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

#include "mt/Mutex.h"
#include "mt/MutexGuard.h"

//***************************************************************************
MutexGuard::MutexGuard(Mutex &lock)
:TSS_Object(), m_lock(lock)
{
    m_lock.lock();
}

//***************************************************************************
MutexGuard::~MutexGuard()
{
    m_lock.unlock();
}

//***************************************************************************
//***************************************************************************
