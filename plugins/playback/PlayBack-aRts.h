/***************************************************************************
        PlayBack-aRts.h  -  playback device for aRts sound daemon
			     -------------------
    begin                : Wed Jul 04 2001
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

#ifndef _PLAY_BACK_ARTS_H_
#define _PLAY_BACK_ARTS_H_

#include <qobject.h>
#include <qstring.h>

#include <artsc.h>

#include "mt/Mutex.h"
#include "PlayBackDevice.h"

class PlayBackArts: public PlayBackDevice
{
    Q_OBJECT
public:

    /** Default constructor */
    PlayBackArts();

    /** Destructor */
    virtual ~PlayBackArts();

    /**
     * Opens the device for playback.
     * @see PlayBackDevice::open
     */
    virtual QString open(const QString &device, unsigned int rate,
                         unsigned int channels, unsigned int bits,
                         unsigned int bufbase);

    /**
     * Writes an array of samples to the output device.
     * @see PlayBackDevice::write
     */
    virtual int write(QArray<sample_t> &samples);

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

    /** Playback stream for aRts */
    arts_stream_t m_stream;

    /** Playback rate [samples/second] */
    unsigned int m_rate;

    /** Number of channels */
    unsigned int m_channels;

    /** Resolution in bits per sample */
    unsigned int m_bits;

    /** buffer with raw device data */
    QByteArray m_buffer;

    /** Buffer size on bytes */
    unsigned int m_buffer_size;

    /** number of bytes in the buffer */
    unsigned int m_buffer_used;

//    /** the static global lock for aRts, artsc is not threadsafe ! */
//    static Mutex *m_lock_aRts;

};

#endif /* _PLAY_BACK_ARTS_H_ */
