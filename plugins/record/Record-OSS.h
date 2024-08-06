/*************************************************************************
           Record-OSS.h  -  device for audio recording via OSS
                             -------------------
    begin                : Sun Jul 24 2005
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

#ifndef RECORD_OSS_H
#define RECORD_OSS_H

#include "config.h"
#ifdef HAVE_OSS_SUPPORT

#include <QByteArray>
#include <QString>

#include "RecordDevice.h"

namespace Kwave
{
    class RecordOSS: public Kwave::RecordDevice
    {
    public:

        /** Constructor */
        RecordOSS();

        /** Destructor */
        virtual ~RecordOSS() override;

        /**
         * Open the record device.
         * @param dev path of the record device
         * @return zero-length string if successful, or an error
         *         message if failed
         */
        virtual QString open(const QString &dev) override;

        /**
         * Read the raw audio data from the record device.
         * @param buffer array of bytes to receive the audio data
         *        might be resized for alignment
         * @param offset offset in bytes within the buffer
         * @return number of bytes read, zero or negative if failed
         */
        virtual int read(QByteArray &buffer, unsigned int offset)
            override;

        /** Close the device */
        virtual int close() override;

        /** return a string list with supported device names */
        virtual QStringList supportedDevices() override;

        /** return a string suitable for a "File Open..." dialog */
        virtual QString fileFilter() override;

        /**
         * Detect the minimum and maximum number of tracks.
         * If the detection fails, minimum and maximum are set to zero.
         * @param min receives the lowest supported number of tracks
         * @param max receives the highest supported number of tracks
         * @return zero or positive number if ok,
         *         negative error number if failed
         */
        virtual int detectTracks(unsigned int &min, unsigned int &max)
            override;

        /**
         * Try to set a new number of tracks.
         * @note the device must be open
         * @param tracks the number of tracks to be set, can be modified and
         *        decreased to the next supported number of tracks if the
         *        underlying driver supports that.
         * @return zero on success, negative error code if failed
         */
        virtual int setTracks(unsigned int &tracks) override;

        /** Returns the current number of tracks */
        virtual int tracks() override;

        /** get a list of supported sample rates */
        virtual QList<double> detectSampleRates() override;

        /**
         * Try to set a new sample rate.
         * @param new_rate the sample rate to be set [samples/second], can
         *        be modified and rounded up/down to the nearest supported
         *        sample rate if the underlying driver supports that.
         * @return zero on success, negative error code if failed
         */
        virtual int setSampleRate(double &new_rate) override;

        /** Returns the current sample rate of the device */
        virtual double sampleRate() override;

        /**
         * Gets a list of supported compression types. If no compression is
         * supported, the list might be empty.
         */
        virtual QList<Kwave::Compression::Type> detectCompressions()
            override;

        /**
         * Try to set a new compression type.
         * @param new_compression the identifier of the new compression
         * @return zero on success, negative error code if failed
         * @see class Compression
         */
        virtual int setCompression(Kwave::Compression::Type new_compression)
            override;

        /** Returns the current compression type (0==none) */
        virtual Kwave::Compression::Type compression() override;

        /**
         * Detect a list of supported bits per sample.
         * @note this depends on the compression type
         * @return a list of bits per sample, empty if failed
         */
        virtual QList <unsigned int> supportedBits() override;

        /**
         * Set the resolution in bits per sample
         * @param new_bits resolution [bits/sample]
         */
        virtual int setBitsPerSample(unsigned int new_bits) override;

        /**
         * Returns the current resolution in bits per sample or a negative
         * error code if failed
         */
        virtual int bitsPerSample() override;

        /**
         * Gets a list of supported sample formats.
         * @note this depends on the current setting of the compression!
         */
        virtual QList<Kwave::SampleFormat::Format> detectSampleFormats()
            override;

        /**
         * Try to set a new sample format (signed/unsigned)
         * @param new_format the identifier for the new format
         * @return zero on success, negative error code if failed
         * @see class SampleFormat
         */
        virtual int setSampleFormat(Kwave::SampleFormat::Format new_format)
            override;

        /** Returns the current sample format (signed/unsigned) */
        virtual Kwave::SampleFormat::Format sampleFormat() override;

        /** Returns the current endianness (big/little/cpu) */
        virtual Kwave::byte_order_t endianness() override;

    private:

        /**
         * split a device format bitmask into it's parameters.
         * @param format the device specific format
         * @param compression receives a compression type
         * @see Compression
         * @param bits receives the number of bits per sample, related
         *        to the decoded stream
         * @param sample_format receives the sample format, as defined in
         *        libaudiofile (signed or unsigned)
         */
        void format2mode(int format, Kwave::Compression::Type &compression,
                         int &bits,
                         Kwave::SampleFormat::Format &sample_format);

        /**
         * create a device format bitmask from it's parameters.
         * @param compression the compression type
         * @see Compression
         * @param bits the number of bits per sample, related
         *        to the decoded stream
         * @param sample_format the sample format, as defined in
         *        libaudiofile (signed or unsigned)
         * @return the device specific format
         */
        int mode2format(Kwave::Compression::Type compression, int bits,
                        Kwave::SampleFormat::Format sample_format);

    private:

        /** file descriptor of the device or -1 if not open */
        int m_fd;

        /** sample rate  */
        int m_rate;

        /** number of tracks  */
        int m_tracks;

        /** OSS driver version  */
        int m_oss_version;
    };
}

#endif /* HAVE_OSS_SUPPORT */

#endif /* RECORD_OSS_H */

//***************************************************************************
//***************************************************************************
