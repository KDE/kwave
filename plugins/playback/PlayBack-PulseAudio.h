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

#ifndef PLAY_BACK_PULSE_AUDIO_H
#define PLAY_BACK_PULSE_AUDIO_H

#include "config.h"
#ifdef HAVE_PULSEAUDIO_SUPPORT

#include <poll.h>

#include <pulse/context.h>
#include <pulse/error.h>
#include <pulse/gccmacro.h>
#include <pulse/introspect.h>
#include <pulse/mainloop.h>
#include <pulse/proplist.h>
#include <pulse/stream.h>

#include <QList>
#include <QMap>
#include <QMutex>
#include <QString>
#include <QWaitCondition>

#include "libkwave/FileInfo.h"
#include "libkwave/PlayBackDevice.h"
#include "libkwave/Runnable.h"
#include "libkwave/SampleArray.h"
#include "libkwave/WorkerThread.h"

namespace Kwave
{
    class PlayBackPulseAudio: public Kwave::PlayBackDevice,
                              public Kwave::Runnable
    {
    public:

        /**
         * Constructor
         * @param info the current FileInfo with metadata
         */
        explicit PlayBackPulseAudio(const Kwave::FileInfo &info);

        /** Destructor */
        ~PlayBackPulseAudio() override;

        /**
         * Opens the device for playback.
         * @see PlayBackDevice::open
         */
        virtual QString open(const QString &device, double rate,
                             unsigned int channels, unsigned int bits,
                             unsigned int bufbase) override;

        /**
         * Writes an array of samples to the output device.
         * @see PlayBackDevice::write
         */
        int write(const Kwave::SampleArray &samples) override;

        /**
         * Closes the output device.
         * @see PlayBackDevice::close
         */
        int close() override;

        /** return a string list with supported device names */
        QStringList supportedDevices() override;

        /** return a string suitable for a "File Open..." dialog */
        QString fileFilter() override;

        /**
         * returns a list of supported bits per sample resolutions
         * of a given device.
         *
         * @param device filename of the device
         * @return list of supported bits per sample, or empty on errors
         */
        virtual QList<unsigned int> supportedBits(const QString &device)
            override;

        /**
         * Detect the minimum and maximum number of channels.
         * If the detection fails, minimum and maximum are set to zero.
         *
         * @param device filename of the device
         * @param min receives the lowest supported number of channels
         * @param max receives the highest supported number of channels
         * @return zero or positive number if ok,
         *         negative error number if failed
         */
        virtual int detectChannels(const QString &device,
                                   unsigned int &min, unsigned int &max)
            override;

        /**
         * our own poll function, for timeout support
         * @internal
         */
        int mainloopPoll(struct pollfd *ufds, unsigned long int nfds,
                         int timeout);

    protected:

        /** Writes the output buffer to the device */
        int flush();

        /** re-implementation of the threaded mainloop of PulseAudio */
        void run_wrapper(const QVariant &params) override;

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
         * @param userdata user data, pointer to a PlayBackPulseAudio object
         */
        static void pa_stream_state_cb(pa_stream *p, void *userdata);

        /**
         * called from pulse audio after data has been written
         *
         * @param p pulse audio stream
         * @param nbytes number of written bytes, maybe (unused)
         * @param userdata user data, pointer to a PlayBackPulseAudio object
         */
        static void pa_write_cb(pa_stream *p, size_t nbytes, void *userdata);

        /**
         * called from pulse audio after data has been written
         *
         * @param s pulse audio stream
         * @param success indicates success (unused)
         * @param userdata user data, pointer to a PlayBackPulseAudio object
         */
        static void pa_stream_success_cb(pa_stream *s, int success,
                                         void *userdata);

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
         * Callback after writing data.
         *
         * @param stream pulse audio stream
         * @param nbytes number of written bytes, maybe (unused)
         */
        void notifyWrite(pa_stream *stream, size_t nbytes);

        /**
         * Callback after successful stream operations.
         *
         * @param stream pulse audio stream
         * @param success (unused)
         */
        void notifySuccess(pa_stream *stream, int success);

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
            quint32  m_card;              /**< index of the card or -1    */
            pa_sample_spec m_sample_spec; /**< accepted sample format     */
        } sink_info_t;

    private:
        /** worker thread, running the event loop */
        Kwave::WorkerThread m_mainloop_thread;

        /** lock for the main loop */
        QMutex m_mainloop_lock;

        /** wait condition for mainloopWait/mainloopSignal */
        QWaitCondition m_mainloop_signal;

        /** file info, for meta info like title, author, name etc. */
        Kwave::FileInfo m_info;

        /** sample rate used when opening the device */
        double m_rate;

        /** number of bytes per sample x nr of channels */
        unsigned int m_bytes_per_sample;

        /** buffer with raw device data */
        void *m_buffer;

        /** buffer size in bytes */
        size_t m_buffer_size;

        /** number of bytes in the buffer */
        size_t m_buffer_used;

        /**
         * exponent of the buffer size,
         * buffer size should be (1 << m_bufbase)
         */
        unsigned int m_bufbase;

        /** pulse: property list of the context */
        pa_proplist *m_pa_proplist;

        /** pulse: main loop */
        pa_mainloop *m_pa_mainloop;

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
}

#endif /* HAVE_PULSEAUDIO_SUPPORT */

#endif /* PLAY_BACK_PULSE_AUDIO_H */

//***************************************************************************
//***************************************************************************
