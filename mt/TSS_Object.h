/***************************************************************************
    TSS_Object.h  - base class for usage of TSS (supports asynchronous exits)
			     -------------------
    begin                : Sun Oct 01 2000
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

#ifndef _TSS_OBJECT_H_
#define _TSS_OBJECT_H_

#include <pthread.h>
#include <qobject.h>

/**
 * @class TSS_Object
 *
 * TSS_Object base class for usage of TSS, supports asynchronous exits
 * and cleans up the derived object if the thread that created it
 * is closed or exits without (having a chance for) cleaning up.
 *
 * Provides registration and a cleanup handler for objects in
 * thread-specific-storage (TSS). It should be inherited by all classes that
 * are only valid within a certain thread. The cleanup handler ensures
 * that the object will be removed if it´s owning thread died. For this,
 * the derived object must call the constructor and should have a
 * <b>virtual</b> destructor.
 *
 * @author Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de>
 * @date 2000-10-01
 *
 * @bug I disabled TSS because it led into too many problems. The cleanup
 *      handlers work nevertheless...
 */
class TSS_Object: public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor, creates the TSS key
     */
    TSS_Object();

    /**
     * Destructor, releases the TSS key
     */
    virtual ~TSS_Object();

private:
    /** thread specific key */
    pthread_key_t m_key;

    /** number of allocated instances */
    static unsigned int m_count;
};

#endif /* _TSS_OBJECT_H_ */

/* end of mt/TSS_Object.h */
