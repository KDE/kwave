/***************************************************************************
           Semaphore.cpp -  interface for POSIX 1003.b semaphores
			     -------------------
    begin                : Sun Jun 03 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#include <stdio.h> // for perror(...)
#include <qobject.h> // for ASSERT(...)
#include "mt/Semaphore.h"

//***************************************************************************
Semaphore::Semaphore()
    :TSS_Object()
{
    sem_init(&m_semaphore, 0, 0);
}

//***************************************************************************
Semaphore::~Semaphore()
{
    sem_destroy(&m_semaphore);
}

//***************************************************************************
void Semaphore::post()
{
    int result = sem_post(&m_semaphore);
    ASSERT(result == 0);
    if (result) perror(strerror(result));
}

//***************************************************************************
void Semaphore::wait()
{
    sem_wait(&m_semaphore);
}

//***************************************************************************
int Semaphore::trywait()
{
    return sem_trywait(&m_semaphore);
}

//***************************************************************************
//***************************************************************************
