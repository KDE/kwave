/***************************************************************************
			  AsyncSync.cpp  -  multithreading support for qt
			     -------------------
    begin                : Jun 1997
    copyright            : (C) 2000 by Brian Theodore,
                           Science Applications International Corp.
                           Simulation Technology Division
    email                : theodore@std.saic.com

  (copied to the kwave project by Thomas.Eschenbacher@gmx.de at Jun 2000)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <unistd.h> // for pipe
#include <stdio.h>

#include <qobject.h>
#include <qsocketnotifier.h>

#include "mt/Mutex.h"
#include "mt/MutexGuard.h"
#include "mt/AsyncSync.h"

//*****************************************************************************
AsyncSync::AsyncSync()
    :QObject()
{
    MutexGuard lock(m_lock);

    // Create IPC pipe for async/sync communication with X server
    if ( ::pipe(m_fds) == -1 ) {
	perror( "Creating pipe" );
	m_fds[0] = m_fds[1] = -1;
    }

    // Create new socket notifier to monitor when data is ready to be
    // read from pipe
    m_sn = new QSocketNotifier(m_fds[0], QSocketNotifier::Read);
    Q_ASSERT(m_sn);

    // Connect up the socket notifier's activated routine to dequeue
    // any new clients added to Database
    if (m_sn) connect(m_sn, SIGNAL(activated(int)),
                      this, SLOT(SyncHandler()) );
}

//*****************************************************************************
AsyncSync::~AsyncSync()
{
    MutexGuard lock(m_lock);

    // Delete socket notifier
    if (m_sn) delete m_sn;

    // Close pipe file descriptors
    if ( ::close(m_fds[0] ) == -1 ) {
	perror( "Closing read file descriptor" );
    }

    if ( ::close(m_fds[1] ) == -1 ) {
	perror( "Closing writing file descriptor" );
    }
}

//*****************************************************************************
void AsyncSync::SyncHandler()
{
    // First remove message from pipe ( the writer only wrote 1 byte )
    static char dummy;
    if ( ::read( m_fds[0], &dummy, 1 ) == -1 ) {
	::perror( "Reading from pipe" );
    }

    // Now emit activated signal, and let user decide what to do
    emit Activated();
}

//*****************************************************************************
void AsyncSync::AsyncHandler()
{
    // Just send a single byte of data;
    static const char dummy = 0x00;
    if ( ::write( m_fds[1], &dummy, 1 ) == -1 ) {
	::perror( "Writing to pipe" );
    }
}

//*****************************************************************************
//*****************************************************************************
