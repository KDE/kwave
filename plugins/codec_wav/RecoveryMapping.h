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

#ifndef _RECOVERY_MAPPING_H_
#define _RECOVERY_MAPPING_H_

#include "config.h"
#include "RecoverySource.h"

class RecoveryMapping: public RecoverySource
{
public:
    /**
     * Constructor
     * @param offset position in the repaired file
     * @param length number of recovered bytes
     * @param dev damaged source/file
     * @param dev_offset offset withing dev
     */
    RecoveryMapping(unsigned int offset, unsigned int length,
                    QIODevice &dev, unsigned int dev_offset);

    /** Destructor */
    virtual ~RecoveryMapping() {};

    /** implementation of Recoverysource::read */
    virtual unsigned int read(unsigned int offset, char *data,
                              unsigned int bytes);

private:

    /** source with the damaged file */
    QIODevice &m_dev;

    /** start offset in the damaged file */
    unsigned int m_dev_offset;
};

#endif /* _RECOVERY_MAPPING_H_ */
