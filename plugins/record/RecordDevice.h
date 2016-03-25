/*************************************************************************
         RecordDevice.h  -  base class for audio recording devices
                             -------------------
    begin                : Wed Sep 17 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
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

#ifndef RECORD_DEVICE_H
#define RECORD_DEVICE_H

#include "config.h"

#include <QByteArray>
#include <QList>
#include <QString>
#include <QStringList>

#include "libkwave/ByteOrder.h"
#include "libkwave/SampleFormat.h"
#include "libkwave/String.h"

namespace Kwave
{
    class RecordDevice
    {
    public:

	/** Constructor */
	RecordDevice() {}

	/** Destructor */
	virtual ~RecordDevice() {}

	/**
	 * Open the record device.
	 * @param dev path of the record device
	 * @return zero-length string if successful, or an error
	 *         message if failed
	 */
	virtual QString open(const QString &dev) = 0;

	/**
	 * Read the raw audio data from the record device.
	 * @param buffer array of bytes to receive the audio data
	 *        might be resized for alignment
	 * @param offset offset in bytes within the buffer
	 * @return number of bytes read, zero or negative if failed
	 */
	virtual int read(QByteArray &buffer, unsigned int offset) = 0;

	/** Close the device */
	virtual int close() = 0;

	/** return a string list with supported device names */
	virtual QStringList supportedDevices() {
	    return QStringList();
	}

	/** return a string suitable for a "File Open..." dialog */
	virtual QString fileFilter() { return _(""); }

	/**
	 * Detect the minimum and maximum number of tracks.
	 * If the detection fails, minimum and maximum are set to zero.
	 * @param min receives the lowest supported number of tracks
	 * @param max receives the highest supported number of tracks
	 * @return zero or positive number if ok,
	 *         negative error number if failed
	 */
	virtual int detectTracks(unsigned int &min, unsigned int &max) = 0;

	/**
	 * Try to set a new number of tracks.
	 * @note the device must be open
	 * @param tracks the number of tracks to be set, can be modified and
	 *        decreased to the next supported number of tracks if the
	 *        underlying driver supports that.
	 * @return zero on success, negative error code if failed
	 */
	virtual int setTracks(unsigned int &tracks) = 0;

	/** Returns the current number of tracks */
	virtual int tracks() = 0;

	/** get a list of supported sample rates */
	virtual QList<double> detectSampleRates() = 0;

	/**
	 * Try to set a new sample rate.
	 * @param new_rate the sample rate to be set [samples/second], can
	 *        be modified and rounded up/down to the nearest supported
	 *        sample rate if the underlying driver supports that.
	 * @return zero on success, negative error code if failed
	 */
	virtual int setSampleRate(double &new_rate) = 0;

	/** Returns the current sample rate of the device */
	virtual double sampleRate() = 0;

	/**
	 * Gets a list of supported compression types. If no compression is
	 * supported, the list might be empty.
	 */
	virtual QList<int> detectCompressions() = 0;

	/**
	 * Try to set a new compression type.
	 * @param new_compression the identifier of the new compression
	 * @return zero on success, negative error code if failed
	 * @see class Kwave::Compression
	 */
	virtual int setCompression(int new_compression) = 0;

	/** Returns the current compression type (0==none) */
	virtual int compression() = 0;

	/**
	 * Detect a list of supported bits per sample.
	 * @note this depends on the compression type
	 * @return a list of bits per sample, empty if failed
	 */
	virtual QList<unsigned int> supportedBits() = 0;

	/**
	 * Set the resolution in bits per sample
	 * @param new_bits resolution [bits/sample]
	 */
	virtual int setBitsPerSample(unsigned int new_bits) = 0;

	/**
	 * Returns the current resolution in bits per sample or a negative
	 * error code if failed
	 */
	virtual int bitsPerSample() = 0;

	/**
	 * Gets a list of supported sample formats.
	 * @note this depends on the current setting of the compression!
	 */
	virtual QList<Kwave::SampleFormat::Format> detectSampleFormats() = 0;

	/**
	 * Try to set a new sample format (signed/unsigned)
	 * @param new_format the identifier for the new format
	 * @return zero on success, negative error code if failed
	 * @see class SampleFormat
	 */
	virtual int setSampleFormat(Kwave::SampleFormat::Format new_format) = 0;

	/** Returns the current sample format (signed/unsigned) */
	virtual Kwave::SampleFormat::Format sampleFormat() = 0;

	/** Returns the current endianness (big/little) */
	virtual Kwave::byte_order_t endianness() = 0;

    };
}

#endif /* RECORD_DEVICE_H */

//***************************************************************************
//***************************************************************************
