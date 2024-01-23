/***************************************************************************
            FixedPool.h  -  simple pool with fixed number of elements
                             -------------------
    begin                : Thu Jan 02 2014
    copyright            : (C) 2014 by Thomas Eschenbacher
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

#ifndef FIXED_POOL_H
#define FIXED_POOL_H

#include "config.h"

#include <QMutex>
#include <QMutexLocker>
#include <QQueue>
#include <QSemaphore>

namespace Kwave
{
    template<unsigned int SIZE, class T> class FixedPool
    {
    public:
        /** Constructor */
        FixedPool()
            :m_storage(), m_free_queue(), m_sem_free(0), m_lock()
        {
            for (unsigned int i = 0; i < SIZE; ++i)
                release(&(m_storage[i]));
        }

        /** Destructor */
        virtual ~FixedPool() { }

        /**
         * Allocate an element from the pool
         * @return pointer to an element
         */
        T *allocate() {
            m_sem_free.acquire();
            {
                QMutexLocker _lock(&m_lock);
                return m_free_queue.dequeue();
            }
        }

        /**
         * Release an element and put it back into the pool
         * @param element pointer to an element
         */
        void release(T *element) {
            QMutexLocker _lock(&m_lock);
            m_free_queue.enqueue(element);
            m_sem_free.release();
        }

    private:
        /** array used as storage for the pool elements */
        T m_storage[SIZE];

        /** queue with free elements */
        QQueue<T *> m_free_queue;

        /** semaphore for counting available elements */
        QSemaphore m_sem_free;

        /** lock for protecting the m_free_queue */
        QMutex m_lock;
    };
}

#endif /* FIXED_POOL_H */

//***************************************************************************
//***************************************************************************
