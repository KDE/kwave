/*************************************************************************
         RecordDevice.h  -  device for audio recording, currently only OSS
                             -------------------
    begin                : Wed Sep 17 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
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

#ifndef _RECORD_DEVICE_H_
#define _RECORD_DEVICE_H_

#include "config.h"
#include <qvaluelist.h>
class QString;

class RecordDevice
{
public:

    /** Constructor */
    RecordDevice();

    /** Destructor */
    ~RecordDevice();

    /**
     * Open the record device.
     * @param dev path of the record device
     * @return file descriptor >= 0 or negative error code if failed
     */
    int open(const QString &dev);

    /** Close the device */
    int close();

    /** get a list of supported sample rates */
    QValueList<double> detectSampleRates();

private:

    /** file descriptor of the device or -1 if not open */
    int m_fd;

};

#endif /* _RECORD_DEVICE_H_ */
