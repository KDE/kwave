/***************************************************************************
             MutexSet.h  -  set of mutexes
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

#ifndef _MUTEX_SET_H_
#define _MUTEX_SET_H_

#include "config.h"
#include <qptrlist.h>
#include "mt/TSS_Object.h"

class Mutex;
class MutexGuard;

/**
 * @class MutexSet
 * Holds a set of mutexes and locks by locking each added mutex through
 * a mutex guard. If the mutex set is deleted, it automatically releases
 * all mutexes.
 */
class MutexSet: public TSS_Object
{
public:
    /** Default constructor */
    MutexSet();

    /** Destructor, releases all mutexes */
    virtual ~MutexSet();

    /**
     * Takes over all mutexes from an existing mutex set.
     * Use this with care!
     */
    void takeOver(MutexSet &set);

    /**
     * Creates a mutex guard and lock the passed mutex.
     * @param mutex the mutex to be locked.
     * @return true if ok, false if not enough memory
     */
    bool addLock(Mutex &mutex);

protected:

    /** list of mutex guards */
    QPtrList<MutexGuard> m_guards;
};

#endif /* _MUTEX_SET_H_ */
