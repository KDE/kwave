/***************************************************************************
    libkwave/CommandHandler.h  -  Interface for a class with executeCommand(...)
			     -------------------
    begin                : 2014-09-22
    copyright            : (C) 2014 by Thomas.Eschenbacher
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

#ifndef _COMMAND_HANDLER_H_
#define _COMMAND_HANDLER_H_

#include "config.h"

class QString;

namespace Kwave
{
    class CommandHandler
    {
    public:

	/** default constructor */
	CommandHandler() {}

	/** destructor */
	virtual ~CommandHandler() {}

	/**
	 * Execute a Kwave text command
	 * @param command a text command
	 * @return zero if succeeded or negative error code if failed
	 * @retval -ENOSYS is returned if the command is unknown in this
	 *                 component
	 */
	virtual int executeCommand(const QString &command) = 0;

    };
}

#endif /* _COMMAND_HANDLER_H_ */
//***************************************************************************
//***************************************************************************

