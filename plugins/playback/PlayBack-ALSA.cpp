/***************************************************************************
       PlayBack-ALSA.cpp  -  playback device for ALSA
			     -------------------
    begin                : Sat Mar 05 2005
    copyright            : (C) 2005 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de

    much of this code is based on "aplay.c" from alsa-utils-1.0.8
    from the ALSA project, see http://www.alsa-project.org/

    Copyright (c) by Jaroslav Kysela <perex@suse.cz>
    Based on vplay program by Michael Beck
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307 USA                                                    *
 *                                                                         *
 ***************************************************************************/

#include "config.h"
#ifdef HAVE_ALSA_SUPPORT

#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>

#include <klocale.h>

#include "libkwave/CompressionType.h"
#include "libkwave/SampleFormat.h"

#include "PlayBack-ALSA.h"

/** sleep seconds, only used for recording, not used here */
static const unsigned int g_sleep_min = 0;

QMap<QString, QString> PlayBackALSA::m_device_list;

//***************************************************************************
PlayBackALSA::PlayBackALSA()
    :PlayBackDevice(),
    m_device_name(),
    m_handle(0),
    m_rate(0),
    m_channels(0),
    m_bits(0),
    m_bytes_per_sample(0),
    m_bufbase(0),
    m_buffer(),
    m_buffer_size(0),
    m_buffer_used(0),
    m_format(),
    m_chunk_size(0)//,
//     m_device_list()
{
}

//***************************************************************************
PlayBackALSA::~PlayBackALSA()
{
    close();
}

//***************************************************************************
int PlayBackALSA::setFormat(snd_pcm_hw_params_t *hw_params, unsigned int bits)
{
//     qDebug("PlayBackALSA::setFormat(..., bits=%u)", bits);

    // round the number of effectively used bits up to whole bytes
    if (bits & 0x03) bits = (bits & ~0x3) + 8;

    switch (bits) {
	case 8:
	    m_bits = 8;
	    m_format = SND_PCM_FORMAT_U8;
	    m_bytes_per_sample = 1 * m_channels;
	    break;
	case 16:
	    m_bits = 16;
	    m_format = SND_PCM_FORMAT_S16_LE;
	    m_bytes_per_sample = 2 * m_channels;
	    break;
	case 24:
	    if (!snd_pcm_hw_params_test_format(m_handle, hw_params,
	        SND_PCM_FORMAT_S24_3LE))
	    {
		m_bits = 24;
		m_format = SND_PCM_FORMAT_S24_3LE;
		m_bytes_per_sample = 3 * m_channels;
	    } else if (!snd_pcm_hw_params_test_format(m_handle, hw_params,
	        SND_PCM_FORMAT_S24))
	    {
		m_bits = 24;
		m_format = SND_PCM_FORMAT_S24_LE;
		m_bytes_per_sample = 4 * m_channels;
	    } else {
		m_bits = 0;
		m_format = SND_PCM_FORMAT_UNKNOWN;
		m_bytes_per_sample = 0;
	    }
	    break;
	case 32:
	    m_bits = 32;
	    m_format = SND_PCM_FORMAT_S32_LE;
	    m_bytes_per_sample = 4 * m_channels;
	    break;
	default:
	    m_bits = 0;
	    m_format = SND_PCM_FORMAT_UNKNOWN;
	    m_bytes_per_sample = 0;
    }
    Q_ASSERT(m_bits);
    Q_ASSERT(m_bytes_per_sample);
    Q_ASSERT(m_format != SND_PCM_FORMAT_UNKNOWN);

    int err;

    // check if the format really is okay
    if (m_format == SND_PCM_FORMAT_UNKNOWN) {
	qWarning("PlayBackALSA::setFormat(): %u bit is not supported", bits);
	return -EINVAL;
    }

    err = snd_pcm_hw_params_test_format(m_handle, hw_params, m_format);
    Q_ASSERT(!err);
    if (err) {
	qWarning("PlayBackALSA::setFormat(): %u bit is not supported", bits);
	m_bits = 0;
	m_format = SND_PCM_FORMAT_UNKNOWN;
	m_bytes_per_sample = 0;
	return -EINVAL;
    }

    // activate the settings
    err = snd_pcm_hw_params_set_format(m_handle, hw_params, m_format);
    Q_ASSERT(!err);

    return err;
}

//***************************************************************************
int PlayBackALSA::openDevice(const QString &device, unsigned int rate,
                             unsigned int channels, unsigned int bits)
{
    int err;
    snd_output_t *output = NULL;
    snd_pcm_hw_params_t *hw_params = 0;
    snd_pcm_sw_params_t *sw_params = 0;
    snd_pcm_uframes_t buffer_size;
    unsigned period_time = 0; // period time in us
    unsigned buffer_time = 0; // ring buffer length in us
    snd_pcm_uframes_t period_frames = 0;
    snd_pcm_uframes_t buffer_frames = 0;
    snd_pcm_uframes_t xfer_align;
    size_t n;
    snd_pcm_uframes_t start_threshold, stop_threshold;
    const int avail_min = -1;
    const int start_delay = 0;
    const int stop_delay = 0;

    m_chunk_size = 0;

    // translate verbose name to internal ALSA name
    QString alsa_device = alsaDeviceName(device);
    qDebug("PlayBackALSA::openDevice() - opening ALSA device '%s'",
           alsa_device.data());

    // workaround for bug in ALSA
    // if the device name ends with "," -> invalid name
    if (alsa_device.endsWith(",")) return -ENODEV;

    Q_ASSERT(rate);
    Q_ASSERT(channels);
    Q_ASSERT(bits);
    if (!rate) return -EINVAL;
    if (!channels) return -EINVAL;
    if (!bits) return -EINVAL;

    err = snd_output_stdio_attach(&output, stderr, 0);
    if (err < 0) {
	qWarning("Output failed: %s", snd_strerror(err));
    }

    // open a new one
    err = snd_pcm_open(&m_handle, alsa_device.local8Bit().data(),
                            SND_PCM_STREAM_PLAYBACK,
                            SND_PCM_NONBLOCK);
    if (err < 0) return err;

#if 0
    // this would be very nice if it works, but currently (alsa-1.0.8)
    // it causes only a segfault :-(
    err = snd_spcm_init(m_handle,
	(unsigned int)m_rate,
	m_channels,
	SND_PCM_FORMAT_S16_LE,
	SND_PCM_SUBFORMAT_STD,
	SND_SPCM_LATENCY_MEDIUM,
	SND_PCM_ACCESS_RW_INTERLEAVED,
	SND_SPCM_XRUN_IGNORE
	);
    if (err < 0) {
	qWarning("Cannot initialize '%s': %s",
	         device.local8Bit().data(), snd_strerror(err));
	return err;
    }
#else
    snd_pcm_hw_params_alloca(&hw_params);
    if ((err = snd_pcm_hw_params_any(m_handle, hw_params)) < 0) {
	qWarning("Cannot initialize hardware parameters: %s",
	         snd_strerror(err));
	snd_output_close(output);
	return err;
    }

    err = snd_pcm_hw_params_set_access(m_handle, hw_params,
         SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0) {
	qWarning("Cannot set access type: %s", snd_strerror(err));
	snd_output_close(output);
	return err;
    }

    err = setFormat(hw_params, bits);
    if (err < 0) {
	qWarning("Cannot set sample format: %s", snd_strerror(err));
	snd_output_close(output);
	return err;
    }

    err = snd_pcm_hw_params_set_channels(m_handle, hw_params, channels);
    if (err < 0) {
	qWarning("Cannot set channel count: %s", snd_strerror(err));
	snd_output_close(output);
	return err;
    }

    unsigned int rrate = rate;
    err = snd_pcm_hw_params_set_rate_near(m_handle, hw_params, &rrate, 0);
    if (err < 0) {
	qWarning("Cannot set sample rate: %s", snd_strerror(err));
	snd_output_close(output);
	return err;
    }
    qDebug("   real rate = %u", rrate);
    if ((float)rate * 1.05 < rrate || (float)rate * 0.95 > rrate) {
	qWarning("rate is not accurate (requested = %iHz, got = %iHz)",
	          rate, rrate);
	qWarning("         please, try the plug plugin (-Dplug:%s)",
	         snd_pcm_name(m_handle));
    }
    rate = rrate;

    if ((buffer_time) == 0 && (buffer_frames) == 0) {
	err = snd_pcm_hw_params_get_buffer_time_max(hw_params, &buffer_time, 0);
	Q_ASSERT(err >= 0);
	if (buffer_time > 500000) buffer_time = 500000;
    }

    if ((period_time == 0) && (period_frames == 0)) {
	if (buffer_time > 0)
	    period_time = buffer_time / 4;
	else
	    period_frames = buffer_frames / 4;
    }

    if (period_time > 0) {
	err = snd_pcm_hw_params_set_period_time_near(m_handle, hw_params,
	                                             &period_time, 0);
    } else {
	err = snd_pcm_hw_params_set_period_size_near(m_handle, hw_params,
	                                             &period_frames, 0);
    }
    Q_ASSERT(err >= 0);
    if (buffer_time > 0) {
	err = snd_pcm_hw_params_set_buffer_time_near(m_handle, hw_params,
	                                             &buffer_time, 0);
    } else {
	err = snd_pcm_hw_params_set_buffer_size_near(m_handle, hw_params,
	                                             &buffer_frames);
    }
    Q_ASSERT(err >= 0);

    qDebug("   setting hw_params");
    err = snd_pcm_hw_params(m_handle, hw_params);
    if (err < 0) {
	snd_pcm_dump(m_handle, output);
	snd_output_close(output);
	qWarning("Cannot set parameters: %s", snd_strerror(err));
	return err;
    }

    snd_pcm_hw_params_get_period_size(hw_params, &m_chunk_size, 0);
    snd_pcm_hw_params_get_buffer_size(hw_params, &buffer_size);
    if (m_chunk_size == buffer_size) {
	qWarning("Can't use period equal to buffer size (%lu == %lu)",
	         m_chunk_size, buffer_size);
	snd_output_close(output);
	return -EIO;
    }

    /* set software parameters */
    snd_pcm_sw_params_alloca(&sw_params);
    err = snd_pcm_sw_params_current(m_handle, sw_params);
    if (err < 0) {
	qWarning("Unable to determine current software parameters: %s",
	         snd_strerror(err));
	snd_output_close(output);
	return err;
    }

    err = snd_pcm_sw_params_get_xfer_align(sw_params, &xfer_align);
    if (err < 0) {
	qWarning("Unable to obtain xfer align: %s", snd_strerror(err));
	snd_output_close(output);
	return err;
    }
    if (g_sleep_min) xfer_align = 1;

    err = snd_pcm_sw_params_set_sleep_min(m_handle, sw_params, g_sleep_min);
    Q_ASSERT(err >= 0);
    if (avail_min < 0)
	n = m_chunk_size;
    else
	n = (snd_pcm_uframes_t)((double)rate * avail_min / 1000000);

    err = snd_pcm_sw_params_set_avail_min(m_handle, sw_params, n);

    /* round up to closest transfer boundary */
    n = (buffer_size / xfer_align) * xfer_align;
    start_threshold = (snd_pcm_uframes_t)
                      ((double)rate * start_delay / 1000000);
    if (start_delay <= 0) start_threshold += n;

    if (start_threshold < 1) start_threshold = 1;
    if (start_threshold > n) start_threshold = n;
    err = snd_pcm_sw_params_set_start_threshold(m_handle, sw_params,
                                                start_threshold);
    Q_ASSERT(err >= 0);
    stop_threshold = (snd_pcm_uframes_t)
                     ((double)rate * stop_delay / 1000000);
    if (stop_delay <= 0) stop_threshold += buffer_size;

    err = snd_pcm_sw_params_set_stop_threshold(m_handle, sw_params,
                                               stop_threshold);
    Q_ASSERT(err >= 0);

    err = snd_pcm_sw_params_set_xfer_align(m_handle, sw_params, xfer_align);
    Q_ASSERT(err >= 0);

    // write the software parameters to the playback device
    err = snd_pcm_sw_params(m_handle, sw_params);
    if (err < 0) {
	qDebug("   activating snd_pcm_sw_params FAILED");
	snd_pcm_dump(m_handle, output);
	qWarning("Unable to set software parameters: %s", snd_strerror(err));
    }
#endif

    snd_pcm_dump(m_handle, output);
    snd_output_close(output);

    // prepare the device for playback
    if ((err = snd_pcm_prepare(m_handle)) < 0) {
	snd_pcm_dump(m_handle, output);
	qWarning("cannot prepare interface for use: %s",snd_strerror(err));
    }

    return 0;
}

//***************************************************************************
QString PlayBackALSA::open(const QString &device, double rate,
                          unsigned int channels, unsigned int bits,
                          unsigned int bufbase)
{
    qDebug("PlayBackALSA::open(device=%s,rate=%0.1f,channels=%u,"\
	"bits=%u, bufbase=%u)", device.local8Bit().data(), rate, channels,
	bits, bufbase);

    m_device_name = device;
    m_rate        = rate;
    m_channels    = channels;
    m_bits        = 0;
    m_bufbase     = bufbase;
    m_buffer_size = 0;
    m_buffer_used = 0;
    m_handle      = 0;

    // close the previous device
    if (m_handle) snd_pcm_close(m_handle);
    m_handle = 0;

    int err = openDevice(device, (unsigned int)rate, channels, bits);
    if (err) {
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
		reason = i18n("Opening the device '%1' failed: %2").arg(
	            device).arg(QString::fromLocal8Bit(snd_strerror(err)));
	}
	return reason;
    }

    // resize our buffer and reset it
    Q_ASSERT(m_chunk_size);
    Q_ASSERT(m_bytes_per_sample);
    unsigned int chunk_bytes = m_chunk_size * m_bytes_per_sample;
    Q_ASSERT(chunk_bytes);
    if (!chunk_bytes) return 0;
    unsigned int n = (unsigned int)(ceil((float)(1 << m_bufbase) /
                                         (float)chunk_bytes));
    if (n < 1) n = 1;
    m_buffer_size = n * m_chunk_size * m_bytes_per_sample;
    m_buffer.resize(m_buffer_size);
    m_buffer_size = m_buffer.size();

    qDebug("PlayBackALSA::open: OK, buffer resized to %u bytes",
           m_buffer_size);

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

	switch (m_format) {
	    case SND_PCM_FORMAT_S8:
		m_buffer[m_buffer_used++] = sample >> 16;
		break;
	    case SND_PCM_FORMAT_S16:
		m_buffer[m_buffer_used++] = sample >> 8;
		m_buffer[m_buffer_used++] = sample >> 16;
		break;
	    case SND_PCM_FORMAT_S24_3LE:
		// play in 24 bit format, 3 bytes per sample
		m_buffer[m_buffer_used++] = sample & 0x0FF;
		m_buffer[m_buffer_used++] = sample >> 8;
		m_buffer[m_buffer_used++] = sample >> 16;
		break;
	    case SND_PCM_FORMAT_S24:
	    case SND_PCM_FORMAT_S32:
		// play in 24 or 32 bit format, 4 bytes per sample
		m_buffer[m_buffer_used++] = 0x00;
		m_buffer[m_buffer_used++] = sample & 0x0FF;
		m_buffer[m_buffer_used++] = sample >> 8;
		m_buffer[m_buffer_used++] = sample >> 16;
		break;
	    default:
		return -EINVAL;
	}
    }

    // write buffer to device if full
    if (m_buffer_used >= m_buffer_size) return flush();

    return 0;
}

//***************************************************************************
int PlayBackALSA::flush()
{
    if (!m_buffer_used) return 0; // nothing to do
    Q_ASSERT(m_channels);
    Q_ASSERT(m_bytes_per_sample);
    if (!m_channels || !m_bytes_per_sample) return -EINVAL;

    if (m_handle) {
	unsigned int samples = m_buffer_used / m_bytes_per_sample;
	unsigned int buffer_samples = m_buffer_size / m_bytes_per_sample;
	unsigned int timeout = (m_rate > 0) ?
	    3 * ((1000 * buffer_samples) / (unsigned int)m_rate) : 1000U;
	unsigned char *p = reinterpret_cast<unsigned char *>(m_buffer.data());
	int r;

	// pad the buffer with silence if necessary
	if (!g_sleep_min && (samples < m_chunk_size)) {
	    snd_pcm_format_set_silence(m_format,
	        m_buffer.data() + samples * m_bytes_per_sample,
	        (m_chunk_size - samples) * m_channels);
	    samples = m_chunk_size;
	    qDebug("--- added silence ---");
	}

	while (samples > 0) {
	    // try to write as much as the device accepts
	    r = snd_pcm_writei(m_handle, p, samples);
	    if ((r == -EAGAIN) || ((r >= 0) && (r < (int)samples))) {
		snd_pcm_wait(m_handle, timeout);
	    } else if (r == -EPIPE) {
		// underrun -> start again
		qWarning("PlayBackALSA::flush(), underrun");
		r = snd_pcm_prepare(m_handle);
		if (r < 0) {
		    qWarning("PlayBackALSA::flush(), "\
		             "resume after underrun failed: %s",
		             snd_strerror(r));
		    m_buffer_used = 0;
		    return r;
		}
		qWarning("PlayBackALSA::flush(), after underrun: resuming");
		continue; // try again
	    } else if (r == -ESTRPIPE) {
		qWarning("PlayBackALSA::flush(), suspended. "\
		         "trying to resume...");
		while ((r = snd_pcm_resume(m_handle)) == -EAGAIN)
		    sleep(1); /* wait until suspend flag is released */
		if (r < 0) {
		    qWarning("PlayBackALSA::flush(), resume failed, "\
		             "restarting stream.");
		    if ((r = snd_pcm_prepare(m_handle)) < 0) {
			qWarning("PlayBackALSA::flush(), resume error: %s",
			         snd_strerror(r));
			m_buffer_used = 0;
			return r;
		    }
		}
		qWarning("PlayBackALSA::flush(), after suspend: resuming");
		continue; // try again
	    } else if (r < 0) {
		qWarning("write error: %s", snd_strerror(r));
		m_buffer_used = 0;
		return r;
	    }
	    if (r > 0) {
		// advance in the buffer
		Q_ASSERT(r <= (int)samples);
		p       += r * m_bytes_per_sample;
		samples -= r;
	    }
	}
    }

    m_buffer_used = 0;
    return 0;
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
void PlayBackALSA::scanDevices()
{
    snd_ctl_t *handle;
    int card, err, dev;
//  int idx;
    snd_ctl_card_info_t *info;
    snd_pcm_info_t *pcminfo;
    snd_ctl_card_info_alloca(&info);
    snd_pcm_info_alloca(&pcminfo);

    m_device_list.clear();

    // per default: offer the dmix plugin
    m_device_list.insert(i18n("DMIX plugin"), "plug:dmix");

    card = -1;
    if (snd_card_next(&card) < 0 || card < 0) {
	qWarning("no soundcards found...");
	return;
    }

    qDebug("**** List of PLAYBACK Hardware Devices ****");
    while (card >= 0) {
	QString name;
	name = "hw:%1";
	name = name.arg(card);
	if ((err = snd_ctl_open(&handle, name.data(), 0)) < 0) {
	    qWarning("control open (%i): %s", card, snd_strerror(err));
	    goto next_card;
	}
	if ((err = snd_ctl_card_info(handle, info)) < 0) {
	    qWarning("control hardware info (%i): %s",
	             card, snd_strerror(err));
	    snd_ctl_close(handle);
	    goto next_card;
	}
	dev = -1;
	while (1) {
// 	    unsigned int count;
	    if (snd_ctl_pcm_next_device(handle, &dev)<0)
		qWarning("snd_ctl_pcm_next_device");
	    if (dev < 0)
		break;
	    snd_pcm_info_set_device(pcminfo, dev);
	    snd_pcm_info_set_subdevice(pcminfo, 0);
	    snd_pcm_info_set_stream(pcminfo, SND_PCM_STREAM_PLAYBACK);
	    if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
		if (err != -ENOENT)
		    qWarning("control digital audio info (%i): %s", card,
		             snd_strerror(err));
		continue;
	    }
	    qDebug("card %i: %s [%s], device %i: %s [%s]",
		card,
		snd_ctl_card_info_get_id(info),
		snd_ctl_card_info_get_name(info),
		dev,
		snd_pcm_info_get_id(pcminfo),
		snd_pcm_info_get_name(pcminfo));

	    // add the device to the list
	    QString hw_device;
	    hw_device = "plughw:%1,%2";
	    hw_device = hw_device.arg(card).arg(dev);

	    QString device_name = "%1 - %2";
	    device_name = device_name.arg(
	        snd_ctl_card_info_get_name(info)).arg(
		snd_pcm_info_get_name(pcminfo));
	    m_device_list.insert(device_name, hw_device);

// 	    count = snd_pcm_info_get_subdevices_count(pcminfo);
// 	    fprintf(stderr, "  Subdevices: %i/%i\n", snd_pcm_info_get_subdevices_avail(pcminfo), count);
// 	    for (idx = 0; idx < (int)count; idx++) {
// 		snd_pcm_info_set_subdevice(pcminfo, idx);
// 		if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
// 		    qWarning("control digital audio playback info (%i): %s", card, snd_strerror(err));
// 		} else {
// 		    fprintf(stderr, "  Subdevice #%i: %s\n", idx, snd_pcm_info_get_subdevice_name(pcminfo));
// 		}
// 	    }
	}
	snd_ctl_close(handle);
next_card:
	if (snd_card_next(&card) < 0) {
	    qWarning("snd_card_next");
	    break;
	}
    }

    snd_config_update_free_global();
}

//***************************************************************************
QString PlayBackALSA::alsaDeviceName(const QString &name)
{
    if (m_device_list.isEmpty() || (name.length() &&
        !m_device_list.contains(name))) scanDevices();

    if (!m_device_list.contains(name)) return name;
    return m_device_list[name];
}

//***************************************************************************
QStringList PlayBackALSA::supportedDevices()
{
    QStringList list;

    // re-validate the list if necessary
    alsaDeviceName(m_device_name);

    QMap<QString, QString>::Iterator it;
    for (it = m_device_list.begin(); it != m_device_list.end(); ++it)
	list.append(it.key());

    return list;
}

//***************************************************************************
QString PlayBackALSA::fileFilter()
{
    return "";
}

//***************************************************************************
snd_pcm_t *PlayBackALSA::openDevice(const QString &device)
{
    snd_pcm_t *pcm = m_handle;

//     qDebug("PlayBackALSA::openDevice(%s)", device.local8Bit().data());

    // translate verbose name to internal ALSA name
    QString alsa_device = alsaDeviceName(device);

    if (!alsa_device.length()) return 0;

    // workaround for bug in ALSA
    // if the device name ends with "," -> invalid name
    if (alsa_device.endsWith(",")) return 0;

    if (!pcm) {
	// open the device in case it's not already open
	int err = snd_pcm_open(&pcm, alsa_device.local8Bit().data(),
	                       SND_PCM_STREAM_PLAYBACK,
	                       SND_PCM_NONBLOCK);
	if (err < 0) {
	    pcm = 0;
	    qWarning("PlayBackALSA::openDevice('%s') - "\
	             "failed, err=%d (%s)",
	             alsa_device.local8Bit().data(),
	             err, snd_strerror(err));
	}
    }

    return pcm;
}

//***************************************************************************
QValueList<unsigned int> PlayBackALSA::supportedBits(const QString &device)
{
    QValueList <unsigned int> bits;
//     qDebug("PlayBackALSA::supportedBits(%s)", device.local8Bit().data());

    snd_pcm_hw_params_t *p;

    snd_pcm_hw_params_alloca(&p);
    if (!p) return bits;

    snd_pcm_t *pcm = openDevice(device);
    if (!pcm) return bits;

    if (snd_pcm_hw_params_any(pcm, p) >= 0) {
	// check only "signed int" formats...
	if (!snd_pcm_hw_params_test_format(pcm, p, SND_PCM_FORMAT_S8 ))
	    bits.append( 8);
	if (!snd_pcm_hw_params_test_format(pcm, p, SND_PCM_FORMAT_S16))
	    bits.append(16);
	if (!snd_pcm_hw_params_test_format(pcm, p, SND_PCM_FORMAT_S24) ||
	    !snd_pcm_hw_params_test_format(pcm, p, SND_PCM_FORMAT_S24_3LE))
	    bits.append(24);
	if (!snd_pcm_hw_params_test_format(pcm, p, SND_PCM_FORMAT_S32))
	    bits.append(32);
    }

    // close the device if *we* opened it
    if (pcm != m_handle) snd_pcm_close(pcm);

    return bits;
}

//***************************************************************************
int PlayBackALSA::detectChannels(const QString &device,
                                 unsigned int &min, unsigned int &max)
{
    min = max = 0;
    snd_pcm_hw_params_t *p;

    snd_pcm_hw_params_alloca(&p);
    if (!p) return -1;

    snd_pcm_t *pcm = openDevice(device);
    if (!pcm) return -1;

    if (snd_pcm_hw_params_any(pcm, p) >= 0) {
	int err;

	if ((err = snd_pcm_hw_params_get_channels_min(p, &min)) < 0)
	    qWarning("PlayBackALSA::detectTracks: min: %s",
		     snd_strerror(err));
	if ((err = snd_pcm_hw_params_get_channels_max(p, &max)) < 0)
	    qWarning("PlayBackALSA::detectTracks: max: %s",
		     snd_strerror(err));
    }

    // close the device if *we* opened it
    if (pcm != m_handle) snd_pcm_close(pcm);

//     qDebug("PlayBackALSA::detectTracks, min=%u, max=%u", min, max);
    return 0;
}

#endif /* HAVE_ALSA_SUPPORT */

//***************************************************************************
//***************************************************************************
