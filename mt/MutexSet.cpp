/***************************************************************************
           MutexSet.cpp  -  set of mutexes
			     -------------------
    begin                : Feb 11 2001
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

#include <qlist.h>

#include "mt/TSS_Object.h"
#include "mt/Mutex.h"
#include "mt/MutexGuard.h"
#include "mt/MutexSet.h"

//***************************************************************************
MutexSet::MutexSet()
    :TSS_Object(), m_guards()
{
    m_guards.setAutoDelete(true);
}

//***************************************************************************
MutexSet::~MutexSet()
{
    m_guards.setAutoDelete(true);
    while (m_guards.count()) {
	m_guards.remove(m_guards.count()-1);
    }
}

//***************************************************************************
void MutexSet::takeOver(MutexSet &set)
{
    set.m_guards.setAutoDelete(false);
    while (set.m_guards.count()) {
	MutexGuard *g = set.m_guards.first();
	m_guards.append(g);
	set.m_guards.removeRef(g);
    }
    set.m_guards.setAutoDelete(true);
}

//***************************************************************************
bool MutexSet::addLock(Mutex &mutex)
{
    MutexGuard *guard = new MutexGuard(mutex);
    ASSERT(guard);
    if (!guard) return false;

    m_guards.append(guard);
    return true;
}

//***************************************************************************
//***************************************************************************
