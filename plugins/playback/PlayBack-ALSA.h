/***************************************************************************
        PlayBack-ALSA.h  -  playback device for ALSA
			     -------------------
    begin                : Sat Mar 03 2005
    copyright            : (C) 2005 by Thomas Eschenbacher
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

#ifndef _PLAY_BACK_ALSA_H_
#define _PLAY_BACK_ALSA_H_

#include "config.h"
#ifdef HAVE_ALSA_SUPPORT

#include <qstring.h>
#include <alsa/asoundlib.h>

#include "libkwave/PlayBackDevice.h"

class PlayBackALSA: public PlayBackDevice
{
public:

    /** Default constructor */
    PlayBackALSA();

    /** Destructor */
    virtual ~PlayBackALSA();

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

    /** return a string list with supported device names */
    virtual QStringList supportedDevices();

    /** return a string suitable for a "File Open..." dialog */
    virtual QString fileFilter();

    /**
     * returns a list of supported bits per sample resolutions
     * of a given device.
     *
     * @param device filename of the device
     * @return list of supported bits per sample, or empty on errors
     */
    virtual QValueList<unsigned int> supportedBits(const QString &device);

    /**
     * Detect the minimum and maximum number of channels.
     * If the detection fails, minimum and maximum are set to zero.
     *
     * @param device filename of the device
     * @param min receives the lowest supported number of channels
     * @param max receives the highest supported number of channels
     * @return zero or positive number if ok, negative error number if failed
     */
    virtual int detectChannels(const QString &device,
                               unsigned int &min, unsigned int &max);

protected:

    /**
     * Opens a physical device and returns its file descriptor
     *
     * @param device filename of the device
     * @return pcm stream or null pointer on errors
     */
    snd_pcm_t *openDevice(const QString &device);

    /** Writes the output buffer to the device */
    void flush();

    /** Name of the output device */
    QString m_device_name;

    /** Handle of the output device */
    snd_pcm_t *m_handle;

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

#endif /* HAVE_ALSA_SUPPORT */

#endif /* _PLAY_BACK_ALSA_H_ */
