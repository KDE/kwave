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

#include "config.h"
#include <qobject.h>
#include <qstring.h>
#include <artsc.h>

#include "mt/Mutex.h"
#include "libkwave/PlayBackDevice.h"

class PlayBackArts: public PlayBackDevice
{
    Q_OBJECT
public:

    /** Default constructor */
    PlayBackArts(Mutex &arts_lock);

    /** Destructor */
    virtual ~PlayBackArts();

    /**
     * Opens the device for playback.
     * @see PlayBackDevice::open
     */
    virtual QString open(const QString &, double rate,
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

    /**
     * Translates an aRts error code into a verbose and
     * localized error string.
     * @note we must do that on our own because aRts currently
     *       has no support for i18n()
     * @param errorcode one of the error codes defined in artsc.h
     * @return localized error message
     */
    QString artsErrorText(int errorcode);

protected:

    /** Writes the output buffer to the device */
    void flush();

    /** Playback stream for aRts */
    arts_stream_t m_stream;

    /** Playback rate [samples/second] */
    double m_rate;

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

    /** reference to the global lock for aRts, artsc is not threadsafe ! */
    Mutex &m_lock_aRts;

    /**
     * True if the playback is closed. Needed because it can happen that
     * close() is called multiple times, even if already closed.
     */
    bool m_closed;

};

#endif /* _PLAY_BACK_ARTS_H_ */
