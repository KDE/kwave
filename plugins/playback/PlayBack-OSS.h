/***************************************************************************
         PlayBack-OSS.h  -  playback device for standard linux OSS
			     -------------------
    begin                : Sat May 19 2001
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

#ifndef _PLAY_BACK_OSS_H_
#define _PLAY_BACK_OSS_H_

#include "config.h"
#include <qobject.h>
#include <qstring.h>

#include "libkwave/PlayBackDevice.h"

class PlayBackOSS: public PlayBackDevice
{
    Q_OBJECT
public:

    /** Default constructor */
    PlayBackOSS();

    /** Destructor */
    virtual ~PlayBackOSS();

    /**
     * Opens the device for playback.
     * @see PlayBackDevice::open
     */
    virtual QString open(const QString &device, double rate,
                         unsigned int channels, unsigned int bits,
                         unsigned int bufbase);

    /**
     * Writes an array of samples to the output device.
     * @see PlayBackDevice::write
     */
    virtual int write(QMemArray<sample_t> &samples);

    /**
     * Closes the output device.
     * @see PlayBackDevice::close
     */
    virtual int close();

protected:

    /** Writes the output buffer to the device */
    void flush();

    /** Name of the output device */
    QString m_device_name;

    /** Handle of the output device */
    int m_handle;

    /** Playback rate [samples/second] */
    double m_rate;

    /** Number of channels */
    unsigned int m_channels;

    /** Resolution in bits per sample */
    unsigned int m_bits;

    /** Exponent of the buffer size */
    unsigned int m_bufbase;

    /** buffer with raw device data */
    QByteArray m_buffer;

    /** Buffer size on bytes */
    unsigned int m_buffer_size;

    /** number of bytes in the buffer */
    unsigned int m_buffer_used;

};

#endif /* _PLAY_BACK_OSS_H_ */
