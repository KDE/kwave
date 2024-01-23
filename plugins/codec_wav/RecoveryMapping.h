/*************************************************************************
      RecoveryMapping.h  - mapping of a recovered range in a audio file
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

#ifndef RECOVERY_MAPPING_H
#define RECOVERY_MAPPING_H

#include "config.h"

#include "RecoverySource.h"

class QIODevice;

namespace Kwave
{
    class RecoveryMapping: public Kwave::RecoverySource
    {
    public:
        /**
         * Constructor
         * @param offset position in the repaired file
         * @param length number of recovered bytes
         * @param dev damaged source/file
         * @param dev_offset offset within dev
         */
        RecoveryMapping(quint64 offset, quint64 length,
                        QIODevice &dev, quint64 dev_offset);

        /** Destructor */
        virtual ~RecoveryMapping() Q_DECL_OVERRIDE {}

        /** implementation of Recoverysource::read */
        virtual qint64 read(quint64 offset, char *data, unsigned int bytes)
            Q_DECL_OVERRIDE;

    private:

        /** source with the damaged file */
        QIODevice &m_dev;

        /** start offset in the damaged file */
        quint64 m_dev_offset;
    };
}

#endif /* RECOVERY_MAPPING_H */

//***************************************************************************
//***************************************************************************
