/*************************************************************************
         RecordThread.h  -  thread for lowlevel audio recording
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

#ifndef _RECORD_THREAD_H_
#define _RECORD_THREAD_H_

#include "config.h"
#include <qcstring.h>
#include <qptrqueue.h>
#include "mt/SignalProxy.h"
#include "mt/Thread.h"

class RecordDevice;

class RecordThread: public Thread
{
    Q_OBJECT
public:
    /** Constructor */
    RecordThread();

    /** Destructor */
    virtual ~RecordThread();

    /** does the recording */
    virtual void run();

    /**
     * Select a new record device.
     * @param device a RecordDevice that is opened and set up for reading
     * @note this must not be called during recording
     */
    void setRecordDevice(RecordDevice *device);

    /**
     * Set the number of buffers and their size
     * @param count the number of buffer, minimum allowed is two
     * @param size the number of bytes for each buffer
     * @return number of allocated buffers or -ENOMEM if less than two
     * @note this must not be called during recording
     */
    int setBuffers(unsigned int count, unsigned int size);

    /** Returns the amount of remaining empty buffers */
    unsigned int remainingBuffers();

    /** Returns the number of queued filled buffers */
    unsigned int queuedBuffers();

signals:

    /**
     * emitted when a buffer was full and has been de-queued
     * with dequeue()
     * @param buffer the buffer with recorded raw bytes
     */
    void bufferFull(QByteArray buffer);

    /**
     * emitted when the recording stops or aborts
     * @param errorcode zero if stopped normally or a negative
     *        error code if aborted
     */
    void stopped(int errorcode);

private slots:

    /** forwards a "buffer full" signal to outside the thread */
    void forwardBufferFull();

    /** forwards the "stopped" signal to outside the thread */
    void forwardStopped();

protected:

    /** De-queues a buffer from the m_full_queue. */
    QByteArray dequeue();

private:

    /** the device used as source */
    RecordDevice *m_device;

    /** queue with empty buffers for raw input data */
    QPtrQueue<QByteArray>m_empty_queue;

    /** queue with filled buffers with raw input data */
    QPtrQueue<QByteArray>m_full_queue;

    /** number of buffers to allocate */
    unsigned int m_buffer_count;

    /** size of m_buffer in bytes */
    unsigned int m_buffer_size;

    /** threadsafe proxy for forwarding the "bufferFull" signal */
    SignalProxy<void> m_spx_buffer_full;

    /** threadsafe proxy for forwarding the "stopped" signal */
    SignalProxy1<int> m_spx_stopped;

};

#endif /* _RECORD_THREAD_H_ */
