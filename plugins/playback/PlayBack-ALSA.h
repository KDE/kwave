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

#ifndef PLAY_BACK_ALSA_H
#define PLAY_BACK_ALSA_H

#include "config.h"
#ifdef HAVE_ALSA_SUPPORT

/*
 * use the new ALSA HW/SW params API, needed to compile under SuSE-9.0
 * (workaround as seen in http://www.linuxjournal.com/article/6735)
 */
#define ALSA_PCM_NEW_HW_PARAMS_API
#define ALSA_PCM_NEW_SW_PARAMS_API

#include <alsa/asoundlib.h>

#include <QList>
#include <QMap>
#include <QString>

#include "libkwave/PlayBackDevice.h"
#include "libkwave/SampleArray.h"

namespace Kwave
{

    class SampleEncoder;

    class PlayBackALSA: public Kwave::PlayBackDevice
    {
    public:

        /** Default constructor */
        PlayBackALSA();

        /** Destructor */
        ~PlayBackALSA() override;

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

    protected:

        /**
         * Opens a physical device and returns its file descriptor
         * (short version, used for probing / detecting)
         *
         * @param device filename of the device
         * @return pcm stream or null pointer on errors
         */
        snd_pcm_t *openDevice(const QString &device);

        /**
         * Open the device and set all internal member variables that
         * need to be initialized for playback.
         *
         * @param device name of the ALSA device
         * @param rate sample rate, rounded as unsigned int
         * @param channels number of tracks/channels to use
         * @param bits number of bits per sample
         * @return zero or positive if succeeded, or negative error
         *         code if failed
         */
        int openDevice(const QString &device, unsigned int rate,
                       unsigned int channels, unsigned int bits);

        /**
         * Used in "openDevice()" to set the member variables m_format,
         * m_bytes_per_sample and m_bits according the given
         * resolution in bits per sample. The number of bits per sample
         * will be rounded up to the next multiple of 8. m_handle must
         * already be an opened device.
         *
         * @param hw_params valid ALSA hardware parameters
         * @param bits number of bits per sample [1...32]
         * @return zero or positive if succeeded, or negative error
         *         code if failed
         */
        int setFormat(snd_pcm_hw_params_t *hw_params, unsigned int bits);

        /** Writes the output buffer to the device */
        int flush();

        /** scan all ALSA devices, re-creates m_device_list */
        void scanDevices();

        /**
         * Translate a verbose device name into a ALSA hardware device name.
         *
         * @param name verbose name of the device
         * @return device name that can be used for snd_pcm_open()
         */
        QString alsaDeviceName(const QString &name);

    private:

        /**
         * Walk through the list of all known formats and collect the
         * ones that are supported into "m_supported_formats".
         */
        QList<int> detectSupportedFormats(const QString &device);

        /**
         * create a ALSA device format (enum) from parameters.
         * @param bits the number of bits per sample, related
         *        to the decoded stream
         * @return the index of the best matching format within the list
         *         of known formats, or -1 if no match was found
         */
        int mode2format(int bits);

    private:

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

        /**
        * Number of bytes per sample, already multiplied with
        * the number of channels (m_channels)
        */
        unsigned int m_bytes_per_sample;

        /** Exponent of the buffer size */
        unsigned int m_bufbase;

        /** buffer with raw device data */
        QByteArray m_buffer;

        /** Buffer size on bytes */
        unsigned int m_buffer_size;

        /** number of bytes in the buffer */
        unsigned int m_buffer_used;

        /** sample format, used for ALSA */
        snd_pcm_format_t m_format;

        /** number of samples per period */
        snd_pcm_uframes_t m_chunk_size;

        /**
         * dictionary for translating verbose device names
         * into ALSA hardware device names
         * (key = verbose name, data = ALSA hardware device name)
         */
        static QMap<QString, QString> m_device_list;

        /**
         * list of supported formats of the current device, indices in
         * the global list of known formats.
         * Only valid after a successful call to "open()",
         * otherwise empty
         */
        QList<int> m_supported_formats;

        /** encoder for conversion from samples to raw */
        Kwave::SampleEncoder *m_encoder;

    };
}

#endif /* HAVE_ALSA_SUPPORT */

#endif /* PLAY_BACK_ALSA_H */

//***************************************************************************
//***************************************************************************
