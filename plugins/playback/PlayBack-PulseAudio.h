/***************************************************************************
  PlayBack-PulseAudio.h  -  playback device for PulseAudio
			     -------------------
    begin                : Tue Sep 29 2009
    copyright            : (C) 2009 by Thomas Eschenbacher
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

#ifndef _PLAY_BACK_PULSE_AUDIO_H_
#define _PLAY_BACK_PULSE_AUDIO_H_

#include "config.h"
#ifdef HAVE_PULSEAUDIO_SUPPORT

#include <pulse/context.h>
#include <pulse/error.h>
#include <pulse/gccmacro.h>
#include <pulse/introspect.h>
#include <pulse/proplist.h>
#include <pulse/stream.h>
#include <pulse/thread-mainloop.h>

#include <QList>
#include <QMap>
#include <QSemaphore>
#include <QString>

#include "libkwave/FileInfo.h"
#include "libkwave/KwaveSampleArray.h"
#include "libkwave/PlayBackDevice.h"

class PlayBackPulseAudio: public PlayBackDevice
{
public:

    /**
     * Constructor
     * @param info the current FileInfo with metadata
     */
    PlayBackPulseAudio(const FileInfo &info);

    /** Destructor */
    virtual ~PlayBackPulseAudio();

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
    virtual int write(const Kwave::SampleArray &samples);

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
    virtual QList<unsigned int> supportedBits(const QString &device);

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

    /** Writes the output buffer to the device */
    int flush();

private:

    /**
     * called from pulse audio to inform about state changes of the
     * server context.
     *
     * @param c pulse server context
     * @param data user data, pointer to a PlayBackPulseAudio object
     */
    static void pa_context_notify_cb(pa_context *c, void *data);

    /**
     * called from pulse audio to inform about state changes of the
     * server context.
     *
     * @param c pulse server context
     * @param info pointer to a sink info object
     * @param eol if negative: error occurred, zero: more data follows,
     *            positive: end of info, done.
     * @param userdata pointer to a PlayBackPulseAudio object
     */
    static void pa_sink_info_cb(pa_context *c, const pa_sink_info *info,
                                int eol, void *userdata);

    /**
     * called from pulse audio to inform about state changes of a
     * stream.
     *
     * @param p pulse audio stream
     * @param data user data, pointer to a PlayBackPulseAudio object
     */
    static void pa_stream_state_cb(pa_stream *p, void *userdata);

    /**
     * Callback for pulse audio context state changes
     *
     * @param c pulse server context
     */
    void notifyContext(pa_context *c);

    /**
     * Callback for pulse sink info
     *
     * @param c pulse server context
     * @param info pointer to a sink info object
     * @param eol if negative: error occurred, zero: more data follows,
     *            positive: end of info, done.
     */
    void notifySinkInfo(pa_context *c, const pa_sink_info *info, int eol);

    /**
     * Callback for pulse stream state changes
     *
     * @param stream pulse audio stream
     */
    void notifyStreamState(pa_stream *stream);

    /**
     * Try to connect to the PulseAudio server and create a valid context
     */
    bool connectToServer();

    /**
     * Disconnect from the PulseAudio server and clean up
     */
    void disconnectFromServer();

    /** scan all PulseAudio sinks, re-creates m_device_list */
    void scanDevices();

private:

    /** relevant information about a PulseAudio sink */
    typedef struct
    {
	QString m_name;               /**< internal name of the sink  */
	QString m_description;        /**< verbose name of the sink   */
	QString m_driver;             /**< internal driver name       */
	uint32_t m_card;              /**< index of the card or -1    */
	pa_sample_spec m_sample_spec; /**< accepted sample format     */
    } sink_info_t;

private:

    /** file info, for meta info like title, author, name etc. */
    FileInfo m_info;

    /** Playback rate [samples/second] */
    double m_rate;

    /** Number of channels */
    unsigned int m_channels;

    /** Exponent of the buffer size */
    unsigned int m_bufbase;

    /** buffer with raw device data */
    QByteArray m_buffer;

    /** Buffer size on bytes */
    unsigned int m_buffer_size;

    /** number of bytes in the buffer */
    unsigned int m_buffer_used;

    /** pulse: property list of the context */
    pa_proplist *m_pa_proplist;

    /** pulse: main loop */
    pa_threaded_mainloop *m_pa_mainloop;

    /** pulse: context of the connection to the server */
    pa_context *m_pa_context;

    /** pulse: playback stream */
    pa_stream *m_pa_stream;

    /**
     * list of available devices
     * key=full encoded name of the sink, data=info about the sink
     */
    QMap<QString, sink_info_t> m_device_list;

};

#endif /* HAVE_PULSEAUDIO_SUPPORT */

#endif /* _PLAY_BACK_PULSE_AUDIO_H_ */
