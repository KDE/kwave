/***************************************************************************
  AsyncSyncGuard.h  -  guard for synchronizing X11 for multithreading under qt
			     -------------------
    begin                : Sep 2000
    copyright            : (C) 2000 by Thomas Eschenbacher
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

#ifndef _ASYNC_SYNC_GUARD_H_
#define _ASYNC_SYNC_GUARD_H_

#include "libgui/AsyncSync.h"

class AsyncSyncGuard: private AsyncSync
{
public:
   /**
    * This is the only constructor of this guard. It initializes itself
    * and synchronizes the X11 event processing by calling the AsyncSync's
    * AsyncHandler and SyncHandler.
    */
   AsyncSyncGuard()
   :AsyncSync()
   {
	AsyncHandler();
	SyncHandler();
   };
};

#endif // _ASYNC_SYNC_GUARD_H_
