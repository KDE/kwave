/*************************************************************************
     RecoveryBuffer.cpp  - buffer for reconstructed audio file data
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

#ifndef _RECOVERY_BUFFER_H_
#define _RECOVERY_BUFFER_H_

#include "config.h"
#include <qcstring.h>
#include "RecoverySource.h"

class RecoveryBuffer: public RecoverySource
{
public:
    /**
     * Constructor
     * @param offset position in the repaired file
     * @param length number of recovered bytes
     * @param buffer a buffer with 'length' bytes of data
     */
    RecoveryBuffer(unsigned int offset, unsigned int length, char *buffer);

    /** implementation of Recoverysource::read */
    virtual unsigned int read(unsigned int offset, char *data,
                              unsigned int bytes);

private:

    /** buffer with recovered data */
    QByteArray m_buffer;
};

#endif /* _RECOVERY_BUFFER_H_ */
