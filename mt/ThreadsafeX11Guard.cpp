/***************************************************************************
  ThreadsafeX11Guard.cpp -  guard for using X11 from a worker thread
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

#include "config.h"
#include <qapplication.h>
#include "mt/ThreadsafeX11Guard.h"

//***************************************************************************
ThreadsafeX11Guard::ThreadsafeX11Guard()
    :QObject()
{
    Q_ASSERT(qApp);
    if (qApp) qApp->lock();
}

//***************************************************************************
ThreadsafeX11Guard::~ThreadsafeX11Guard()
{
    Q_ASSERT(qApp);
    if (qApp) qApp->unlock();
}

//***************************************************************************
//***************************************************************************
