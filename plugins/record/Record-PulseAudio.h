/***************************************************************************
 *    Record-PulseAudio.h  -  device for audio recording via PulesAudio
 *                             -------------------
 *    begin                : Sun Okt 20 2013
 *    copyright            : (C) 2014 by Joerg-Christian Boehme
 *    email                : joerg@chaosdorf.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KWAVE_RECORD_PULSEAUDIO_H
#define KWAVE_RECORD_PULSEAUDIO_H

#include "config.h"
#ifdef HAVE_PULSEAUDIO_SUPPORT

#include <poll.h>

#include <pulse/context.h>
#include <pulse/error.h>
#include <pulse/introspect.h>
#include <pulse/proplist.h>
#include <pulse/stream.h>
#include <pulse/mainloop.h>

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QList>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>

#include "libkwave/SampleFormat.h"
#include "libkwave/Runnable.h"
#include "libkwave/WorkerThread.h"

#include "RecordDevice.h"

namespace Kwave
{

    class RecordPulseAudio : public Kwave::RecordDevice,
                             public Kwave::Runnable
    {
    public:

	/** Constructor */
	RecordPulseAudio();

	/** Destructor */
	virtual ~RecordPulseAudio();

	/**
	 * Open the record device.
	 * @param dev path of the record device
	 * @return file descriptor >= 0 or negative error code if failed
	 */
	virtual int open(const QString& dev);

	/** Returns the current endianness (big/little) */
	virtual Kwave::byte_order_t endianness();

	/** Returns the current sample format (signed/unsigned) */
	virtual Kwave::SampleFormat sampleFormat();

	/**
	 * Try to set a new sample format (signed/unsigned)
	 * @param new_format the identifier for the new format
	 * @return zero on success, negative error code if failed
	 * @see class SampleFormat
	 */
	virtual int setSampleFormat(Kwave::SampleFormat new_format);

	/**
	 * Gets a list of supported sample formats.
	 * @note this depends on the current setting of the compression!
	 */
	virtual QList< Kwave::SampleFormat > detectSampleFormats();

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
	virtual QString fileFilter();

	/**
	 * our own poll function, for timeout support
	 * @internal
	 */
	int mainloopPoll(struct pollfd *ufds, unsigned long int nfds,
                         int timeout);

    protected:

	/** re-implementation of the threaded mainloop of PulseAudio */
	virtual void run_wrapper(const QVariant &params);

    private:

	/**
	 * called from pulse audio to inform about state changes of the
	 * server context.
	 *
	 * @param c pulse server context
	 * @param userdata user data, pointer to a RecordPulseAudio object
	 */
	static void pa_context_notify_cb(pa_context *c, void *userdata);

	/**
	 * called from pulse audio to inform about state changes of the
	 * server context.
	 *
	 * @param c pulse server context
	 * @param info pointer to a source info object
	 * @param eol if negative: error occurred, zero: more data follows,
	 *            positive: end of info, done.
	 * @param userdata pointer to a RecordPulseAudio object
	 */
	static void pa_source_info_cb(pa_context *c, const pa_source_info *info,
	                              int eol, void *userdata);

	/**
	 * called from pulse audio to inform about state changes of a
	 * stream.
	 *
	 * @param p pulse audio stream
	 * @param userdata user data, pointer to a RecordPulseAudio object
	 */
	static void pa_stream_state_cb(pa_stream *p, void *userdata);

	/**
	 * called from pulse audio after data has been read
	 *
	 * @param p pulse audio stream
	 * @param nbytes number of read bytes, maybe (unused)
	 * @param userdata user data, pointer to a RecordPulseAudio object
	 */
	static void pa_read_cb(pa_stream *p, size_t nbytes, void *userdata);

	/**
	 * Callback for pulse volume operation.
	 *
	 * @param c pulse server context
	 * @param success success to set volume
	 * @param userdata user data, pointer to a RecordPulseAudio object
	 */
	static void inputVolumeCallback(pa_context *c, int success, void *userdata);

	/**
	 * Callback for pulse sink info.
	 *
	 * @param c pulse server context
	 * @param info pointer to a source info object
	 * @param eol if negative: error occurred, zero: more data follows,
	 *            positive: end of info, done.
	 */
	void notifySourceInfo(pa_context *c, const pa_source_info *info, int eol);

	/**
	 * Callback for pulse audio context state changes
	 *
	 * @param c pulse server context
	 */
	void notifyContext(pa_context *c);

	/**
	 * Callback for pulse stream state changes
	 *
	 * @param stream pulse audio stream
	 */
	void notifyStreamState(pa_stream *stream);

	/**
	 * Callback after reading data.
	 *
	 * @param stream pulse audio stream
	 * @param nbytes number of read bytes, maybe (unused)
	 */
	void notifyRead(pa_stream *stream, size_t nbytes);

	/**
	 * Try to connect to the PulseAudio server and create a valid context
	 */
	bool connectToServer();

	/**
	 * Disconnect from the PulseAudio server and clean up
	 */
	void disconnectFromServer();

	/** scan all PulseAudio source, re-creates m_device_list */
	void scanDevices();

	/**
	 * create a PulseAudio device format (enum) from parameters.
	 * @param compression the compression type
	 * @see Compression
	 * @param bits the number of bits per sample, related
	 *        to the decoded stream
	 * @param sample_format the sample format
	 *        (signed or unsigned)
	 * @return the index of the best matching format within the list
	 *         of known formats, or -1 if no match was found
	 */
	int mode2format(int compression, int bits,
			Kwave::SampleFormat sample_format);

	/**
	 * Initialize the PulseAudio device with current parameters and
	 * prepare it for recording.
	 * @param buffer_size buffer size
	 * @return zero on success or negative error code
	 *         -EINVAL or -EIO
	 */
	int initialize(uint32_t buffer_size);

    private:

	/** relevant information about a PulseAudio sink */
	typedef struct
	{
	    QString m_name;               /**< internal name of the source */
	    QString m_description;        /**< verbose name of the source  */
	    QString m_driver;             /**< internal driver name        */
	    quint32  m_card;              /**< index of the card or -1     */
	    pa_sample_spec m_sample_spec; /**< accepted sample format      */
	} source_info_t;

    private:

	/** worker thread, running the event loop */
	Kwave::WorkerThread m_mainloop_thread;

	/** lock for the main loop */
	QMutex m_mainloop_lock;

	/** wait condition for mainloopWait/mainloopSignal */
	QWaitCondition m_mainloop_signal;

	/** sample format (signed int, unsigned int, float, ... */
	Kwave::SampleFormat m_sample_format;

	/** number of tracks [0...N-1] */
	uint8_t m_tracks;

	/** sample rate  */
	double m_rate;

	/** compression mode */
	int m_compression;

	/** resolution [bits per sample] */
	unsigned int m_bits_per_sample;

	/**
	 * list of supported formats of the current device, indices in
	 * the global list of known formats.
	 * Only valid after a successful call to "open()",
	 * otherwise empty
	 */
	QList<int> m_supported_formats;

	/** true if initialize() has been successfully been run */
	bool m_initialized;

	/** pulse: property list of the context */
	pa_proplist *m_pa_proplist;

	/** pulse: main loop */
	pa_mainloop *m_pa_mainloop;

	/** pulse: context of the connection to the server */
	pa_context *m_pa_context;

	/** pulse: playback stream */
	pa_stream *m_pa_stream;

	/** pulse: device */
	QByteArray m_pa_device;

	/** record plugin name */
	QByteArray m_name;

	/**
	 * list of available devices
	 * key=full encoded name of the sink, data=info about the sink
	 */
	QMap<QString, source_info_t> m_device_list;
    };

}

#endif /* HAVE_PULSEAUDIO_SUPPORT */

#endif // KWAVE_RECORD_PULSEAUDIO_H




