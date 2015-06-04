/*************************************************************************
       RecoverySource.h  - base class for recovered audio file data
                             -------------------
    begin                : Sun May 12 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
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

#ifndef RECOVERY_SOURCE_H
#define RECOVERY_SOURCE_H

#include "config.h"

#include <qglobal.h>

namespace Kwave
{
    class RecoverySource
    {
    public:
	/**
	* Constructor
	* @param offset position within the recovered file
	* @param length number of bytes of the recovered area
	*/
	RecoverySource(quint64 offset, quint64 length);

	/** Destructor */
	virtual ~RecoverySource() {}

	/** Returns the start offset in the recovered file */
	virtual quint64 offset() const;

	/** Returns the number of bytes of the recovered range */
	virtual quint64 length() const;

	/** Returns the end offset in the recovered file */
	virtual quint64 end() const;

	/**
	* Reads bytes from the recovered file into a buffer
	* @param offset position within the recovered file
	* @param data pointer to the buffer to be filled
	* @param bytes number of bytes to read
	* @return number of successfully read bytes
	*/
	virtual qint64 read(quint64 offset, char *data,
	                    unsigned int bytes) = 0;

    private:
	/** offset in the file */
	quint64 m_offset;

	/** length in bytes */
	quint64 m_length;
    };
}

#endif /* RECOVERY_SOURCE_H */

//***************************************************************************
//***************************************************************************
