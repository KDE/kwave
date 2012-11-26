/***************************************************************************
      PlayBack-Phonon.h  -  playback device for KDE4-Phonon
			     -------------------
    begin                : Fri May 15 2009
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

#ifndef _PLAY_BACK_PHONON_H_
#define _PLAY_BACK_PHONON_H_

#include "config.h"
#ifdef HAVE_PHONON_SUPPORT

#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QSemaphore>
#include <QtCore/QString>

#include <phonon/phononnamespace.h>
#include <phonon/abstractmediastream.h>
#include <phonon/audiooutput.h>
#include <phonon/backendcapabilities.h>
#include <phonon/mediaobject.h>
#include <phonon/mediasource.h>
#include <phonon/path.h>

#include "libkwave/PlayBackDevice.h"
#include "libkwave/SampleArray.h"

namespace Kwave
{

    class SampleEncoder;

    class PlayBackPhonon: public Kwave::PlayBackDevice,
                          public Phonon::AbstractMediaStream
    {
    public:

	/** Default constructor */
	PlayBackPhonon();

	/** Destructor */
	virtual ~PlayBackPhonon();

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

	/** @see Phonon::AbstractMediaStream::reset */
	virtual void reset();

	/** @see Phonon::AbstractMediaStream::needData */
	virtual void needData();

    private:

	/**
	 * creates a sample encoder for playback, for linear
	 * formats
	 * @param bits number of bits/sample (8, 16, 24 or 32)
	 */
	void createEncoder(unsigned int bits);

	/**
	 * creates a dummy RIFF wav header for fooling Phonon
	 * @param rate sample rate [samples/second]
	 * @param channels number of channels [1...N]
	 * @param bits number of bits/sample (8, 16, 24 or 32)
	 */
	void createHeader(double rate,
	                  unsigned int channels,
	                  unsigned int bits);

	/**
	 * writes the output buffer to the Phonon layer
	 * @return 0 if succeeded, -EAGAIN on timeout
	 */
	int flush();

    private:

	/** buffer with samples data */
	Kwave::SampleArray m_buffer;

	/** buffer with raw data */
	QByteArray m_raw_buffer;

	/** Buffer size on bytes */
	unsigned int m_buffer_size;

	/** number of bytes in the buffer */
	unsigned int m_buffer_used;

	/** encoder for converting from samples to raw format */
	Kwave::SampleEncoder *m_encoder;

	/** semaphore for communication between phonon and Kwave */
	QSemaphore m_sem;

	/** audio output device for Phonon */
	Phonon::AudioOutput *m_output;

	/** path from m_media_object to m_output */
	Phonon::Path m_path;

	/** media object that serves as container for the media source */
	Phonon::MediaObject m_media_object;

	/** media source, adapter for the playback stream */
	Phonon::MediaSource m_media_source;

	/** if true, send a header in the first write */
	bool m_first_write;

	/** faked RIFF header for wav format */
	QByteArray m_header;

    };
}

#endif /* HAVE_PHONON_SUPPORT */

#endif /* _PLAY_BACK_PHONON_H_ */

//***************************************************************************
//***************************************************************************
