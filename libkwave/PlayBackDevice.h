/***************************************************************************
       PlayBackDevice.h  -  abstract base class for playback devices
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

#ifndef _PLAY_BACK_DEVICE_H_
#define _PLAY_BACK_DEVICE_H_

#include "config.h"
#include <qmemarray.h>
#include <qobject.h>
#include <qstring.h>
#include "libkwave/Sample.h"

/**
 * Abstract base class for all kinds of playback devices.
 * It provides only a minimum of necessary functions, like
 * opening/closing and writing samples.
 *
 * @bug this class is not threadsafe on its own, it relies on the
 *      threadsafe implementation of the PlayBack plugin.
 * @bug there aren no checks for avoiding close without open,
 *      opening twice or similar
 * @bug there are no precautions to prevent duplicate instances
 *
 */
class PlayBackDevice: public QObject
{
    Q_OBJECT
public:

    /** Destructor */
    virtual ~PlayBackDevice() {};

    /**
     * Opens the device for playback.
     * @param device name of the output device, this might be a
     *               file name of a device or any user-defined
     *               string that tells the playback device where
     *               to write data.
     * @param rate playback rate [samples/second]
     * @param channels number of playback channels [1=mono, 2=stereo,...]
     * @param bits resolution for output [bits/sample]
     * @param bufbase exponent of the buffer size. The real buffer
     *                size will be (2 ^ bufbase) bytes.
     * @return zero-length string if successful, or an error
     *         message if failed
     */
    virtual QString open(const QString &device, double rate,
                         unsigned int channels, unsigned int bits,
                         unsigned int bufbase) = 0;

    /**
     * Writes an array of samples to the output device. Each sample
     * in the array is designated to one output channel.
     * @param samples array of samples for output
     * @return 0 if successful, or an error code if failed
     */
    virtual int write(QMemArray<sample_t> &samples) = 0;

    /**
     * Closes the output device.
     */
    virtual int close() = 0;

};

#endif /* _PLAY_BACK_DEVICE_H_ */
