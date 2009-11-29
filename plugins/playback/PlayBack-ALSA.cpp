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

#include <QString>
#include <QtGlobal>

#include <klocale.h>

#include "libkwave/CompressionType.h"
#include "libkwave/memcpy.h"
#include "libkwave/SampleFormat.h"

#include "PlayBack-ALSA.h"
#include "SampleEncoderLinear.h"

QMap<QString, QString> PlayBackALSA::m_device_list;

/** gui name of the default device */
#define DEFAULT_DEVICE (i18n("default device") + QString("|sound_note"))

/** gui name of the null device */
#define NULL_DEVICE (i18n("null device") + QString("|sound_note"))

//***************************************************************************

/* define some endian dependend symbols that are missing in ALSA */
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
// big endian
#define _SND_PCM_FORMAT_S18_3 SND_PCM_FORMAT_S18_3BE
#define _SND_PCM_FORMAT_U18_3 SND_PCM_FORMAT_U18_3BE
#define _SND_PCM_FORMAT_U20_3 SND_PCM_FORMAT_U20_3BE
#define _SND_PCM_FORMAT_S20_3 SND_PCM_FORMAT_S20_3BE
#define _SND_PCM_FORMAT_U24_3 SND_PCM_FORMAT_U24_3BE
#define _SND_PCM_FORMAT_S24_3 SND_PCM_FORMAT_S24_3BE

#else
// little endian
#define _SND_PCM_FORMAT_S18_3 SND_PCM_FORMAT_S18_3LE
#define _SND_PCM_FORMAT_U18_3 SND_PCM_FORMAT_U18_3LE
#define _SND_PCM_FORMAT_U20_3 SND_PCM_FORMAT_U20_3LE
#define _SND_PCM_FORMAT_S20_3 SND_PCM_FORMAT_S20_3LE
#define _SND_PCM_FORMAT_U24_3 SND_PCM_FORMAT_U24_3LE
#define _SND_PCM_FORMAT_S24_3 SND_PCM_FORMAT_S24_3LE

#endif

/**
 * Global list of all known sample formats.
 * @note this list should be sorted so that the most preferable formats
 *       come first in the list. When searching for a format that matches
 *       a given set of parameters, the first entry is taken.
 *
 *       The sort order should be:
 *       - compression:      none -> ulaw -> alaw -> adpcm -> mpeg ...
 *       - bits per sample:  ascending
 *       - sample format:    signed -> unsigned -> float -> double ...
 *       - endianness:       cpu -> little -> big
 *       - bytes per sample: ascending
 */
static const snd_pcm_format_t _known_formats[] =
{
    /* 8 bit */
    SND_PCM_FORMAT_S8,
    SND_PCM_FORMAT_U8,

    /* 16 bit */
    SND_PCM_FORMAT_S16, SND_PCM_FORMAT_S16_LE, SND_PCM_FORMAT_S16_BE,
    SND_PCM_FORMAT_U16, SND_PCM_FORMAT_U16_LE, SND_PCM_FORMAT_U16_BE,

    /* 18 bit / 3 byte */
    _SND_PCM_FORMAT_S18_3, SND_PCM_FORMAT_S18_3LE, SND_PCM_FORMAT_S18_3BE,
    _SND_PCM_FORMAT_U18_3, SND_PCM_FORMAT_U18_3LE, SND_PCM_FORMAT_U18_3BE,

    /* 20 bit / 3 byte */
    _SND_PCM_FORMAT_S20_3, SND_PCM_FORMAT_S20_3LE, SND_PCM_FORMAT_S20_3BE,
    _SND_PCM_FORMAT_U20_3, SND_PCM_FORMAT_U20_3LE, SND_PCM_FORMAT_U20_3BE,

    /* 24 bit / 3 byte */
    _SND_PCM_FORMAT_S24_3, SND_PCM_FORMAT_S24_3LE, SND_PCM_FORMAT_S24_3BE,
    _SND_PCM_FORMAT_U24_3, SND_PCM_FORMAT_U24_3LE, SND_PCM_FORMAT_U24_3BE,

    /* 24 bit / 4 byte */
    SND_PCM_FORMAT_S24, SND_PCM_FORMAT_S24_LE, SND_PCM_FORMAT_S24_BE,
    SND_PCM_FORMAT_U24, SND_PCM_FORMAT_U24_LE, SND_PCM_FORMAT_U24_BE,

    /* 32 bit */
    SND_PCM_FORMAT_S32, SND_PCM_FORMAT_S32_LE, SND_PCM_FORMAT_S32_BE,
    SND_PCM_FORMAT_U32, SND_PCM_FORMAT_U32_LE, SND_PCM_FORMAT_U32_BE,

};

//***************************************************************************
/** find out the SampleFormat of an ALSA format */
static SampleFormat sample_format_of(snd_pcm_format_t fmt)
{
    if (snd_pcm_format_float(fmt)) {
	if (snd_pcm_format_width(fmt) == 32)
	    return SampleFormat::Float;
	if (snd_pcm_format_width(fmt) == 64)
	    return SampleFormat::Double;
    } else if (snd_pcm_format_linear(fmt)) {
	if (snd_pcm_format_signed(fmt) == 1)
	    return SampleFormat::Signed;
	else if (snd_pcm_format_unsigned(fmt) == 1)
	    return SampleFormat::Unsigned;
    }

    return SampleFormat::Unknown;
}

//***************************************************************************
/** find out the endianness of an ALSA format */
static byte_order_t endian_of(snd_pcm_format_t fmt)
{
    if (snd_pcm_format_little_endian(fmt) == 1)
	return LittleEndian;
    if (snd_pcm_format_big_endian(fmt) == 1)
	return BigEndian;
    return CpuEndian;
}

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
    m_chunk_size(0),
    m_supported_formats(),
    m_encoder(0)
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
    qDebug("PlayBackALSA::setFormat(..., bits=%u)", bits);
    m_format = SND_PCM_FORMAT_UNKNOWN;
    m_bits = 0;
    m_bytes_per_sample = 0;
    if (m_encoder) delete m_encoder;
    m_encoder = 0;

    // get a format that matches the number of bits
    int format_index = mode2format(bits);
    if (format_index < 0) {
	qWarning("PlayBackALSA::setFormat(): %u bit is not supported", bits);
	return -EINVAL;
    }

    m_format = _known_formats[format_index];
    m_bits = snd_pcm_format_width(m_format);
    m_bytes_per_sample =
	((snd_pcm_format_physical_width(m_format) + 7) >> 3) * m_channels;

    m_encoder = new SampleEncoderLinear(
	sample_format_of(m_format),
	m_bits,
	endian_of(m_format)
    );
    Q_ASSERT(m_encoder);
    if (!m_encoder) {
	qWarning("PlayBackALSA: out of memory");
	return -ENOMEM;
    }

    // activate the settings
    Q_ASSERT(m_bits);
    Q_ASSERT(m_bytes_per_sample);
    Q_ASSERT(m_format != SND_PCM_FORMAT_UNKNOWN);
    int err = snd_pcm_hw_params_set_format(m_handle, hw_params, m_format);
    Q_ASSERT(!err);

    return err;
}

//***************************************************************************
int PlayBackALSA::mode2format(int bits)
{
    // loop over all supported formats and keep only those that are
    // compatible with the given compression, bits and sample format
    foreach (int index, m_supported_formats) {
	const snd_pcm_format_t *fmt = &_known_formats[index];

	if (snd_pcm_format_width(*fmt) != bits) continue;

	// mode is compatible
	// As the list of known formats is already sorted so that
	// the simplest formats come first, we don't have a lot
	// of work -> just take the first entry ;-)
// 	qDebug("PlayBackALSA::mode2format -> %d", index);
	return index;
    }

    qWarning("PlayBackALSA::mode2format -> no match found !?");
    return -1;
}

//***************************************************************************
QList<int> PlayBackALSA::detectSupportedFormats(const QString &device)
{
    // start with an empty list
    QList<int> supported_formats;

    int err;
    snd_pcm_hw_params_t *p;

    snd_pcm_hw_params_alloca(&p);
    if (!p) return supported_formats;

    snd_pcm_t *pcm = openDevice(device);
    if (!pcm) return supported_formats;

    if (!snd_pcm_hw_params_any(pcm, p) < 0) {
	if (pcm != m_handle) snd_pcm_close(pcm);
	return supported_formats;
    }

    // try all known formats
//     qDebug("--- list of supported formats --- ");
    const unsigned int count =
	sizeof(_known_formats) / sizeof(_known_formats[0]);
    for (unsigned int i=0; i < count; i++) {
	// test the sample format
	snd_pcm_format_t format = _known_formats[i];
	err = snd_pcm_hw_params_test_format(pcm, p, format);
	if (err < 0) continue;

	const snd_pcm_format_t *fmt = &(_known_formats[i]);

	// eliminate duplicate alsa sample formats (e.g. BE/LE)
	foreach (int index, m_supported_formats) {
	    const snd_pcm_format_t *f = &_known_formats[index];
	    if (*f == *fmt) {
		fmt = 0;
		break;
	    }
	}
	if (!fmt) continue;

// 	CompressionType t;
// 	SampleFormat::Map sf;
// 	qDebug("#%2u, %2d, %2u bit [%u byte], %s, '%s'",
// 	    i,
// 	    *fmt,
// 	    snd_pcm_format_width(*fmt),
// 	    (snd_pcm_format_physical_width(*fmt)+7) >> 3,
// 	    endian_of(*fmt) == CpuEndian ? "CPU" :
// 	    (endian_of(*fmt) == LittleEndian ? "LE " : "BE "),
// 	    sf.name(sf.findFromData(sample_format_of(
// 		*fmt))).local8Bit().data());

	supported_formats.append(i);
    }
//     qDebug("--------------------------------- ");

    if (pcm != m_handle) snd_pcm_close(pcm);
    return supported_formats;
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
    size_t n = 0;
    snd_pcm_uframes_t start_threshold, stop_threshold;
    const int start_delay = 0;
    const int stop_delay = 0;

    m_chunk_size = 0;
    if (m_handle) snd_pcm_close(m_handle);
    m_handle = 0;

    // translate verbose name to internal ALSA name
    QString alsa_device = alsaDeviceName(device);
    qDebug("PlayBackALSA::openDevice() - opening ALSA device '%s', "\
           "%dHz %d channels, %u bit",
           alsa_device.toLocal8Bit().data(), rate, channels, bits);

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
    err = snd_pcm_open(&m_handle, alsa_device.toLocal8Bit().data(),
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
    if (static_cast<float>(rate) * 1.05 < rrate ||
	static_cast<float>(rate) * 0.95 > rrate)
    {
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

    err = snd_pcm_sw_params_set_avail_min(m_handle, sw_params, m_chunk_size);

    /* round up to closest transfer boundary */
    start_threshold = static_cast<snd_pcm_uframes_t>(
	static_cast<double>(rate) * start_delay / 1000000);
    if (start_delay <= 0) start_threshold += n;

    if (start_threshold < 1) start_threshold = 1;
    if (start_threshold > n) start_threshold = n;
    err = snd_pcm_sw_params_set_start_threshold(m_handle, sw_params,
                                                start_threshold);
    Q_ASSERT(err >= 0);
    stop_threshold = static_cast<snd_pcm_uframes_t>(
	static_cast<double>(rate) * stop_delay / 1000000);
    if (stop_delay <= 0) stop_threshold += buffer_size;

    err = snd_pcm_sw_params_set_stop_threshold(m_handle, sw_params,
                                               stop_threshold);
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
	"bits=%u, bufbase=%u)", device.toLocal8Bit().data(), rate, channels,
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
    if (m_encoder) delete m_encoder;
    m_encoder = 0;

    // initialize the list of supported formats
    m_supported_formats = detectSupportedFormats(device);

    int err = openDevice(device, static_cast<unsigned int>(rate),
	channels, bits);
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
		"on the command line.)",
		m_device_name);
		break;
	    default:
		reason = i18n("Opening the device '%1' failed: %2",
	            device.section('|',0,0),
		    QString::fromLocal8Bit(snd_strerror(err)));
	}
	return reason;
    }

    // resize our buffer and reset it
    Q_ASSERT(m_chunk_size);
    Q_ASSERT(m_bytes_per_sample);
    unsigned int chunk_bytes = m_chunk_size * m_bytes_per_sample;
    Q_ASSERT(chunk_bytes);
    if (!chunk_bytes) return 0;
    unsigned int n = static_cast<unsigned int>(ceil(
	static_cast<float>(1 << m_bufbase) /
	static_cast<float>(chunk_bytes)));
    if (n < 1) n = 1;
    m_buffer_size = n * m_chunk_size * m_bytes_per_sample;
    m_buffer.resize(m_buffer_size);
    m_buffer_size = m_buffer.size();

//     qDebug("PlayBackALSA::open: OK, buffer resized to %u bytes",
//            m_buffer_size);

    return 0;
}

//***************************************************************************
int PlayBackALSA::write(const Kwave::SampleArray &samples)
{
    Q_ASSERT(m_encoder);
    if (!m_encoder) return -EIO;

    unsigned int bytes = m_bytes_per_sample;
    Q_ASSERT (m_buffer_used + bytes <= m_buffer_size);
    if (m_buffer_used + bytes > m_buffer_size) {
	qWarning("PlayBackALSA::write(): buffer overflow ?! (%u/%u)",
	         m_buffer_used, m_buffer_size);
	m_buffer_used = 0;
	return -EIO;
    }

    QByteArray raw(bytes, char(0));
    m_encoder->encode(samples, m_channels, raw);
    MEMCPY(m_buffer.data() + m_buffer_used, raw.data(), bytes);
    m_buffer_used += bytes;

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
	    3 * ((1000 * buffer_samples) /
	    static_cast<unsigned int>(m_rate)) : 1000U;
	u_int8_t *p = reinterpret_cast<u_int8_t *>(m_buffer.data());
	int r;

	// pad the buffer with silence if necessary
	if (samples < m_chunk_size) {
	    snd_pcm_format_set_silence(m_format,
	        m_buffer.data() + samples * m_bytes_per_sample,
	        (m_chunk_size - samples) * m_channels);
	    samples = m_chunk_size;
	    qDebug("--- added silence ---");
	}

	while (samples > 0) {
	    // try to write as much as the device accepts
	    r = snd_pcm_writei(m_handle, p, samples);
	    if ((r == -EAGAIN) || ((r >= 0) &&
	        (r < static_cast<int>(samples))))
	    {
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
		Q_ASSERT(r <= static_cast<int>(samples));
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

    // get rid of the old sample encoder
    if (m_encoder) delete m_encoder;
    m_encoder = 0;

    // clear the list of supported formats, nothing open -> nothing supported
    m_supported_formats.clear();

    return 0;
}

//***************************************************************************
void PlayBackALSA::scanDevices()
{
    snd_ctl_t *handle;
    int card, err, dev;
    int idx;
    snd_ctl_card_info_t *info;
    snd_pcm_info_t *pcminfo;
    snd_ctl_card_info_alloca(&info);
    snd_pcm_info_alloca(&pcminfo);

    m_device_list.clear();

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
	if ((err = snd_ctl_open(&handle, name.toLocal8Bit().data(), 0)) < 0) {
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
	    unsigned int count;
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
	    count = snd_pcm_info_get_subdevices_count(pcminfo);

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

	    QString card_name   = snd_ctl_card_info_get_name(info);
	    QString device_name = snd_pcm_info_get_name(pcminfo);

 	    qDebug("  Subdevices: %i/%i\n",
		snd_pcm_info_get_subdevices_avail(pcminfo), count);
	    if (count > 1) {
		for (idx = 0; idx < static_cast<int>(count); idx++) {
		    snd_pcm_info_set_subdevice(pcminfo, idx);
		    if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
			qWarning("ctrl digital audio playback info (%i): %s",
			         card, snd_strerror(err));
		    } else {
			QString hwdev = hw_device + QString(",%1").arg(idx);
			QString subdevice_name =
			    snd_pcm_info_get_subdevice_name(pcminfo);
			QString name = QString(
			    i18n("card %1: ", card) + card_name +
			    "|sound_card||" +
			    i18n("device %1: ", dev) + device_name +
			    "|sound_device||" +
			    i18n("subdevice %1: ", idx) + subdevice_name +
			    "|sound_subdevice"
			)/*.arg(card).arg(dev).arg(idx)*/;
			qDebug("# '%s' -> '%s'",
			    hwdev.toLocal8Bit().data(),
			    name.toLocal8Bit().data());
			m_device_list.insert(name, hwdev);
		    }
		}
	    } else {
		// no sub-devices
		QString name = QString(
		    i18n("card %1: ", card) +
		         card_name + "|sound_card||" +
		    i18n("device %1: ", dev) +
		          device_name + "|sound_subdevice"
		);
		qDebug("# '%s' -> '%s'", hw_device.toLocal8Bit().data(),
		    name.toLocal8Bit().data());
		m_device_list.insert(name, hw_device);
	    }
	}
	snd_ctl_close(handle);
next_card:
	if (snd_card_next(&card) < 0) {
	    qWarning("snd_card_next failed");
	    break;
	}
    }

    // per default: offer the dmix plugin and the default device
    // if slave devices exist
    if (!m_device_list.isEmpty()) {
        m_device_list.insert(i18n("DMIX plugin")+QString("|sound_note"),
                             "plug:dmix");
        m_device_list.insert(DEFAULT_DEVICE, "default");
    } else {
        m_device_list.insert(NULL_DEVICE, "null");
    }

}

//***************************************************************************
QString PlayBackALSA::alsaDeviceName(const QString &name)
{
    if (m_device_list.isEmpty() || (name.length() &&
        !m_device_list.contains(name)))
    {
// 	qDebug("### RESCAN ### (list.count=%d, name='%s')",
// 	       m_device_list.count(), name.data());
	scanDevices();
    }

    if (!m_device_list.contains(name)) {
	// maybe we already have a ALSA compatible name (like in init state)
	foreach (QString n, m_device_list.values())
	    if (n == name) return n;

	qWarning("PlayBackALSA::alsaDeviceName('%s') - NOT FOUND",
	    name.toLocal8Bit().data());
	return "";
    }
    return m_device_list[name];
}

//***************************************************************************
QStringList PlayBackALSA::supportedDevices()
{
    // re-validate the list if necessary
    scanDevices();

    QStringList list = m_device_list.keys();

    // move "default" or "null" to the start of the list
    if (list.contains(NULL_DEVICE))
	list.move(list.indexOf(NULL_DEVICE), 0);
    if (list.contains(DEFAULT_DEVICE))
	list.move(list.indexOf(DEFAULT_DEVICE), 0);

    if (!list.isEmpty()) list.append("#TREE#");

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
	int err = snd_pcm_open(&pcm, alsa_device.toLocal8Bit().data(),
	                       SND_PCM_STREAM_PLAYBACK,
	                       SND_PCM_NONBLOCK);
	if (err < 0) {
	    pcm = 0;
	    qWarning("PlayBackALSA::openDevice('%s') - "\
	             "failed, err=%d (%s)",
	             alsa_device.toLocal8Bit().data(),
	             err, snd_strerror(err));
	}
    }

    return pcm;
}

//***************************************************************************
QList<unsigned int> PlayBackALSA::supportedBits(const QString &device)
{
    QList<unsigned int> list;
    QList<int> supported_formats;

    // try all known sample formats
    supported_formats = detectSupportedFormats(device);
    foreach (int index, supported_formats) {
	const snd_pcm_format_t *fmt = &(_known_formats[index]);
	const unsigned int bits = snd_pcm_format_width(*fmt);

	// 0  bits means invalid/does not apply
	if (!bits) continue;

	// do not produce duplicates
	if (list.contains(bits)) continue;

// 	qDebug("found bits/sample %u", bits);
	list.append(bits);
    }

    return list;
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

    return 0;
}

#endif /* HAVE_ALSA_SUPPORT */

//***************************************************************************
//***************************************************************************
