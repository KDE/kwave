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

// System includes
#include <unistd.h> // for pipe
#include <stdio.h>

// QT includes
#include <qsocknot.h>

// JBIIS includes
#include "AsyncSync.h"

//*****************************************************************************
AsyncSync::AsyncSync()
{
    // Create IPC pipe for async/sync communication with X server
    if ( ::pipe( fds ) == -1 ) {
	perror( "Creating pipe" );
	fds[0] = fds[1] = -1;
    }

    // Create new socket notifier to monitory when data is ready to be
    // read from pipe
    sn = new QSocketNotifier( fds[0], QSocketNotifier::Read );
    ASSERT(sn);

    // Connect up the socket notifier's activated routine to the dequeue
    // any new clients added to Database
    if (sn) connect( sn, SIGNAL(activated(int)), SLOT(SyncHandler()) );
}

//*****************************************************************************
AsyncSync::~AsyncSync()
{
    // Delete socket notifier
    if (sn) delete sn;

    // Close pipe file descriptors
    if ( ::close( fds[0] ) == -1 ) {
	perror( "Closing read file descriptor" );
    }

    if ( ::close( fds[1] ) == -1 ) {
	perror( "Closing writing file descriptor" );
    }
}

//*****************************************************************************
void AsyncSync::SyncHandler(QGList &params)
{
    // First remove message from pipe ( the writer only wrote 1 byte )
    static char buf;
    if ( ::read( fds[0], &buf, 1 ) == -1 ) {
	::perror( "Reading from pipe" );
    }

    // Now emit activated signal, and let user decide what to do
    emit Activated(params);
}

//*****************************************************************************
void AsyncSync::AsyncHandler(QGList &params)
{
    // Just send a single byte of data;
    static const char *buf = "";
    if ( ::write( fds[1], buf, 1 ) == -1 ) {
	::perror( "Writing to pipe" );
    }
}

//*****************************************************************************
//*****************************************************************************
