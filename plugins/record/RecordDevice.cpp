/*************************************************************************
       RecordDevice.cpp  -  device for audio recording, currently only OSS
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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/soundcard.h>
#include "RecordDevice.h"

//***************************************************************************
RecordDevice::RecordDevice()
    :m_fd(-1)
{
}

//***************************************************************************
RecordDevice::~RecordDevice()
{
    close();
}

//***************************************************************************
int RecordDevice::open(const QString &dev)
{
    // close the device if it is still open
    Q_ASSERT(m_fd < 0);
    if (m_fd >= 0) close();

    // first of all: try to open the device itself
    int fd = ::open(dev.ascii(), O_RDONLY);
    if (fd < 0) return fd;

    m_fd = fd;

    return m_fd;
}

//***************************************************************************
int RecordDevice::close()
{
    if (m_fd < 0) return 0; // already closed
    ::close(m_fd);
    m_fd = -1;

    return 0;
}

// bits per sample: 16,18,19,20,22,23,24

//***************************************************************************
QValueList<double> RecordDevice::detectSampleRates()
{
    QValueList<double> list;
    Q_ASSERT(m_fd >= 0);

    static const int known_rates[] = {
	 4000, // standard OSS
	 5125, // seen in Harmony driver (HP712, 715/new)
	 5510, // seen in AD1848 driver
	 5512, // seen in ES1370 driver
	 6215, // seen in ES188X driver
	 6615, // seen in Harmony driver (HP712, 715/new)
	 6620, // seen in AD1848 driver
	 7350, // seen in AWACS and Burgundy sound driver
	 8000, // standard OSS
	 8820, // seen in AWACS and Burgundy sound driver
	 9600, // seen in AD1848 driver
	11025, // soundblaster
	14700, // seen in AWACS and Burgundy sound driver
	16000, // standard OSS
	17640, // seen in AWACS and Burgundy sound driver
	18900, // seen in Harmony driver (HP712, 715/new)
	22050, // soundblaster
	24000, // seen in NM256 driver
	27428, // seen in Harmony driver (HP712, 715/new)
	29400, // seen in AWACS and Burgundy sound driver
	32000, // standard OSS
	32768, // seen in CS4299 driver
	33075, // seen in Harmony driver (HP712, 715/new)
	37800, // seen in Harmony driver (HP712, 715/new)
	44100, // soundblaster
	48000, // AC97
	64000, // AC97
	88200, // seen in RME96XX driver
	96000  // AC97
    };

    // try all known sample rates
    for (unsigned int i=0; i < sizeof(known_rates)/sizeof(int); i++) {
	int rate = known_rates[i];
	int err = ioctl(m_fd, SNDCTL_DSP_SPEED, &rate);
	if (err < 0) continue;
	qDebug("found rate %d Hz", rate);
	list.append(rate);
    }

    return list;
}

//***************************************************************************
//***************************************************************************
