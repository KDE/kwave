/***************************************************************************
         GlobalLock.cpp  -  application global lock
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

#include <config.h>

#include "libkwave/GlobalLock.h"

// static initializer
QMutex Kwave::GlobalLock::m_global_lock;

//***************************************************************************
Kwave::GlobalLock::GlobalLock()
{
    m_global_lock.lock();
}

//***************************************************************************
Kwave::GlobalLock::~GlobalLock()
{
    m_global_lock.unlock();
}

//*****************************************************************************
//*****************************************************************************
