/***************************************************************************
                mt/Mutex.cpp  -  Synchronization by Mutal Exclusion
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

#include <pthread.h>
#include "mt/Mutex.h"

//***************************************************************************
Mutex::Mutex()
{
    pthread_mutex_init(&m_mutex, 0);
}

//***************************************************************************
Mutex::~Mutex()
{
    pthread_mutex_destroy(&m_mutex);
}

//***************************************************************************
void Mutex::lock()
{
    pthread_mutex_lock(&m_mutex);
}

//***************************************************************************
void Mutex::unlock()
{
    pthread_mutex_unlock(&m_mutex);
}

//***************************************************************************
//***************************************************************************
