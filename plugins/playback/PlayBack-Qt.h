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

#include <QAudio>
#include <QAudioDeviceInfo>
#include <QByteArray>
#include <QIODevice>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QQueue>
#include <QSemaphore>
#include <QString>

#include "libkwave/PlayBackDevice.h"
#include "libkwave/SampleArray.h"

class QAudioOutput;
class QIODevice;

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
	virtual ~PlayBackQt();

	/**
	 * Opens the device for playback.
	 * @see PlayBackDevice::open
	 */
        virtual QString open(const QString &device, double rate,
	                     unsigned int channels, unsigned int bits,
	                     unsigned int bufbase) Q_DECL_OVERRIDE;

	/**
	 * Writes an array of samples to the output device.
	 * @see PlayBackDevice::write
	 */
        virtual int write(const Kwave::SampleArray &samples) Q_DECL_OVERRIDE;

	/**
	 * Closes the output device.
	 * @see PlayBackDevice::close
	 */
        virtual int close() Q_DECL_OVERRIDE;

	/** return a string list with supported device names */
        virtual QStringList supportedDevices() Q_DECL_OVERRIDE;

	/** return a string suitable for a "File Open..." dialog */
        virtual QString fileFilter() Q_DECL_OVERRIDE;

	/**
	 * returns a list of supported bits per sample resolutions
	 * of a given device.
	 *
	 * @param device filename of the device
	 * @return list of supported bits per sample, or empty on errors
	 */
        virtual QList<unsigned int> supportedBits(const QString &device)
            Q_DECL_OVERRIDE;

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
	                           unsigned int &min, unsigned int &max)
            Q_DECL_OVERRIDE;

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
	 * Gets the full device info of a playback device, identified by
	 * the device name.
	 *
	 * @param device name of the device or empty string for default
	 * @return a QAudioDeviceInfo
	 */
	QAudioDeviceInfo deviceInfo(const QString &device) const;

    private:

	class Buffer : public QIODevice
	{
	public:
	    /** constructor */
	    Buffer();

	    /** destructor */
	    virtual ~Buffer();

	    /**
	     * start filling the buffer
	     * @param buf_size size of the buffer in bytes
	     * @param timeout read/write timeout [ms]
	     */
	    void start(unsigned int buf_size, int timeout);

	    /**
	     * set a new read/write timeout
	     * @note does not influence currently waiting reads/writes
	     * @param timeout a new read/write timeout [ms]
	     */
	    void setTimeout(int timeout);

	    /**
	     * drain the sink, at the end of playback:
	     * provide padding to provide data for a full period
	     * @param padding array of bytes used for padding
	     */
	    void drain(QByteArray &padding);

	    /** stop filling the buffer */
	    void stop();

	    /**
	     * read data out from the buffer, called from the Qt audio device
	     * side
	     * @param data pointer to a buffer (of bytes) to receive the data
	     * @param len number of bytes to read
	     * @return number of bytes that have been read
	     */
            virtual qint64 readData(char *data, qint64 len) Q_DECL_OVERRIDE;

	    /**
	     * write data into the buffer, called from our own worker thread
	     * @param data pointer to a buffer (of bytes) to write
	     * @param len number of bytes to write
	     * @return number of bytes written
	     */
            virtual qint64 writeData(const char *data, qint64 len)
                Q_DECL_OVERRIDE;

	    /** returns the number of bytes available for reading */
            virtual qint64 bytesAvailable() const Q_DECL_OVERRIDE;

	private:

	    /** mutex for locking the queue */
	    QMutex m_lock;

	    /** semaphore with free buffer space */
	    QSemaphore m_sem_free;

	    /** semaphore with filled buffer space */
	    QSemaphore m_sem_filled;

	    /** raw buffer with audio data */
	    QQueue<char> m_raw_buffer;

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
	QMap<QString, QString> m_device_name_map;

	/** list of available Qt output devices */
	QList<QAudioDeviceInfo> m_available_devices;

	/** Qt audio output instance */
	QAudioOutput *m_output;

	/** buffer size in bytes */
	unsigned int m_buffer_size;

	/** encoder for converting from samples to raw format */
	Kwave::SampleEncoder *m_encoder;

	Kwave::PlayBackQt::Buffer m_buffer;
    };
}

#endif /* HAVE_QT_AUDIO_SUPPORT */

#endif /* PLAY_BACK_QT_H */

//***************************************************************************
//***************************************************************************
