/***************************************************************************
       PlayBack-ALSA.cpp  -  playback device for ALSA
			     -------------------
    begin                : Sat Mar 05 2005
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

#include "config.h"
#ifdef HAVE_ALSA_SUPPORT

// #include <sys/types.h>
// #include <sys/stat.h>
// #include <sys/ioctl.h>
// #include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>

#include <klocale.h>

#include "libkwave/CompressionType.h"
#include "libkwave/SampleFormat.h"

#include "PlayBack-ALSA.h"

/** use at least 2^8 = 256 bytes for playback buffer !!! */
#define MIN_PLAYBACK_BUFFER 8

/** use at most 2^16 = 65536 bytes for playback buffer !!! */
#define MAX_PLAYBACK_BUFFER 16

/** highest available number of channels */
#define MAX_CHANNELS 2

//***************************************************************************
PlayBackALSA::PlayBackALSA()
    :PlayBackDevice(),
    m_device_name(),
    m_handle(0),
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
PlayBackALSA::~PlayBackALSA()
{
    close();
}

class AlsaPcmHwParamsGuard
{
public:

    /** Constructor, takes the hwparams and initializes it with 0 */
    AlsaPcmHwParamsGuard(snd_pcm_hw_params_t **params);

    /** Destructor, calls snd_pcm_hw_params_free() */
    virtual ~AlsaPcmHwParamsGuard();

private:

    /** pointer to the hwparams */
    snd_pcm_hw_params_t **m_params;
};

AlsaPcmHwParamsGuard::AlsaPcmHwParamsGuard(snd_pcm_hw_params_t **params)
    :m_params(params)
{
    if (m_params) *m_params = 0;
}

AlsaPcmHwParamsGuard::~AlsaPcmHwParamsGuard()
{
    if (m_params && *m_params) snd_pcm_hw_params_free(*m_params);
}

//***************************************************************************
QString PlayBackALSA::open(const QString &device, double rate,
                          unsigned int channels, unsigned int bits,
                          unsigned int bufbase)
{
    qDebug("PlayBackALSA::open(device=%s,rate=%0.1f,channels=%u,"\
	"bits=%u, bufbase=%u)", device.local8Bit().data(), rate, channels,
	bits, bufbase);

    int err, dir;
    snd_pcm_hw_params_t *hw_params;
    unsigned int buffer_time = 500000; /* ring buffer length in us */
    unsigned int period_time = 100000; /* period time in us */
    snd_pcm_uframes_t buffer_size;
    snd_pcm_uframes_t period_size;
    AlsaPcmHwParamsGuard _params_guard(&hw_params);

    m_device_name = device;
    m_rate        = rate;
    m_channels    = channels;
    m_bits        = bits;
    m_bufbase     = bufbase;
    m_buffer_size = 0;
    m_buffer_used = 0;
    m_handle      = 0;

    // round the number of effectively used bits up to whole bytes
    if (m_bits & 0x03) m_bits += 8;

    snd_output_t *output = NULL;
    err = snd_output_stdio_attach(&output, stderr, 0);
    if (err < 0) {
	qWarning("Output failed: %s", snd_strerror(err));
    }

    // close the previous device
    if (m_handle) snd_pcm_close(m_handle);
    m_handle = 0;

    // open a new one
    if ((err = snd_pcm_open(&m_handle, device.local8Bit().data(),
                            SND_PCM_STREAM_PLAYBACK,
                            SND_PCM_NONBLOCK)) < 0)
    {
	QString reason;
	switch (err) {
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
		reason = i18n("Opening the device '%1' failed (%2).").arg(
	            device).arg(QString::fromLocal8Bit(snd_strerror(err)));
	}
	return reason;
    }

#if 0
    // this would be very nice if it works, but currently it causes
    // only a segfault :-(
    if ((err = snd_spcm_init(m_handle,
	(unsigned int)m_rate,
	m_channels,
	SND_PCM_FORMAT_S16_LE,
	SND_PCM_SUBFORMAT_STD,
	SND_SPCM_LATENCY_MEDIUM,
	SND_PCM_ACCESS_RW_INTERLEAVED,
	SND_SPCM_XRUN_IGNORE
	)) < 0)
    {
	return i18n("Cannot set sample format for '%1' (%2).").arg(
	            device).arg(QString::fromLocal8Bit(snd_strerror(err)));
    }
#endif

    if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
	return i18n("Cannot allocate hardware parameter structure "\
	            " for '%1' (%2).").arg(
	            device).arg(QString::fromLocal8Bit(snd_strerror(err)));
    }

    if ((err = snd_pcm_hw_params_any(m_handle, hw_params)) < 0) {
	return i18n("Cannot initialize hardware parameters for "\
	            "'%1' (%2).").arg(
	            device).arg(QString::fromLocal8Bit(snd_strerror(err)));
    }

    if ((err = snd_pcm_hw_params_set_access(m_handle, hw_params,
         SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
    {
	return i18n("Cannot set access type for '%1' (%2).").arg(
	            device).arg(QString::fromLocal8Bit(snd_strerror(err)));
    }

    snd_pcm_format_t format =
	(m_bits ==  8) ? SND_PCM_FORMAT_S8  :
	(m_bits == 16) ? SND_PCM_FORMAT_S16 :
	(m_bits == 24) ? SND_PCM_FORMAT_S24 :
	(m_bits == 32) ? SND_PCM_FORMAT_S32 :
	SND_PCM_FORMAT_UNKNOWN;
    qDebug("PlayBackALSA::open: configuring for %u bits", m_bits);

    if ((err = snd_pcm_hw_params_set_format(m_handle, hw_params,
         format)) < 0)
    {
	return i18n("Cannot set sample format for '%1' (%2).").arg(
	            device).arg(QString::fromLocal8Bit(snd_strerror(err)));
    }

    if ((err = snd_pcm_hw_params_set_channels(m_handle, hw_params,
         channels)) < 0)
    {
	return i18n("Cannot set channel count for '%1' (%2).").arg(
	            device).arg(QString::fromLocal8Bit(snd_strerror(err)));
    }

    unsigned int rrate = (unsigned int)(rint(rate));
    if ((err = snd_pcm_hw_params_set_rate_near(m_handle, hw_params,
         &rrate, 0)) < 0)
    {
	qDebug("--> real rate = %u", rrate);
	return i18n("Cannot set sample rate for '%1' (%2).").arg(
	            device).arg(QString::fromLocal8Bit(snd_strerror(err)));
    }

    /* set the buffer time */
    if ((err = snd_pcm_hw_params_set_buffer_time_near(m_handle, hw_params,
         &buffer_time, &dir)) < 0)
    {
	return i18n("Cannot set buffer time (%1) for '%2' (%3).").arg(
	            buffer_time).arg(
	            device).arg(QString::fromLocal8Bit(snd_strerror(err)));
    }

    if ((err = snd_pcm_hw_params_get_buffer_size(hw_params,
        &buffer_size)) < 0)
    {
	return i18n("Cannot get buffer size of '%1' (%2).").arg(
	            device).arg(QString::fromLocal8Bit(snd_strerror(err)));
    }

    /* set the period time */
    if ((err = snd_pcm_hw_params_set_period_time_near(m_handle,
         hw_params, &period_time, &dir)) < 0)
    {
	return i18n("Cannot set period time (%1) for '%2' (%3).").arg(
	            period_time).arg(
	            device).arg(QString::fromLocal8Bit(snd_strerror(err)));
    }

    if ((err = snd_pcm_hw_params_get_period_size(hw_params,
        &period_size, &dir)) < 0)
    {
	return i18n("Cannot set period size (%1) for '%2' (%3).").arg(
	            period_size).arg(
	            device).arg(QString::fromLocal8Bit(snd_strerror(err)));
    }

    snd_pcm_dump(m_handle, output);

    if ((err = snd_pcm_hw_params(m_handle, hw_params)) < 0) {
	snd_pcm_dump(m_handle, output);
	return i18n("Cannot set parameters for '%1' (%2)").arg(
	            device).arg(QString::fromLocal8Bit(snd_strerror(err)));
    }

    if ((err = snd_pcm_prepare (m_handle)) < 0) {
        return i18n("cannot prepare audio interface '%1' for use (%2)").arg(
	            device).arg(QString::fromLocal8Bit(snd_strerror(err)));
    }

    // resize our buffer and reset it
    m_buffer_size = (1 << m_bufbase);
    m_buffer.resize(m_buffer_size);
    m_buffer_size = m_buffer.size();
    Q_ASSERT(m_buffer_size == (1U << m_bufbase));

    qDebug("PlayBackALSA::open: OK.");
    return 0;
}

//***************************************************************************
int PlayBackALSA::write(QMemArray<sample_t> &samples)
{

    Q_ASSERT (m_buffer_used < m_buffer_size);
    if (m_buffer_used >= m_buffer_size) {
	qWarning("PlayBackALSA::write(): buffer overflow ?!");
	return -EIO;
    }

    // convert into byte stream
    unsigned int channel;
    for (channel=0; channel < m_channels; channel++) {
	sample_t sample = samples[channel];

	if (m_bits <= 8) {
	    m_buffer[m_buffer_used++] = sample >> 16;
	} else if (m_bits <= 16) {
	    m_buffer[m_buffer_used++] = sample >> 8;
	    m_buffer[m_buffer_used++] = sample >> 16;
	} else if (m_bits <= 24) {
	    // play in 24 bit format
	    m_buffer[m_buffer_used++] = sample & 0x0FF;
	    m_buffer[m_buffer_used++] = sample >> 8;
	    m_buffer[m_buffer_used++] = sample >> 16;
	} else if (m_bits <= 32) {
	    // play in 32 bit format
	    m_buffer[m_buffer_used++] = 0x00;
	    m_buffer[m_buffer_used++] = sample & 0x0FF;
	    m_buffer[m_buffer_used++] = sample >> 8;
	    m_buffer[m_buffer_used++] = sample >> 16;
	} else return -EINVAL;
    }

    // write buffer to device if full
    if (m_buffer_used >= m_buffer_size) flush();

    return 0;
}

//***************************************************************************
void PlayBackALSA::flush()
{
    int err;

    if (!m_buffer_used) return; // nothing to do

    if (m_handle) {
	const unsigned int bytes_per_sample = (m_bits >> 3) * m_channels;
	const unsigned int samples = m_buffer_used / bytes_per_sample;
	const unsigned int buffer_samples = m_buffer_size / bytes_per_sample;
	double buffer_time = (m_rate > 0) ?
	    (1E3 * buffer_samples / m_rate) : 1E3;

	// wait untile device is available with timeout == 3 full buffers
	err = snd_pcm_wait(m_handle, 3 * (unsigned int)buffer_time);
	if (err < 0) {
	    qWarning("PlayBackALSA::flush() write timeout: (%s)",
	             snd_strerror (err));
	    m_buffer_used = 0;
	    return;
	}

	if ((err = snd_pcm_writei(m_handle, m_buffer.data(), samples))
	   != (int)samples)
	{
	    qWarning("PlayBackALSA::flush() failed: (%s)",
	             snd_strerror (err));
	}
    }

    m_buffer_used = 0;
}

//***************************************************************************
int PlayBackALSA::close()
{
    flush();

    // close the device handle
    if (m_handle) snd_pcm_close(m_handle);
    m_handle = 0;

    return 0;
}

//***************************************************************************
QStringList PlayBackALSA::supportedDevices()
{
    QStringList list;

    list.append("hw:0,0");
    list.append("hw:0,1");
    list.append("#EDIT#");
//     list.append("#SELECT#");

    return list;
}

//***************************************************************************
QString PlayBackALSA::fileFilter()
{
    QString filter;

    if (filter.length()) filter += "\n";
    filter += QString("adsp*|") + i18n("ALSA playback device (adsp*)");

    if (filter.length()) filter += "\n";
    filter += QString("*|") + i18n("Any device (*)");

    return filter;
}

//***************************************************************************
snd_pcm_t *PlayBackALSA::openDevice(const QString &device)
{
    snd_pcm_t *pcm = m_handle;

    qDebug("PlayBackALSA::openDevice(%s)", device.local8Bit().data());
    if (!device.length()) return 0;

    if (!pcm) {
	// open the device in case it's not already open
	int err = snd_pcm_open(&pcm, device.local8Bit().data(),
	                       SND_PCM_STREAM_PLAYBACK,
	                       SND_PCM_NONBLOCK);
	if (err < 0) pcm = 0;
    }
    if (!pcm) {
	qWarning("PlayBackALSA::openDevice('%s') - "\
	         "failed, errno=%d (%s)",
	         device.local8Bit().data(),
	         errno, strerror(errno));
    }

    return pcm;
}

//***************************************************************************
QValueList<unsigned int> PlayBackALSA::supportedBits(const QString &device)
{
    QValueList <unsigned int> bits;
    qDebug("PlayBackALSA::supportedBits(%s)", device.local8Bit().data());

    snd_pcm_hw_params_t *p;

    if (snd_pcm_hw_params_malloc(&p) < 0) return bits;

    snd_pcm_t *pcm = openDevice(device);
    if (!pcm) {
	snd_pcm_hw_params_free(p);
	return bits;
    }

    if (snd_pcm_hw_params_any(pcm, p) >= 0) {
	if (!snd_pcm_hw_params_test_format(pcm, p, SND_PCM_FORMAT_S8 ))
	    bits.append( 8);
	if (!snd_pcm_hw_params_test_format(pcm, p, SND_PCM_FORMAT_S16))
	    bits.append(16);
	if (!snd_pcm_hw_params_test_format(pcm, p, SND_PCM_FORMAT_S24))
	    bits.append(24);
	if (!snd_pcm_hw_params_test_format(pcm, p, SND_PCM_FORMAT_S32))
	    bits.append(32);
    }

    snd_pcm_hw_params_free(p);

    // close the device if *we* opened it
    if ((pcm != m_handle) && (pcm)) snd_pcm_close(pcm);

    return bits;
}

//***************************************************************************
int PlayBackALSA::detectChannels(const QString &device,
                                 unsigned int &min, unsigned int &max)
{
    min = max = 0;
    snd_pcm_hw_params_t *p;

    if (snd_pcm_hw_params_malloc(&p) < 0) return -1;

    snd_pcm_t *pcm = openDevice(device);
    if (!pcm) {
	snd_pcm_hw_params_free(p);
	return -1;
    }

    if (pcm) {
	if (snd_pcm_hw_params_any(pcm, p) >= 0) {
	    int err;

	    if ((err = snd_pcm_hw_params_get_channels_min(p, &min)) < 0)
		qWarning("PlayBackALSA::detectTracks: min: %s",
		         snd_strerror(err));
	    if ((err = snd_pcm_hw_params_get_channels_max(p, &max)) < 0)
		qWarning("PlayBackALSA::detectTracks: max: %s",
		         snd_strerror(err));
	}
    }
    snd_pcm_hw_params_free(p);

    // close the device if *we* opened it
    if ((pcm != m_handle) && (pcm)) snd_pcm_close(pcm);

    qDebug("PlayBackALSA::detectTracks, min=%u, max=%u", min, max);
    return 0;
}

#endif /* HAVE_ALSA_SUPPORT */

//***************************************************************************
//***************************************************************************
