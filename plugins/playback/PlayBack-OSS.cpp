/***************************************************************************
       PlayBack-OSS.cpp  -  playback device for standard linux OSS
			     -------------------
    begin                : Sat May 19 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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
#ifdef HAVE_OSS_SUPPORT

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>

#include <klocale.h>

#include "libkwave/CompressionType.h"
#include "libkwave/SampleFormat.h"

#include "PlayBack-OSS.h"

/** use at least 2^8 = 256 bytes for playback buffer !!! */
#define MIN_PLAYBACK_BUFFER 8

/** use at most 2^16 = 65536 bytes for playback buffer !!! */
#define MAX_PLAYBACK_BUFFER 16

/** highest available number of channels */
#define MAX_CHANNELS 2

//***************************************************************************
PlayBackOSS::PlayBackOSS()
    :PlayBackDevice(),
    m_device_name(),
    m_handle(-1),
    m_rate(0),
    m_channels(0),
    m_bits(0),
    m_bufbase(0),
    m_buffer(),
    m_buffer_size(0),
    m_buffer_used(0)
{
}

//***************************************************************************
PlayBackOSS::~PlayBackOSS()
{
    close();
}

//***************************************************************************
QString PlayBackOSS::open(const QString &device, double rate,
                          unsigned int channels, unsigned int bits,
                          unsigned int bufbase)
{
    qDebug("PlayBackOSS::open(device=%s,rate=%0.1f,channels=%u,"\
	"bits=%u, bufbase=%u)", device.local8Bit().data(), rate, channels,
	bits, bufbase);

    m_device_name = device;
    m_rate        = rate;
    m_channels    = channels;
    m_bits        = bits;
    m_bufbase     = bufbase;
    m_buffer_size = 0;
    m_buffer_used = 0;
    m_handle      = 0;

    // prepeare for playback by opening the sound device
    // and initializing with the proper settings
    m_handle = ::open(m_device_name.local8Bit(), O_WRONLY | O_NONBLOCK);
    if (m_handle == -1) {
	QString reason;
	switch (errno) {
	    case ENOENT:
	    case ENODEV:
	    case ENXIO:
	    case EIO:
		reason = i18n("i/o error, maybe the driver\n"\
		"is not present in your kernel or it is not\n"\
		"properly configured.");
		break;
	    case EBUSY:
		reason = i18n(
		"The device is busy. Maybe an other application is \n"\
		"currently using it. Please try again later. \n"\
		"(Hint: you might find out the name and process id of \n"\
		"the program by calling: \"fuser -v %1\" \n"\
		"on the command line.)").arg(
		m_device_name);
		break;
	    default:
		reason = strerror(errno);
	}
	return reason;
    }

    // the device was opened in non-blocking mode to detect if it is
    // busy or not - but from now on we need blocking mode again
    ::fcntl(m_handle, F_SETFL, fcntl(m_handle, F_GETFL) & ~O_NONBLOCK);
    if (fcntl(m_handle, F_GETFL) & O_NONBLOCK) {
	// resetting O:NONBLOCK failed
	return i18n("The device '%1' cannot be opened "\
	            "in the correct mode.").arg(m_device_name);
    }

    int format = (m_bits == 8) ? AFMT_U8 : AFMT_S16_LE;

    // number of bits per sample
    if (ioctl(m_handle, SNDCTL_DSP_SAMPLESIZE, &format) == -1) {
	return i18n("%1 bits per sample are not supported").arg(m_bits);
    }

    // mono/stereo selection
    int stereo = (m_channels >= 2) ? 1 : 0;
    if (ioctl(m_handle, SNDCTL_DSP_STEREO, &stereo) == -1) {
	return i18n("stereo playback is not supported");
    }

    // playback rate
    int int_rate = (int)m_rate;
    if (ioctl(m_handle, SNDCTL_DSP_SPEED, &int_rate) == -1) {
	return i18n("playback rate %1 Hz is not supported").arg(int_rate);
    }

    // buffer size
    Q_ASSERT(bufbase >= MIN_PLAYBACK_BUFFER);
    Q_ASSERT(bufbase <= MAX_PLAYBACK_BUFFER);
    if (bufbase < MIN_PLAYBACK_BUFFER) bufbase = MIN_PLAYBACK_BUFFER;
    if (bufbase > MAX_PLAYBACK_BUFFER) bufbase = MAX_PLAYBACK_BUFFER;
    if (ioctl(m_handle, SNDCTL_DSP_SETFRAGMENT, &bufbase) == -1) {
	return i18n("unusable buffer size: %1").arg(1 << bufbase);
    }

    // get the real buffer size in bytes
    ioctl(m_handle, SNDCTL_DSP_GETBLKSIZE, &m_buffer_size);

    // resize our buffer and reset it
    m_buffer.resize(m_buffer_size);

    return 0;
}

//***************************************************************************
int PlayBackOSS::write(QMemArray<sample_t> &samples)
{
    Q_ASSERT (m_buffer_used < m_buffer_size);
    if (m_buffer_used >= m_buffer_size) {
	qWarning("PlayBackOSS::write(): buffer overflow ?!");
	return -EIO;
    }

    // convert into byte stream
    unsigned int channel;
    for (channel=0; channel < m_channels; channel++) {
	sample_t sample = samples[channel];

	switch (m_bits) {
	    case 8:
		sample += 1 << 23;
		m_buffer[m_buffer_used++] = sample >> 16;
		break;
	    case 16:
		sample += 1 << 23;
		m_buffer[m_buffer_used++] = sample >> 8;
		m_buffer[m_buffer_used++] = (sample >> 16) + 128;
		break;
	    case 24:
		// play in 32 bit format
		m_buffer[m_buffer_used++] = 0x00;
		m_buffer[m_buffer_used++] = sample & 0x0FF;
		m_buffer[m_buffer_used++] = sample >> 8;
		m_buffer[m_buffer_used++] = (sample >> 16) + 128;
		break;
	    default:
		return -EINVAL;
	}
    }

    // write buffer to device if full
    if (m_buffer_used >= m_buffer_size) flush();

    return 0;
}

//***************************************************************************
void PlayBackOSS::flush()
{
    if (!m_buffer_used) return; // nothing to do
    if (m_handle) ::write(m_handle, m_buffer.data(), m_buffer_used);
    m_buffer_used = 0;
}

//***************************************************************************
int PlayBackOSS::close()
{
    flush();

    // close the device handle
    if (m_handle) ::close(m_handle);
    return 0;
}

//***************************************************************************
QStringList PlayBackOSS::supportedDevices()
{
    QStringList list;

    list.append("/dev/dsp");
    list.append("/dev/dsp0");
    list.append("/dev/dsp1");
    list.append("/dev/dsp2");
    list.append("/dev/audio");
    list.append("/dev/sound/dsp");
    list.append("/dev/sound/dsp0");
    list.append("/dev/sound/dsp1");
    list.append("/dev/sound/dsp2");
    list.append("/dev/sound/audio");
    list.append("/dev/adsp");
    list.append("/dev/adsp0");
    list.append("/dev/adsp1");
    list.append("/dev/adsp2");
    list.append("/dev/dio");
    list.append("#EDIT#");
    list.append("#SELECT#");

    return list;
}

//***************************************************************************
QString PlayBackOSS::fileFilter()
{
    QString filter;

    if (filter.length()) filter += "\n";
    filter += QString("dsp*|") + i18n("OSS playback device (dsp*)");

    if (filter.length()) filter += "\n";
    filter += QString("adsp*|") + i18n("ALSA playback device (adsp*)");

    if (filter.length()) filter += "\n";
    filter += QString("*|") + i18n("Any device (*)");

    return filter;
}

//***************************************************************************
void PlayBackOSS::format2mode(int format, int &compression,
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
int PlayBackOSS::openDevice(const QString &device)
{
    int fd = m_handle;

//     qDebug("PlayBackOSS::openDevice(%s)", device.local8Bit().data());
    if (!device.length()) return -1;

    if (fd <= 0) {
	// open the device in case it's not already open
	fd = ::open(device.local8Bit(), O_WRONLY | O_NONBLOCK);
    }
    if (fd <= 0) {
	qWarning("PlayBackOSS::openDevice('%s') - "\
	         "failed, errno=%d (%s)",
	         device.local8Bit().data(),
	         errno, strerror(errno));
    }

    return fd;
}

//***************************************************************************
QValueList<unsigned int> PlayBackOSS::supportedBits(const QString &device)
{
    QValueList <unsigned int> bits;
    bits.clear();
    int err = -1;
    int mask = AFMT_QUERY;
    int fd;

//     qDebug("PlayBackOSS::supportedBits(%s)", device.local8Bit().data());

    fd = openDevice(device);
    if (fd >= 0) {
	err = ::ioctl(fd, SNDCTL_DSP_GETFMTS, &mask);
	if (err < 0) {
	    qWarning("PlayBackOSS::supportedBits() - "\
	             "SNDCTL_DSP_GETFMTS failed, "\
	             "fd=%d, result=%d, error=%d (%s)",
	             fd, err, errno, strerror(errno));
	}
    }

    // close the device if *we* opened it
    if ((fd != m_handle) && (fd >= 0)) ::close(fd);

    if (err < 0) return bits;

    // mask out all modes that do not match the current compression
    const int compression = AF_COMPRESSION_NONE;
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

    return bits;
}

//***************************************************************************
int PlayBackOSS::detectChannels(const QString &device,
                                unsigned int &min, unsigned int &max)
{
    int fd, t, err;

    min = 0;
    max = 0;

    fd = openDevice(device);
    if (fd < 0) return -1;

    // find the smalles number of tracks, limit to MAX_CHANNELS
    for (t=1; t < MAX_CHANNELS; t++) {
	int real_tracks = t;
	err = ioctl(fd, SOUND_PCM_WRITE_CHANNELS, &real_tracks);
	Q_ASSERT(real_tracks == t);
	if (err >= 0) {
	    int readback_tracks = real_tracks;
	    if (ioctl(fd, SOUND_PCM_READ_CHANNELS, &readback_tracks) >= 0) {
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
	// close the device if *we* opened it
	if ((fd != m_handle) && (fd >= 0)) ::close(fd);
	return err;
    }

    // find the highest number of tracks, start from MAX_CHANNELS downwards
    for (t=MAX_CHANNELS; t >= (int)min; t--) {
	int real_tracks = t;
	err = ioctl(fd, SOUND_PCM_WRITE_CHANNELS, &real_tracks);
	Q_ASSERT(real_tracks == t);
	if (err >= 0) {
	    int readback_tracks = real_tracks;
	    if (ioctl(fd, SOUND_PCM_READ_CHANNELS, &readback_tracks) >= 0) {
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
//     qDebug("PlayBackOSS::detectTracks, min=%u, max=%u", min, max);

    // close the device if *we* opened it
    if ((fd != m_handle) && (fd >= 0)) ::close(fd);
    return 0;
}

#endif /* HAVE_OSS_SUPPORT */

//***************************************************************************
//***************************************************************************
