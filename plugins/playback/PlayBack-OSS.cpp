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

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h>
#include <math.h>

#include <klocale.h>

#include "PlayBack-OSS.h"

/** use at least 2^8 = 256 bytes for playback buffer !!! */
#define MIN_PLAYBACK_BUFFER 8

/** use at most 2^16 = 65536 bytes for playback buffer !!! */
#define MAX_PLAYBACK_BUFFER 16

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
    debug("PlayBackOSS::open(device=%s,rate=%f,channels=%u,"\
	"bits=%u, bufbase=%u)", device.latin1(), rate, channels,
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
    m_handle = ::open(m_device_name.latin1(), O_WRONLY | O_NONBLOCK);
    debug("PlayBackOSS::open(): file descriptor=%u", m_handle);
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
	warning("PlayBackOSS::write(): buffer overflow ?!");
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
//***************************************************************************
