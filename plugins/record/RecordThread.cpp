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
#include <stdlib.h>
#include "RecordDevice.h"
#include "RecordThread.h"

//***************************************************************************
RecordThread::RecordThread()
    :Thread(), m_device(0), m_empty_queue(), m_buffer_count(0),
    m_buffer_size(0), m_should_close(false),
    m_spx_started(this, SLOT(forwardStarted())),
    m_spx_stopped(this, SLOT(forwardStopped()), 1)
{
    qDebug("RecordThread::RecordThread()");
}

//***************************************************************************
RecordThread::~RecordThread()
{
    qDebug("RecordThread::~RecordThread()");

    m_should_close = true;
    stop();

    setBuffers(0, 0);
}

//***************************************************************************
void RecordThread::setRecordDevice(RecordDevice *device)
{
    Q_ASSERT(!running());
    if (running()) return;

    m_device = device;
}

//***************************************************************************
int RecordThread::setBuffers(unsigned int count, unsigned int size)
{
    Q_ASSERT(!running());
    if (running()) return -EBUSY;

    // de-queue all full buffers if any (which normally should not happen)
    while (m_full_queue.count()) {
	unsigned char *buffer = m_full_queue.dequeue();
	free(buffer);
    }

    // de-queue and delete all empty buffers if the buffer size has changed
    if (m_buffer_size != size) {
	while (!m_empty_queue.isEmpty()) {
	    unsigned char *buffer = m_empty_queue.dequeue();
	    free(buffer);
	}
    }

    // take the new settings
    m_buffer_size = size;
    m_buffer_count = count;

    // remove superflous buffers
    while (m_empty_queue.count() > m_buffer_count) {
	unsigned char *buffer = m_empty_queue.dequeue();
	free(buffer);
    }

    // enqueue empty buffers until the required amount has been reached
    while (m_empty_queue.count() < m_buffer_count) {
	unsigned char *buffer = (unsigned char *)malloc(size);
	Q_ASSERT(buffer);
	if (!buffer) break;
	m_empty_queue.enqueue(buffer);
    }

    // return number of buffers or -ENOMEM if not even two allocated
    return (m_empty_queue.count() < 2) ?
           (int)m_empty_queue.count() : -ENOMEM;
}

//***************************************************************************
void RecordThread::run()
{
    qDebug("RecordThread::run() - started");
    int result = 0;

    // signal that we have started
    m_spx_started.AsyncHandler();

    // read data until we receive a close signal
    m_should_close = false;
    while (!m_should_close) {
	// dequeue a buffer from the "empty" queue
	unsigned char *buffer = 0;
	if (m_empty_queue.isEmpty() || !(buffer = m_empty_queue.dequeue())) {
	    // we had a "buffer overflow"
	    qWarning("RecordThread::run() -> NO EMPTY BUFFER FOUND !!!");
	    result = -ENOBUFS;
	    break;
	}
	qDebug("RecordThread::run(): %u empty buffers remaining",
	       m_empty_queue.count());

	// read into the current buffer
	unsigned int len = m_buffer_size;
	unsigned char *p = buffer;
	while (len) {
	    // read raw data from the record device
	    int result = m_device->read(p, len);

	    Q_ASSERT(result >= 1);
	    if (result < 1) {
		// something went wrong !?
		qWarning("RecordThread::run(): read returned %d", result);
		break;
	    } else {
		Q_ASSERT(result <= (int)len);
		len = ((int)len > result) ? (len - result) : 0;
		p += len;
	    }
	}

	// fill remaining space with zeroes
	while (len) {
	    *(p++) = 0;
	    len--;
	}

	// enqueue the buffer for the application
	m_full_queue.enqueue(buffer);
	qDebug("RecordThread::run(): %u full buffers queued",
	       m_full_queue.count());
    }

    m_spx_stopped.enqueue(result);
    qDebug("RecordThread::run() - done");
}

//***************************************************************************
void RecordThread::forwardStarted()
{
    qDebug("RecordThread::forwardStarted()");
    emit started();
}

//***************************************************************************
void RecordThread::forwardStopped()
{
    qDebug("RecordThread::forwardStopped()");

    unsigned int count = m_spx_stopped.count();
    int error = 0;

    // dequeue all pointers and keep the latest one (should only be one!)
    Q_ASSERT(count == 1);
    if (!count) return;
    while (count--) {
	int *perror = m_spx_stopped.dequeue();
	Q_ASSERT(perror);
	if (!perror) continue;
	error = *perror;
	delete perror;
    }

    emit stopped(error);
}

//***************************************************************************
//***************************************************************************
