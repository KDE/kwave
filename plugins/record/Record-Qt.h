/*************************************************************************
          Record-Qt.cpp  -  device for audio recording via Qt Multimedia
                             -------------------
    begin                : Sun Mar 20 2016
    copyright            : (C) 2016 by Thomas Eschenbacher
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

#ifndef KWAVE_RECORD_QT_H
#define KWAVE_RECORD_QT_H

#include "config.h"
#ifdef HAVE_QT_AUDIO_SUPPORT

#include <QtAudio>
#include <QAudioDevice>
#include <QList>
#include <QRecursiveMutex>
#include <QSemaphore>
#include <QString>
#include <QStringList>
#include <QWaitCondition>

#include "libkwave/SampleFormat.h"

#include "RecordDevice.h"

class QAudioSource;
class QAudioFormat;
class QIODevice;

namespace Kwave
{

    class RecordQt: public QObject, public Kwave::RecordDevice
    {
        Q_OBJECT
    public:

        /** Constructor */
        RecordQt();

        /** Destructor */
        ~RecordQt() override;

        /**
         * Open the record device.
         * @param dev path of the record device
         * @retval QString() if successful
         * @retval QString::number(ENODEV) if device not found
         * @retval QString::number(EBUSY) if device is busy
         * @retval QString(...) device specific error message
         *                      (already translated)
         */
        QString open(const QString &dev) override;

        /** Returns the current endianness (big/little) */
        Kwave::byte_order_t endianness() override;

        /** Returns the current sample format (signed/unsigned) */
        Kwave::SampleFormat::Format sampleFormat() override;

        /**
         * Try to set a new sample format (signed/unsigned)
         * @param new_format the identifier for the new format
         * @return zero on success, negative error code if failed
         * @see class SampleFormat
         */
        virtual int setSampleFormat(Kwave::SampleFormat::Format new_format)
            override;

        /**
         * Gets a list of supported sample formats.
         * @note this depends on the current setting of the compression!
         */
        virtual QList<Kwave::SampleFormat::Format> detectSampleFormats()
            override;

        /**
         * Returns the current resolution in bits per sample or a negative
         * error code if failed
         */
        int bitsPerSample() override;

        /**
         * Set the resolution in bits per sample
         * @param new_bits resolution [bits/sample]
         * @return zero on success, negative error code if failed
         */
        int setBitsPerSample(unsigned int new_bits) override;

        /**
         * Detect a list of supported bits per sample.
         * @note this depends on the compression type
         * @return a list of bits per sample, empty if failed
         */
        QList< unsigned int > supportedBits() override;

        /** Returns the current compression type (0==none) */
        Kwave::Compression::Type compression() override;

        /**
         * Try to set a new compression type.
         * @param new_compression the identifier of the new compression
         * @return zero on success, negative error code if failed
         * @see class Compression
         */
        virtual int setCompression(Kwave::Compression::Type new_compression)
            override;

        /**
         * Gets a list of supported compression types. If no compression is
         * supported, the list might be empty.
         */
        virtual QList<Kwave::Compression::Type> detectCompressions()
            override;

        /** Returns the current sample rate of the device */
        double sampleRate() override;

        /**
         * Try to set a new sample rate.
         * @param new_rate the sample rate to be set [samples/second], can
         *        be modified and rounded up/down to the nearest supported
         *        sample rate if the underlying driver supports that.
         * @return zero on success, negative error code if failed
         */
        int setSampleRate(double& new_rate) override;

        /** get a list of supported sample rates */
        QList< double > detectSampleRates() override;

        /** Returns the current number of tracks */
        int tracks() override;

        /**
         * Try to set a new number of tracks.
         * @note the device must be open
         * @param tracks the number of tracks to be set, can be modified and
         *        decreased to the next supported number of tracks if the
         *        underlying driver supports that.
         * @return zero on success, negative error code if failed
         */
        int setTracks(unsigned int& tracks) override;

        /**
         * Detect the minimum and maximum number of tracks.
         * If the detection fails, minimum and maximum are set to zero.
         * @param min receives the lowest supported number of tracks
         * @param max receives the highest supported number of tracks
         * @return zero or positive if ok, negative error number if failed
         */
        virtual int detectTracks(unsigned int& min, unsigned int& max)
            override;

        /** Close the device */
        int close() override;

        /**
         * Read the raw audio data from the record device.
         * @param buffer array of bytes to receive the audio data
         *        might be resized for alignment
         * @param offset offset in bytes within the buffer
         * @return number of bytes read, zero or negative if failed
         */
        virtual int read(QByteArray& buffer, unsigned int offset)
            override;

        /** return a string list with supported device names */
        QStringList supportedDevices() override;

    signals:

        /**
         * request createInMainThread()
         * @param format reference to the audio format specification
         * @param buffer_size size of the audio buffer in bytes
         */
        void sigCreateRequested(QAudioFormat &format, unsigned int buffer_size);

        /** request closeInMainThread() */
        void sigCloseRequested();

    private slots:

        /**
         * handles the request to create the device, running in the context
         * of the main thread
         * @param format reference to the audio format specification
         * @param buffer_size size of the audio buffer in bytes
         */
        void createInMainThread(QAudioFormat &format, unsigned int buffer_size);

        /**
         * handles the request to close the device, running in the context
         * of the main thread
         */
        void closeInMainThread();

    private:

        /**
         * Initialize the audio device with current parameters and
         * prepare it for recording.
         * @param buffer_size size of the audio buffer in bytes
         * @return zero on success or negative error code
         *         -EINVAL or -EIO
         */
        int initialize(unsigned int buffer_size);

        /** scan all PulseAudio source, re-creates m_device_list */
        void scanDevices();

        /**
         * Gets a playback device, identified by the device name.
         *
         * @param device name of the device or empty string for default
         * @return a QAudioDevice
         */
        QAudioDevice getDevice(const QString &device) const;

    private:

        /** mutex for locking the streaming thread against main thread */
        QRecursiveMutex m_lock;

        /**
         * dictionary for translating verbose device names
         * into Qt audio output device ids
         * (key = verbose name, data = Qt output device id)
         */
        QMap<QString, QByteArray> m_device_name_map;

        /** list of available Qt output devices */
        QList<QAudioDevice> m_available_devices;

        /** Qt audio input instance */
        QAudioSource *m_input;

        /** QIODevice for reading the data */
        QIODevice *m_source;

        /** sample format (signed int, unsigned int, float, ... */
        Kwave::SampleFormat::Format m_sample_format;

        /** number of tracks [0...N-1] */
        quint8 m_tracks;

        /** sample rate  */
        double m_rate;

        /** compression mode */
        Kwave::Compression::Type m_compression;

        /** resolution [bits per sample] */
        unsigned int m_bits_per_sample;

        /** encoded name of the sink */
        QString m_device;

        /** true if initialize() has been successfully been run */
        bool m_initialized;

    };

}

#endif /* HAVE_QT_AUDIO_SUPPORT */

#endif // KWAVE_RECORD_QT_H
