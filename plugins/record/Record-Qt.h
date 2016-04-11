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

#include <QAudio>
#include <QAudioDeviceInfo>
#include <QList>
#include <QMutex>
#include <QSemaphore>
#include <QString>
#include <QStringList>
#include <QWaitCondition>

#include "libkwave/Runnable.h"
#include "libkwave/SampleFormat.h"
#include "libkwave/WorkerThread.h"

#include "RecordDevice.h"

class QAudioInput;
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
	virtual ~RecordQt();

	/**
	 * Open the record device.
	 * @param dev path of the record device
	 * @retval QString::null if successful
	 * @retval QString::number(ENODEV) if device not found
	 * @retval QString::number(EBUSY) if device is busy
	 * @retval QString(...) device specific error message
	 *                      (already translated)
	 */
	virtual QString open(const QString &dev);

	/** Returns the current endianness (big/little) */
	virtual Kwave::byte_order_t endianness();

	/** Returns the current sample format (signed/unsigned) */
	virtual Kwave::SampleFormat::Format sampleFormat();

	/**
	 * Try to set a new sample format (signed/unsigned)
	 * @param new_format the identifier for the new format
	 * @return zero on success, negative error code if failed
	 * @see class SampleFormat
	 */
	virtual int setSampleFormat(Kwave::SampleFormat::Format new_format);

	/**
	 * Gets a list of supported sample formats.
	 * @note this depends on the current setting of the compression!
	 */
	virtual QList<Kwave::SampleFormat::Format> detectSampleFormats();

	/**
	 * Returns the current resolution in bits per sample or a negative
	 * error code if failed
	 */
	virtual int bitsPerSample();

	/**
	 * Set the resolution in bits per sample
	 * @param new_bits resolution [bits/sample]
	 * @return zero on success, negative error code if failed
	 */
	virtual int setBitsPerSample(unsigned int new_bits);

	/**
	 * Detect a list of supported bits per sample.
	 * @note this depends on the compression type
	 * @return a list of bits per sample, empty if failed
	 */
	virtual QList< unsigned int > supportedBits();

	/** Returns the current compression type (0==none) */
	virtual int compression();

	/**
	 * Try to set a new compression type.
	 * @param new_compression the identifier of the new compression
	 * @return zero on success, negative error code if failed
	 * @see class Compression
	 */
	virtual int setCompression(int new_compression);

	/**
	 * Gets a list of supported compression types. If no compression is
	 * supported, the list might be empty.
	 */
	virtual QList< int > detectCompressions();

	/** Returns the current sample rate of the device */
	virtual double sampleRate();

	/**
	 * Try to set a new sample rate.
	 * @param new_rate the sample rate to be set [samples/second], can
	 *        be modified and rounded up/down to the nearest supported
	 *        sample rate if the underlying driver supports that.
	 * @return zero on success, negative error code if failed
	 */
	virtual int setSampleRate(double& new_rate);

	/** get a list of supported sample rates */
	virtual QList< double > detectSampleRates();

	/** Returns the current number of tracks */
	virtual int tracks();

	/**
	 * Try to set a new number of tracks.
	 * @note the device must be open
	 * @param tracks the number of tracks to be set, can be modified and
	 *        decreased to the next supported number of tracks if the
	 *        underlying driver supports that.
	 * @return zero on success, negative error code if failed
	 */
	virtual int setTracks(unsigned int& tracks);

	/**
	 * Detect the minimum and maximum number of tracks.
	 * If the detection fails, minimum and maximum are set to zero.
	 * @param min receives the lowest supported number of tracks
	 * @param max receives the highest supported number of tracks
	 * @return zero or positive number if ok, negative error number if failed
	 */
	virtual int detectTracks(unsigned int& min, unsigned int& max);

	/** Close the device */
	virtual int close();

	/**
	 * Read the raw audio data from the record device.
	 * @param buffer array of bytes to receive the audio data
	 *        might be resized for alignment
	 * @param offset offset in bytes within the buffer
	 * @return number of bytes read, zero or negative if failed
	 */
	virtual int read(QByteArray& buffer, unsigned int offset);

	/** return a string list with supported device names */
	virtual QStringList supportedDevices();

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

	/** called when recorded data gets available */
	void notified();

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
	 * Gets the full device info of a playback device, identified by
	 * the device name.
	 *
	 * @param device name of the device or empty string for default
	 * @return a QAudioDeviceInfo
	 */
	QAudioDeviceInfo deviceInfo(const QString &device) const;

    private:

	/** mutex for locking the streaming thread against main thread */
	QMutex m_lock;

	/**
	 * dictionary for translating verbose device names
	 * into Qt audio output device names
	 * (key = verbose name, data = Qt output device name)
	 */
	QMap<QString, QString> m_device_name_map;

	/** list of available Qt output devices */
	QList<QAudioDeviceInfo> m_available_devices;

	/** Qt audio input instance */
	QAudioInput *m_input;

	/** QIODevice for reading the data */
	QIODevice *m_source;

	/** sample format (signed int, unsigned int, float, ... */
	Kwave::SampleFormat::Format m_sample_format;

	/** number of tracks [0...N-1] */
	quint8 m_tracks;

	/** sample rate  */
	double m_rate;

	/** compression mode */
	int m_compression;

	/** resolution [bits per sample] */
	unsigned int m_bits_per_sample;

	/** encoded name of the sink */
	QString m_device;

	/** true if initialize() has been successfully been run */
	bool m_initialized;

	/** semaphore for signaling "data available" */
	QSemaphore m_sem;
    };

}

#endif /* HAVE_QT_AUDIO_SUPPORT */

#endif // KWAVE_RECORD_QT_H
