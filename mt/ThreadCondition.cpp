/***************************************************************************
     ThreadCondition.cpp -  interface for POSIX type thread conditions
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

#include "mt/ThreadCondition.h"

//***************************************************************************
ThreadCondition::ThreadCondition()
{
    pthread_cond_init(&m_condition, 0);
    pthread_mutex_init(&m_lock, 0);
}

//***************************************************************************
ThreadCondition::~ThreadCondition()
{
    pthread_mutex_lock(&m_lock);
    pthread_cond_destroy(&m_condition);
    pthread_mutex_unlock(&m_lock);
    pthread_mutex_destroy(&m_lock);
}

//***************************************************************************
void ThreadCondition::wait()
{
    pthread_mutex_lock(&m_lock);
    pthread_cond_wait(&m_condition, &m_lock);
    pthread_mutex_unlock(&m_lock);
}

//***************************************************************************
void ThreadCondition::notify()
{
    pthread_mutex_lock(&m_lock);
    pthread_cond_signal(&m_condition);
    pthread_mutex_unlock(&m_lock);
}

//***************************************************************************
void ThreadCondition::notifyAll()
{
    pthread_mutex_lock(&m_lock);
    pthread_cond_broadcast(&m_condition);
    pthread_mutex_unlock(&m_lock);
}

//***************************************************************************
//***************************************************************************
