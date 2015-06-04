/***************************************************************************
           GlobalLock.h  -  application global lock
			     -------------------
    begin                : Jan 02 2014
    copyright            : (C) 2014 by Thomas Eschenbacher
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

#ifndef GLOBAL_LOCK_H
#define GLOBAL_LOCK_H

#include "config.h"

#include <QtCore/QMutex>

#include <kdemacros.h>

namespace Kwave {

    /**
     * Wrapper for an application global lock.
     * Use with care!
     * (might be needed for protecting external non-threadsafe libraries)
     */
    class Q_DECL_EXPORT GlobalLock
    {
    public:
	/** Constructor, acquires the global lock */
	GlobalLock();

	/** Destructor, releases the global lock */
	virtual ~GlobalLock();

    private:
	/** global lock, use with care! */
	static QMutex m_global_lock;
    };
}

#endif /* GLOBAL_LOCK_H */

//*****************************************************************************
//*****************************************************************************
