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
#include <strings.h> // for bzero

#include "RecordDevice.h"
#include "RecordThread.h"

//***************************************************************************
RecordThread::RecordThread()
    :Thread(), m_device(0), m_empty_queue(), m_buffer_count(0),
    m_buffer_size(0),
    m_spx_buffer_full(this, SLOT(forwardBufferFull())),
    m_spx_stopped(this,     SLOT(forwardStopped()), 1)
{
}

//***************************************************************************
RecordThread::~RecordThread()
{
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
    qDebug("RecordThread::setBuffers(%u,%u)", count, size);
    Q_ASSERT(!running());
    if (running()) return -EBUSY;

    // de-queue all full buffers if any -> flush output queue
    while (m_full_queue.count()) {
	dequeue();
    }

    // de-queue and delete all empty buffers if the buffer size has changed
    if (m_buffer_size != size) {
	while (!m_empty_queue.isEmpty()) {
	    char *buffer = m_empty_queue.dequeue();
	    free(buffer);
	}
    }

    // take the new settings
    m_buffer_size = size;
    m_buffer_count = count;

    // remove superflous buffers
    while (m_empty_queue.count() > m_buffer_count) {
	char *buffer = m_empty_queue.dequeue();
	free(buffer);
    }

    // enqueue empty buffers until the required amount has been reached
    while (m_empty_queue.count() < m_buffer_count) {
	char *buffer = (char *)malloc(size);
	Q_ASSERT(buffer);
	if (!buffer) break;

	// write to the buffer with dummy data to probably
	// make it physically mapped
	bzero(buffer, size);

	m_empty_queue.enqueue(buffer);
    }

    // return number of buffers or -ENOMEM if not even two allocated
    return (m_empty_queue.count() < 2) ?
           (int)m_empty_queue.count() : -ENOMEM;
}

//***************************************************************************
unsigned int RecordThread::remainingBuffers()
{
    return (m_empty_queue.count());
}

//***************************************************************************
unsigned int RecordThread::queuedBuffers()
{
    return (m_full_queue.count());
}

//***************************************************************************
QByteArray RecordThread::dequeue()
{
    QByteArray buffer;
    buffer.resize(0);

    if (m_full_queue.count()) {
	// de-queue the buffer from the full list
	char *buf = (char *)m_full_queue.dequeue();
	buffer.duplicate(buf, m_buffer_size);

	// put the buffer back to the empty list
	m_empty_queue.enqueue(buf);
    }

    return buffer;
}

//***************************************************************************
void RecordThread::run()
{
    qDebug("RecordThread::run() - started (buffers: %u x %u byte)",
           m_buffer_count, m_buffer_size);
    int result = 0;

    // read data until we receive a close signal
    while (!shouldStop()) {
	// dequeue a buffer from the "empty" queue
	char *buffer = 0;

//	qDebug(">>> %u <<<", m_empty_queue.count()); // ###
	Q_ASSERT(!m_empty_queue.isEmpty());
	buffer = m_empty_queue.dequeue();
	Q_ASSERT(buffer);
	if (!buffer) {
	    // we had a "buffer overflow"
	    qWarning("RecordThread::run() -> NO EMPTY BUFFER FOUND !!!");
	    result = -ENOBUFS;
	    break;
	}

	// read into the current buffer
	unsigned int len = m_buffer_size;
	char *p = buffer;
	while (len) {
	    // read raw data from the record device
	    int result = m_device->read(p, len);
	    Q_ASSERT(result == (int)len);

	    if (result == -EAGAIN) {
		// thread was interrupted, received signal?
		qWarning("RecordThread::run(): read returned -EAGAIN, INT?");
		continue;
	    } else if (result < 1) {
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
	if (len) bzero(p, len);

	// enqueue the buffer for the application
	m_full_queue.enqueue(buffer);

	// inform the application that there is something to dequeue
	m_spx_buffer_full.AsyncHandler();
    }

    if (result) m_spx_stopped.enqueue(result);
    qDebug("RecordThread::run() - done");
}

//***************************************************************************
void RecordThread::forwardBufferFull()
{
    QByteArray buffer = dequeue();
    if (!buffer.count()) return; // was removed before, maybe settings changed

    // forward the buffer to the application/consumer
    emit bufferFull(buffer);
}

//***************************************************************************
void RecordThread::forwardStopped()
{
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
