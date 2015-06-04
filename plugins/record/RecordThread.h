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

#ifndef RECORD_THREAD_H
#define RECORD_THREAD_H

#include "config.h"

#include <QByteArray>
#include <QQueue>

#include "libkwave/WorkerThread.h"

namespace Kwave
{

    class RecordDevice;

    class RecordThread: public Kwave::WorkerThread
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
	void setRecordDevice(Kwave::RecordDevice *device);

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

	/** De-queues a buffer from the m_full_queue. */
	QByteArray dequeue();

    signals:

	/**
	 * emitted when a buffer was full and has been de-queued
	 * with dequeue()
	 */
	void bufferFull();

	/**
	 * emitted when the recording stops or aborts
	 * @param errorcode zero if stopped normally or a negative
	 *        error code if aborted
	 */
	void stopped(int errorcode);

    private:

	/** the device used as source */
	Kwave::RecordDevice *m_device;

	/** queue with empty buffers for raw input data */
	QQueue<QByteArray>m_empty_queue;

	/** queue with filled buffers with raw input data */
	QQueue<QByteArray>m_full_queue;

	/** number of buffers to allocate */
	unsigned int m_buffer_count;

	/** size of m_buffer in bytes */
	unsigned int m_buffer_size;

    };
}

#endif /* RECORD_THREAD_H */

//***************************************************************************
//***************************************************************************
