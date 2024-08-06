/*************************************************************************
       RecordThread.cpp  -  thread for lowlevel audio recording
                             -------------------
    begin                : Mon Oct 20 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
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

#include "config.h"

#include <errno.h>

#include <QMutexLocker>
#include <QVariant>

#include "RecordDevice.h"
#include "RecordThread.h"

//***************************************************************************
Kwave::RecordThread::RecordThread()
    :Kwave::WorkerThread(Q_NULLPTR, QVariant()),
     m_lock(),
     m_device(Q_NULLPTR),
     m_empty_queue(),
     m_full_queue(),
     m_buffer_count(0),
     m_buffer_size(0)
{
}

//***************************************************************************
Kwave::RecordThread::~RecordThread()
{
    stop();

    {
        QMutexLocker lock(&m_lock);
        m_full_queue.clear();
        m_empty_queue.clear();
    }
}

//***************************************************************************
void Kwave::RecordThread::setRecordDevice(Kwave::RecordDevice *device)
{
    Q_ASSERT(!isRunning());
    if (isRunning()) return;

    m_device = device;
}

//***************************************************************************
int Kwave::RecordThread::setBuffers(unsigned int count, unsigned int size)
{
    QMutexLocker lock(&m_lock);

    Q_ASSERT(!isRunning());
    if (isRunning()) return -EBUSY;

    // flush all queues
    m_full_queue.clear();
    m_empty_queue.clear();

    // fill the "empty" queue again
    QByteArray buf(size, 0x00);
    for (unsigned int i = 0; i < count; i++)
        m_empty_queue.enqueue(buf);

    // take the new settings
    m_buffer_size  = size;
    m_buffer_count = count;

    // return number of buffers or -ENOMEM if not even two allocated
    return (m_empty_queue.count() >= 2) ? m_empty_queue.count() : -ENOMEM;
}

//***************************************************************************
unsigned int Kwave::RecordThread::remainingBuffers()
{
    QMutexLocker lock(&m_lock);
    return (m_empty_queue.count());
}

//***************************************************************************
unsigned int Kwave::RecordThread::queuedBuffers()
{
    QMutexLocker lock(&m_lock);
    return (m_full_queue.count());
}

//***************************************************************************
QByteArray Kwave::RecordThread::dequeue()
{
    QMutexLocker lock(&m_lock);

    if (m_full_queue.count()) {
        // de-queue the buffer from the full list
        QByteArray buf = m_full_queue.dequeue();

        // put the buffer back to the empty list
        m_empty_queue.enqueue(buf);

        // return the buffer
        return buf;
    } else {
        // return an empty buffer
        return QByteArray();
    }
}

//***************************************************************************
void Kwave::RecordThread::run()
{
    int  result      = 0;
    bool interrupted = false;

    // read data until we receive a close signal
    while (!isInterruptionRequested() && !interrupted) {
        // dequeue a buffer from the "empty" queue

        QByteArray buffer;
        int        len;
        {
            QMutexLocker lock(&m_lock);

            if (m_empty_queue.isEmpty()) {
                // we had a "buffer overflow"
                qWarning("RecordThread::run() -> NO EMPTY BUFFER FOUND !!!");
                result = -ENOBUFS;
                break;
            }

            buffer  = m_empty_queue.dequeue();
            len     = buffer.size();
            Q_ASSERT(buffer.size());
            if (!len) {
                result = -ENOBUFS;
                break;
            }
        }

        // read into the current buffer
        unsigned int offset = 0;
        while (len && !interrupted && !isInterruptionRequested()) {
            // read raw data from the record device
            result =  (m_device) ?
                m_device->read(buffer, offset) : -EBADF;

            if ((result < 0) && (result != -EAGAIN))
                qWarning("RecordThread: read result = %d (%s)",
                         result, strerror(-result));

            if (result == -EAGAIN) {
                continue;
            } else if (result == -EBADF) {
                // file open has failed
                interrupted = true;
                break;
            } else if (result == -EINTR) {
                // thread was interrupted, received signal?
                interrupted = true;
                break;
            } else if (result < 1) {
                // something went wrong !?
                interrupted = true;
                qWarning("RecordThread::run(): read returned %d", result);
                break;
            } else {
                offset += result;
                len = buffer.size() - offset;
                Q_ASSERT(len >= 0);
                if (len < 0) len = 0;
            }
        }

        // return buffer into the empty queue and abort on errors
        // do not use it
        if (interrupted && (result < 0)) {
            QMutexLocker lock(&m_lock);
            m_empty_queue.enqueue(buffer);
            break;
        }

        // inform the application that there is something to dequeue
        {
            QMutexLocker lock(&m_lock);
            m_full_queue.enqueue(buffer);
        }
        emit bufferFull();
    }

    // do not evaluate the result of the last operation if there
    // was the external request to stop
    if (isInterruptionRequested() || (interrupted && (result > 0)))
        result = 0;

    if (result) emit stopped(result);
}

//***************************************************************************
//***************************************************************************

#include "moc_RecordThread.cpp"
