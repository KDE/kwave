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

#ifndef RECOVERY_BUFFER_H
#define RECOVERY_BUFFER_H

#include "config.h"

#include <QByteArray>

#include "RecoverySource.h"

namespace Kwave
{
    class RecoveryBuffer: public Kwave::RecoverySource
    {
    public:
        /**
         * Constructor
         * @param offset position in the repaired file
         * @param length number of recovered bytes
         * @param buffer a buffer with 'length' bytes of data
         */
        RecoveryBuffer(quint64 offset, quint64 length, char *buffer);

        /** Destructor */
        virtual ~RecoveryBuffer() Q_DECL_OVERRIDE {}

        /** implementation of RecoverySource::read */
        virtual qint64 read(quint64 offset, char *data, unsigned int bytes)
            Q_DECL_OVERRIDE;

    private:

        /** buffer with recovered data */
        QByteArray m_buffer;
    };
}

#endif /* RECOVERY_BUFFER_H */

//***************************************************************************
//***************************************************************************
