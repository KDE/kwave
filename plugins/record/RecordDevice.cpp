/*************************************************************************
       RecordDevice.cpp  -  device for audio recording, currently only OSS
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

#include "config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>

#include "libkwave/CompressionType.h"
#include "libkwave/SampleFormat.h"

#include "RecordDevice.h"

#define MAX_CHANNELS 2 /**< highest available number of channels */

//***************************************************************************
RecordDevice::RecordDevice()
    :m_fd(-1)
{
}

//***************************************************************************
RecordDevice::~RecordDevice()
{
    close();
}

//***************************************************************************
int RecordDevice::open(const QString &dev)
{
    // close the device if it is still open
    Q_ASSERT(m_fd < 0);
    if (m_fd >= 0) close();
    if (dev.isEmpty()) return -1; // no device name

    // first of all: try to open the device itself
    int fd = ::open(dev.ascii(), O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
	qWarning("open failed, fd=%d, errno=%d (%s)",
	         fd, errno, strerror(errno));
	return -errno;
    }

    m_fd = fd;
    return m_fd;
}

//***************************************************************************
int RecordDevice::read(char *buffer, unsigned int length)
{
    fd_set rfds;
    struct timeval tv;
    int retval;
    int read_bytes = 0;

//  Q_ASSERT(m_fd >= 0);
    Q_ASSERT(buffer);
    if (m_fd < 0) return -EBADF; // file not opened
    if (!buffer) return -EINVAL; // buffer is null pointer

//     int blocksize = length;
//     int err = ioctl(m_fd, SNDCTL_DSP_GETBLKSIZE, &blocksize);
//     Q_ASSERT(!err);
//     qDebug("blocksize = %u", blocksize);
//     if (err) {
// 	blocksize = length;
//     }
//
//     int blocksize = (127 << 16) + 6;
//     err = ioctl(m_fd, SNDCTL_DSP_SETFRAGMENT, &blocksize);

    // determine the timeout for reading, use safety factor 2
    int rate = (int)sampleRate();
    if (rate < 1) rate = 1;
    unsigned int timeout = (length / rate) * 2;
    if (timeout < 2) timeout = 2;

    while (length) {
	FD_ZERO(&rfds);
	FD_SET(m_fd, &rfds);

	tv.tv_sec  = timeout;
	tv.tv_usec = 0;
	retval = select(m_fd+1, &rfds, 0, 0, &tv);

	if (retval == -1) {
	    if (errno == EINTR)
		return -errno; // return without warning

	    qWarning("RecordDevice::read() - select() failed errno=%d (%s)",
	             errno, strerror(errno));
	    return -errno;
	} else if (retval) {
	    int res = ::read(m_fd, buffer, length);
	    if ((res == -1) && (errno == EINTR))
		return -errno; // interrupted, return without warning

	    if ((res == -1) && (errno == EAGAIN))
		continue;

	    if (res < 0) {
		qWarning("RecordDevice::read() - read error %d (%s)",
		         errno, strerror(errno));
		return read_bytes;
	    }
	    read_bytes += res;
	    length -= res;
	    buffer += res;
	} else {
	    printf("No data within 5 seconds.\n");
	    return -EIO;
        }
    }

    return read_bytes;
}

//***************************************************************************
int RecordDevice::close()
{
    if (m_fd < 0) return 0; // already closed
    ::close(m_fd);
    m_fd = -1;

    return 0;
}

//***************************************************************************
int RecordDevice::detectTracks(unsigned int &min, unsigned int &max)
{
    Q_ASSERT(m_fd >= 0);
    int t;
    int err;

    // find the smalles number of tracks, limit to MAX_CHANNELS
    for (t=1; t < MAX_CHANNELS; t++) {
	int real_tracks = t;
	err = ioctl(m_fd, SOUND_PCM_WRITE_CHANNELS, &real_tracks);
	Q_ASSERT(real_tracks == t);
	if (err >= 0) {
	    int readback_tracks = real_tracks;
	    if (ioctl(m_fd, SOUND_PCM_READ_CHANNELS, &readback_tracks) >= 0) {
	        // readback succeeded
		real_tracks = readback_tracks;
	    };
	}
	if (err >= 0) {
	    min = real_tracks;
	    break;
	}
    }
    if (t >= MAX_CHANNELS) {
	// no minimum track number found :-o
	qWarning("no minimum track number found, err=%d",err);
	min = 0;
	max = 0;
	return err;
    }

    // find the highest number of tracks, start from MAX_CHANNELS downwards
    for (t=MAX_CHANNELS; t >= (int)min; t--) {
	int real_tracks = t;
	err = ioctl(m_fd, SOUND_PCM_WRITE_CHANNELS, &real_tracks);
	Q_ASSERT(real_tracks == t);
	if (err >= 0) {
	    int readback_tracks = real_tracks;
	    if (ioctl(m_fd, SOUND_PCM_READ_CHANNELS, &readback_tracks) >= 0) {
	        // readback succeeded
		real_tracks = readback_tracks;
	    };
	}
	if (err >= 0) {
	    max = real_tracks;
	    break;
	}
    }
    max = t;
    qDebug("RecordDevice::detectTracks, min=%u, max=%u",min,max);

    return 0;
}

//***************************************************************************
int RecordDevice::setTracks(unsigned int &tracks)
{
    Q_ASSERT(m_fd >= 0);

    // set the number of tracks in the device (must already be opened)
    int t = tracks;
    int err = ioctl(m_fd, SOUND_PCM_WRITE_CHANNELS, &t);
    if (err < 0) return err;

    // return the number of tracks if succeeded
    tracks = t;

    return 0;
}

//***************************************************************************
int RecordDevice::tracks()
{
    Q_ASSERT(m_fd >= 0);

    // read back the number of tracks
    int t = 0;
    int err = ioctl(m_fd, SOUND_PCM_READ_CHANNELS, &t);
    if (err < 0) return err;

    return t;
}

//***************************************************************************
QValueList<double> RecordDevice::detectSampleRates()
{
    QValueList<double> list;
    Q_ASSERT(m_fd >= 0);

    static const int known_rates[] = {
	  1000, // (just for testing)
	  2000, // (just for testing)
	  4000, // standard OSS
	  5125, // seen in Harmony driver (HP712, 715/new)
	  5510, // seen in AD1848 driver
	  5512, // seen in ES1370 driver
	  6215, // seen in ES188X driver
	  6615, // seen in Harmony driver (HP712, 715/new)
	  6620, // seen in AD1848 driver
	  7350, // seen in AWACS and Burgundy sound driver
	  8000, // standard OSS
	  8820, // seen in AWACS and Burgundy sound driver
	  9600, // seen in AD1848 driver
	 11025, // soundblaster
	 14700, // seen in AWACS and Burgundy sound driver
	 16000, // standard OSS
	 17640, // seen in AWACS and Burgundy sound driver
	 18900, // seen in Harmony driver (HP712, 715/new)
	 22050, // soundblaster
	 24000, // seen in NM256 driver
	 27428, // seen in Harmony driver (HP712, 715/new)
	 29400, // seen in AWACS and Burgundy sound driver
	 32000, // standard OSS
	 32768, // seen in CS4299 driver
	 33075, // seen in Harmony driver (HP712, 715/new)
	 37800, // seen in Harmony driver (HP712, 715/new)
	 44100, // soundblaster
	 48000, // AC97
	 64000, // AC97
	 88200, // seen in RME96XX driver
	 96000, // AC97
	128000, // (just for testing)
	192000, // AC97
	196000, // (just for testing)
	256000  // (just for testing)
    };

    // try all known sample rates
    for (unsigned int i=0; i < sizeof(known_rates)/sizeof(int); i++) {
	int rate = known_rates[i];
	int err = ioctl(m_fd, SOUND_PCM_WRITE_RATE, &rate);
	if (err >= 0) {
	    // try to read back
	    int real_rate = -1;
	    err = ioctl(m_fd, SOUND_PCM_READ_RATE, &real_rate);
	    if ((err >= 0) && (real_rate >= 0)) rate = real_rate;
	}
	if (err < 0) {
	    qDebug("RecordDevice::detectSampleRates(): "\
	           "sample rate %d Hz not supported", known_rates[i]);
	    continue;
	}

	// do not produce duplicates
	if (list.contains(rate)) continue;

	// qDebug("found rate %d Hz", rate);
	list.append(rate);
    }

    return list;
}

//***************************************************************************
int RecordDevice::setSampleRate(double &new_rate)
{
    Q_ASSERT(m_fd >= 0);
    int rate = (int)rint(new_rate); // OSS supports only integer rates

    // set the rate of the device (must already be opened)
    int err = ioctl(m_fd, SOUND_PCM_WRITE_RATE, &rate);
    if (err < 0) return err;

    // read back the sample rate for verification
    double r = sampleRate();

    // return the sample rate if succeeded
    new_rate = (r >= 0) ? r : rate;

    return 0;
}

//***************************************************************************
double RecordDevice::sampleRate()
{
    Q_ASSERT(m_fd >= 0);

    // get the rate from the device (must already be opened)
    int rate = 0;
    int err = ioctl(m_fd, SOUND_PCM_READ_RATE, &rate);
    if (err < 0) return err;

    return rate;
}

//***************************************************************************
void RecordDevice::format2mode(int format, int &compression,
                               int &bits, int &sample_format)
{

    switch (format) {
	case AFMT_MU_LAW:
	    compression   = AF_COMPRESSION_G711_ULAW;
	    sample_format = AF_SAMPFMT_TWOSCOMP;
	    bits          = 16;
	    break;
	case AFMT_A_LAW:
	    compression   = AF_COMPRESSION_G711_ALAW;
	    sample_format = AF_SAMPFMT_TWOSCOMP;
	    bits          = 16;
	    break;
	case AFMT_IMA_ADPCM:
	    compression   = AF_COMPRESSION_MS_ADPCM;
	    sample_format = AF_SAMPFMT_TWOSCOMP;
	    bits          = 16;
	    break;
	case AFMT_U8:
	    compression   = AF_COMPRESSION_NONE;
	    sample_format = AF_SAMPFMT_UNSIGNED;
	    bits          = 8;
	    break;
	case AFMT_S16_LE:
	case AFMT_S16_BE:
	    compression   = AF_COMPRESSION_NONE;
	    sample_format = AF_SAMPFMT_TWOSCOMP;
	    bits          = 16;
	    break;
	case AFMT_S8:
	    compression   = AF_COMPRESSION_NONE;
	    sample_format = AF_SAMPFMT_TWOSCOMP;
	    bits          = 8;
	    break;
	case AFMT_U16_LE:
	case AFMT_U16_BE:
	    compression   = AF_COMPRESSION_NONE;
	    sample_format = AF_SAMPFMT_UNSIGNED;
	    bits          = 16;
	    break;
	case AFMT_MPEG:
	    compression   = (int)CompressionType::MPEG_LAYER_II;
	    sample_format = AF_SAMPFMT_TWOSCOMP;
	    bits          = 16;
	    break;
#if 0
	case AFMT_AC3: /* Dolby Digital AC3 */
	    compression   = AF_COMPRESSION_NONE;
	    sample_format = 0;
	    bits          = 16;
	    break;
#endif
	default:
	    compression   = -1;
	    sample_format = -1;
	    bits          = -1;
    }

}

//***************************************************************************
int RecordDevice::mode2format(int compression, int bits, int sample_format)
{
    // first level: compression
    if (compression == AF_COMPRESSION_G711_ULAW) return AFMT_MU_LAW;
    if (compression == AF_COMPRESSION_G711_ALAW) return AFMT_A_LAW;
    if (compression == AF_COMPRESSION_MS_ADPCM)  return AFMT_IMA_ADPCM;
    if (compression == (int)CompressionType::MPEG_LAYER_II)  return AFMT_MPEG;

    // non-compressed: switch by sample format
    if ((sample_format == AF_SAMPFMT_UNSIGNED) && (bits == 8))
	return AFMT_U8;
    if ((sample_format == AF_SAMPFMT_TWOSCOMP) && (bits == 8))
        return AFMT_S8;

    // get supported modes, maybe one endianness is not supported
    int mask = 0;
    int err = ioctl(m_fd, SNDCTL_DSP_GETFMTS, &mask);
    if (err < 0) return bits;

    // unsigned 16 bit mode
    // note: we prefer the machine's native endianness if both are supported !
    if ((sample_format == AF_SAMPFMT_UNSIGNED) && (bits == 16)) {
	mask &= (AFMT_U16_LE | AFMT_U16_BE);
	if (mask != (AFMT_U16_LE | AFMT_U16_BE)) return mask;
#if defined(ENDIANESS_BIG)
	return AFMT_U16_BE;
#else
	return AFMT_U16_LE;
#endif
    }

    // signed 16 bit mode
    if ((sample_format == AF_SAMPFMT_TWOSCOMP) && (bits == 16)) {
	mask &= (AFMT_S16_LE | AFMT_S16_BE);
	if (mask != (AFMT_S16_LE | AFMT_S16_BE)) return mask;
#if defined(ENDIANESS_BIG)
	return AFMT_S16_BE;
#else
	return AFMT_S16_LE;
#endif
    }

    qWarning("RecordDevice: unknown format: sample_format=%d, bits=%d",
             sample_format, bits);
    return 0;
}

//***************************************************************************
QValueList<int> RecordDevice::detectCompressions()
{
    Q_ASSERT(m_fd >= 0);
    QValueList <int> compressions;
    compressions.clear();
    int err = 0;
    int mask = AFMT_QUERY;

    err = ioctl(m_fd, SNDCTL_DSP_GETFMTS, &mask);
    if (err < 0) return compressions;

    if (mask & AFMT_MPEG)
        compressions += (int)CompressionType::MPEG_LAYER_II;
    if (mask & AFMT_A_LAW)     compressions += AF_COMPRESSION_G711_ALAW;
    if (mask & AFMT_MU_LAW)    compressions += AF_COMPRESSION_G711_ULAW;
    if (mask & AFMT_IMA_ADPCM) compressions += AF_COMPRESSION_MS_ADPCM;
    if (mask & (AFMT_U16_LE | AFMT_U16_BE | AFMT_S16_LE | AFMT_S16_BE))
        compressions += AF_COMPRESSION_NONE;

    return compressions;
}

//***************************************************************************
int RecordDevice::setCompression(int new_compression)
{
    Q_ASSERT(m_fd >= 0);
    int format, compression, bits, sample_format;

    // read back current format
    int err = ioctl(m_fd, SOUND_PCM_READ_BITS, &format);
    if (err < 0) return err;
    format2mode(format, compression, bits, sample_format);

    // modify the compression
    compression = new_compression;

    // activate new format
    format = mode2format(compression, bits, sample_format);
    err = ioctl(m_fd, SNDCTL_DSP_SETFMT, &format);
    if (err < 0) return err;

    return 0;
}

//***************************************************************************
int RecordDevice::compression()
{
    Q_ASSERT(m_fd >= 0);
    int mask = AF_COMPRESSION_NONE;
    int err = ioctl(m_fd, SOUND_PCM_READ_BITS, &mask);
    if (err < 0) return AF_COMPRESSION_NONE;

    int c, b, s;
    format2mode(mask, c, b, s);
    return c;
}

//***************************************************************************
QValueList <unsigned int> RecordDevice::detectBitsPerSample()
{
    Q_ASSERT(m_fd >= 0);
    QValueList <unsigned int> bits;
    bits.clear();
    int err = 0;
    int mask = AFMT_QUERY;

    err = ioctl(m_fd, SNDCTL_DSP_GETFMTS, &mask);
    if (err < 0) return bits;

    // mask out all modes that do not match the current compression
    const int compression = this->compression();
    for (unsigned int bit=0; bit < (sizeof(mask) << 3); bit++) {
	if (!mask & (1 << bit)) continue;

	// format is supported, split into compression, bits, sample format
	int c, b, s;
	format2mode(1 << bit, c, b, s);
	if (b < 0) continue; // unknown -> skip

	// take the mode if compression matches and it is not already known
	if ((c == compression) && !(bits.contains(b))) {
	    bits += b;
	}
    }

#if 0
    if (mask | AFMT_AC3)
	qDebug("RecordDevice: your device supports AC3 which is not "\
	       "yet supported, sorry :-(");
#endif
    return bits;
}

//***************************************************************************
int RecordDevice::setBitsPerSample(unsigned int new_bits)
{
    Q_ASSERT(m_fd >= 0);
    int format, compression, bits, sample_format;

    // read back current format
    int err = ioctl(m_fd, SOUND_PCM_READ_BITS, &format);
    if (err < 0) return err;
    format2mode(format, compression, bits, sample_format);

    // modify the bits per sample
    bits = new_bits;

    // activate new format
    format = mode2format(compression, bits, sample_format);
    err = ioctl(m_fd, SNDCTL_DSP_SETFMT, &format);
    if (err < 0) return err;

    return 0;
}

//***************************************************************************
int RecordDevice::bitsPerSample()
{
    Q_ASSERT(m_fd >= 0);
    int mask = 0;
    int err = ioctl(m_fd, SOUND_PCM_READ_BITS, &mask);
    if (err < 0) return err;

    int c, b, s;
    format2mode(mask, c, b, s);
    return b;
}

//***************************************************************************
QValueList<int> RecordDevice::detectSampleFormats()
{
    Q_ASSERT(m_fd >= 0);
    QValueList <int> formats;
    formats.clear();
    int err = 0;
    int mask = AFMT_QUERY;

    err = ioctl(m_fd, SNDCTL_DSP_GETFMTS, &mask);
    if (err < 0) return formats;

    // mask out all modes that do not match the current compression
    // and bits per sample
    const int compression     = this->compression();
    const int bits_per_sample = this->bitsPerSample();
    for (unsigned int bit=0; bit < (sizeof(mask) << 3); bit++) {
	if (!mask & (1 << bit)) continue;

	// format is supported, split into compression, bits, sample format
	int c, b, s;
	format2mode(1 << bit, c, b, s);
	if (c < 0) continue; // unknown -> skip

	if ((c == compression) && (b == bits_per_sample)) {
	    // this mode matches -> append it if not already known
	    if (!formats.contains(s)) formats += s;
	}
    }

    return formats;
}

//***************************************************************************
int RecordDevice::setSampleFormat(int new_format)
{
    Q_ASSERT(m_fd >= 0);
    int format, compression, bits, sample_format;

    // read back current format
    int err = ioctl(m_fd, SOUND_PCM_READ_BITS, &format);
    if (err < 0) return err;
    format2mode(format, compression, bits, sample_format);

    // modify the sample format
    sample_format = new_format;

    // activate new format
    format = mode2format(compression, bits, sample_format);
    err = ioctl(m_fd, SNDCTL_DSP_SETFMT, &format);
    if (err < 0) return err;

    return 0;
}

//***************************************************************************
int RecordDevice::sampleFormat()
{
    Q_ASSERT(m_fd >= 0);
    int mask = 0;
    int err = ioctl(m_fd, SOUND_PCM_READ_BITS, &mask);
    if (err < 0) return err;

    int c, b, s;
    format2mode(mask, c, b, s);
    return s;
}

//***************************************************************************
//***************************************************************************
