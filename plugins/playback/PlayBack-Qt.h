/***************************************************************************
          PlayBack-Qt.h  -  playback device for Qt Multimedia
                             -------------------
    begin                : Thu Nov 12 2015
    copyright            : (C) 2015 by Thomas Eschenbacher
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

#ifndef PLAY_BACK_QT_H
#define PLAY_BACK_QT_H

#include "config.h"
#ifdef HAVE_QT_AUDIO_SUPPORT

#include <QtAudio>
#include <QAudioDevice>
#include <QByteArray>
#include <QIODevice>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QRecursiveMutex>
#include <QSemaphore>
#include <QString>

#include "libkwave/PlayBackDevice.h"
#include "libkwave/SampleArray.h"

class QAudioSink;

namespace Kwave
{

    class SampleEncoder;

    class PlayBackQt: public QObject,
                      public Kwave::PlayBackDevice
    {
        Q_OBJECT
    public:

        /** Default constructor */
        PlayBackQt();

        /** Destructor */
        virtual ~PlayBackQt() override;

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

    private slots:

        /**
         * connected to the audio output device and gets notified in case
         * of state changes like start/stop of the stream or errors.
         * @param state the new device state, like active, stopped, idle etc.
         */
        void stateChanged(QAudio::State state);

    private:

        /**
         * creates a sample encoder for playback, for linear
         * formats
         * @param format the preferred format of the audio output device
         */
        void createEncoder(const QAudioFormat &format);

        /** scan all Qt audio output devices, re-creates m_device_list */
        void scanDevices();

        /**
         * Gets an audio device, identified by the device name.
         *
         * @param device name of the device or empty string for default
         * @return a QAudioDevice
         */
        QAudioDevice getDevice(const QString &device) const;

    private:

        /**
         * Internal buffer operating as a QIODevice for satisfying
         * the Qt playback engine. Uses a QByteArray as FIFO, with a write
         * pointer and a read pointer. No extra locking is needed for the
         * FIFO, we have two semaphores: one with the number of bytes
         * free and one with the number of bytes written.
         */
        class Buffer : public QIODevice
        {
        public:
            /** constructor */
            Buffer();

            /** destructor */
            virtual ~Buffer() override;

            /**
             * start filling the buffer
             * @param buf_size size of the buffer in bytes
             * @param timeout read/write timeout [ms]
             */
            void start(unsigned int buf_size, int timeout);

            /**
             * drain the sink, at the end of playback:
             * provide padding to provide data for a full period
             * @param padding array of bytes used for padding
             */
            void drain(const QByteArray &padding);

            /** stop filling the buffer */
            void stop();

            /**
             * read data out from the buffer, called from the Qt audio device
             * side
             * @param data pointer to a buffer (of bytes) to receive the data
             * @param len number of bytes to read
             * @return number of bytes that have been read
             */
            virtual qint64 readData(char *data, qint64 len) override;

            /**
             * write data into the buffer, called from our own worker thread
             * @param data pointer to a buffer (of bytes) to write
             * @param len number of bytes to write
             * @return number of bytes written
             */
            virtual qint64 writeData(const char *data, qint64 len)
                override;

            /** returns the number of bytes available for reading */
            virtual qint64 bytesAvailable() const override;

        private:

            /** mutex for locking the queue */
            QRecursiveMutex m_lock;

            /** semaphore with free buffer space */
            QSemaphore m_sem_free;

            /** semaphore with filled buffer space */
            QSemaphore m_sem_filled;

            /** raw buffer with audio data */
            QByteArray m_raw_buffer;

            /** read pointer within the raw buffer */
            qsizetype m_rp;

            /** write pointer within the raw buffer */
            qsizetype m_wp;

            /** read timeout [ms] */
            int m_timeout;

            /** buffer with padding data */
            QByteArray m_pad_data;

            /** read pointer within m_pad_data */
            int m_pad_ofs;
        };

    private:

        /** mutex for locking the streaming thread against main thread */
        QMutex m_lock;

        /**
         * dictionary for translating verbose device names
         * into Qt audio output device names
         * (key = verbose name, data = Qt output device name)
         */
        QMap<QString, QByteArray> m_device_name_map;

        /** list of available Qt output devices */
        QList<QAudioDevice> m_available_devices;

        /** Qt audio output instance */
        QAudioSink *m_output;

        /** buffer size in bytes */
        unsigned int m_buffer_size;

        /** encoder for converting from samples to raw format */
        Kwave::SampleEncoder *m_encoder;

        /** buffer object to use as interface to the qt playback thread */
        Kwave::PlayBackQt::Buffer m_buffer;

        /** internal buffer for encoding one frame */
        QByteArray m_one_frame;
    };
}

#endif /* HAVE_QT_AUDIO_SUPPORT */

#endif /* PLAY_BACK_QT_H */

//***************************************************************************
//***************************************************************************
