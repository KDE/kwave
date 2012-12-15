/*************************************************************************
        Record-ALSA.cpp  -  device for audio recording via ALSA
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

#include "config.h"
#ifdef HAVE_ALSA_SUPPORT

#include <errno.h>
#include <math.h>

#include <QtCore/QtGlobal>

#include "libkwave/CompressionType.h"
#include "libkwave/String.h"

#include "Record-ALSA.h"

/** initializer for the list of devices */
QMap<QString, QString> Kwave::RecordALSA::m_device_list;

/** gui name of the default device */
#define DEFAULT_DEVICE (i18n("DSNOOP plugin") + _("|sound_note"))

/** helper macro: returns the number of elements in an array */
#define ELEMENTS_OF(__array__) (sizeof(__array__) / sizeof(__array__[0]))

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

    /* float, 32 bit */
    SND_PCM_FORMAT_FLOAT, SND_PCM_FORMAT_FLOAT_LE, SND_PCM_FORMAT_FLOAT_BE,

    /* float, 64 bit */
    SND_PCM_FORMAT_FLOAT64,
    SND_PCM_FORMAT_FLOAT64_LE, SND_PCM_FORMAT_FLOAT64_BE,

#if 0
    /* IEC958 subframes (not supported) */
    SND_PCM_FORMAT_IEC958_SUBFRAME,
    SND_PCM_FORMAT_IEC958_SUBFRAME_LE, SND_PCM_FORMAT_IEC958_SUBFRAME_BE,
#endif

    /* G711 ULAW */
    SND_PCM_FORMAT_MU_LAW,

    /* G711 ALAW */
    SND_PCM_FORMAT_A_LAW,

    /*
     * some exotic formats, does anyone really use them
     * for recording ?
     * -> omit support for them, this makes life easier ;-)
     */
#if 0
    /* IMA ADPCM, 3 or 4 bytes per sample (not supported) */
    SND_PCM_FORMAT_IMA_ADPCM,

    /* MPEG (not supported) */
    SND_PCM_FORMAT_MPEG,

    /* GSM */
    SND_PCM_FORMAT_GSM,

    /* special (not supported) */
    SND_PCM_FORMAT_SPECIAL,
#endif
};

/** sleep seconds, used for recording */
static const unsigned int g_sleep_min = 0;

//***************************************************************************
/** find out the SampleFormat of an ALSA format */
static Kwave::SampleFormat sample_format_of(snd_pcm_format_t fmt)
{
    if (snd_pcm_format_float(fmt)) {
	if (snd_pcm_format_width(fmt) == 32)
	    return Kwave::SampleFormat::Float;
	if (snd_pcm_format_width(fmt) == 64)
	    return Kwave::SampleFormat::Double;
    } else if (snd_pcm_format_linear(fmt)) {
	if (snd_pcm_format_signed(fmt) == 1)
	    return Kwave::SampleFormat::Signed;
	else if (snd_pcm_format_unsigned(fmt) == 1)
	    return Kwave::SampleFormat::Unsigned;
    }

    return Kwave::SampleFormat::Unknown;
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
static int compression_of(snd_pcm_format_t fmt)
{
    switch (fmt) {
	case SND_PCM_FORMAT_MU_LAW:
	    return AF_COMPRESSION_G711_ULAW;
	case SND_PCM_FORMAT_A_LAW:
	    return AF_COMPRESSION_G711_ALAW;
	case SND_PCM_FORMAT_IMA_ADPCM:
	    return AF_COMPRESSION_MS_ADPCM;
	case SND_PCM_FORMAT_MPEG:
	    return Kwave::CompressionType::MPEG_LAYER_I;
	case SND_PCM_FORMAT_GSM:
	    return AF_COMPRESSION_GSM;
	default:
	    return AF_COMPRESSION_NONE;
    }
    return AF_COMPRESSION_NONE;
}

//***************************************************************************
Kwave::RecordALSA::RecordALSA()
    :Kwave::RecordDevice(), m_handle(0), m_hw_params(0),
     m_sw_params(0), m_open_result(0), m_tracks(0),
     m_rate(0.0), m_compression(0), m_bits_per_sample(0),
     m_bytes_per_sample(0), m_sample_format(Kwave::SampleFormat::Unknown),
     m_supported_formats(), m_initialized(false), m_buffer_size(0),
     m_chunk_size(0)
{
    snd_pcm_hw_params_malloc(&m_hw_params);
    snd_pcm_sw_params_malloc(&m_sw_params);
    Q_ASSERT(m_hw_params);
    Q_ASSERT(m_sw_params);
}

//***************************************************************************
Kwave::RecordALSA::~RecordALSA()
{
    close();
    snd_pcm_hw_params_free(m_hw_params);
    snd_pcm_sw_params_free(m_sw_params);
}

//***************************************************************************
void Kwave::RecordALSA::detectSupportedFormats()
{
    // start with an empty list
    m_supported_formats.clear();

    Q_ASSERT(m_handle);
    if (!m_handle || !m_hw_params) return;

    int err;
    if (!snd_pcm_hw_params_any(m_handle, m_hw_params) < 0) return;

    // try all known formats
//     qDebug("--- list of supported formats --- ");
    const unsigned int count =
	sizeof(_known_formats) / sizeof(_known_formats[0]);
    for (unsigned int i=0; i < count; i++) {
	// test the sample format
	snd_pcm_format_t format = _known_formats[i];
	err = snd_pcm_hw_params_test_format(m_handle, m_hw_params, format);
	if (err < 0) continue;

	const snd_pcm_format_t *fmt = &(_known_formats[i]);

	// eliminate duplicate alsa sample formats (e.g. BE/LE)
	foreach (int it, m_supported_formats) {
	    const snd_pcm_format_t *f = &_known_formats[it];
	    if (*f == *fmt) {
		fmt = 0;
		break;
	    }
	}
	if (!fmt) continue;

// 	Kwave::CompressionType t;
// 	Kwave::SampleFormat::Map sf;
// 	qDebug("#%2u, %2d, %2u bit [%u byte], %s, '%s', '%s'",
// 	    i,
// 	    *fmt,
// 	    snd_pcm_format_width(*fmt),
// 	    (snd_pcm_format_physical_width(*fmt)+7) >> 3,
// 	    endian_of(*fmt) == CpuEndian ? "CPU" :
// 	    (endian_of(*fmt) == LittleEndian ? "LE " : "BE "),
// 	    sf.description(sf.findFromData(sample_format_of(
// 		*fmt), true)).toLocal8Bit().data(),
// 	    t.description(t.findFromData(compression_of(
// 		*fmt), true)).toLocal8Bit().data());

	m_supported_formats.append(i);
    }
//     qDebug("--------------------------------- ");

}

//***************************************************************************
int Kwave::RecordALSA::open(const QString &device)
{
//     qDebug("RecordALSA::open(%s)", device.toLocal8Bit().data());

    // close the previous device
    if (m_handle) close();
    m_initialized = false;

    if (!device.length()) return -1; // no device name

    // translate verbose name to internal ALSA name
    QString alsa_device = alsaDeviceName(device);
    qDebug("RecordALSA::open -> '%s'", alsa_device.toLocal8Bit().data());

    if (!alsa_device.length()) return -ENOENT;

    // workaround for bug in ALSA
    // if the device name ends with "," -> invalid name
    if (alsa_device.endsWith(_(","))) return -ENOENT;

    // open the device in case it's not already open
    m_open_result = snd_pcm_open(&m_handle, alsa_device.toLocal8Bit().data(),
                                 SND_PCM_STREAM_CAPTURE,
                                 SND_PCM_NONBLOCK);
    if (m_open_result < 0) {
	m_handle = 0;
	qWarning("RecordALSA::openDevice('%s') - failed, err=%d (%s)",
	         alsa_device.toLocal8Bit().data(),
	         m_open_result, snd_strerror(m_open_result));
	return m_open_result;
    }

    // now we can detect all supported formats
    detectSupportedFormats();

    return 0;
}

//***************************************************************************
int Kwave::RecordALSA::initialize()
{
    int err;
    snd_output_t *output = NULL;

    snd_pcm_uframes_t buffer_size;
    unsigned period_time = 0; // period time in us
    unsigned buffer_time = 0; // ring buffer length in us
    snd_pcm_uframes_t period_frames = 0;
    snd_pcm_uframes_t buffer_frames = 0;
    size_t n;
    snd_pcm_uframes_t start_threshold, stop_threshold;
    const int start_delay = 0;
    const int stop_delay = 0;

//     qDebug("RecordALSA::initialize");
    Q_ASSERT(!m_initialized);

    m_buffer_size = 0;

    Q_ASSERT(m_handle);
    if (!m_handle || !m_hw_params) return -EBADF; // file not opened

    // close the device if it was previously open
    snd_pcm_drop(m_handle);

    err = snd_output_stdio_attach(&output, stderr, 0);
    if (err < 0) {
	qWarning("Output failed: %s", snd_strerror(err));
    }

    if ((err = snd_pcm_hw_params_any(m_handle, m_hw_params)) < 0) {
	qWarning("Cannot initialize hardware parameters: %s",
	         snd_strerror(err));
	snd_output_close(output);
	return -EIO;
    }

    err = snd_pcm_hw_params_set_access(m_handle, m_hw_params,
         SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0) {
	qWarning("Cannot set access type: %s", snd_strerror(err));
	snd_output_close(output);
	return -EIO;
    }

    int format_index = mode2format(m_compression, m_bits_per_sample,
                                   m_sample_format);
    Q_ASSERT(format_index >= 0);
    if (format_index < 0) {
	Kwave::CompressionType t;
	Kwave::SampleFormat::Map sf;

	qWarning("RecordkALSA::setFormat(): no matching format for "\
	         "compression '%s', %d bits/sample, format '%s'",
	         sf.description(sf.findFromData(m_sample_format),
	                                        true).toLocal8Bit().data(),
	         m_bits_per_sample,
	         QString(t.name(
		     t.findFromData(m_compression))).toLocal8Bit().data());

	snd_output_close(output);
	return -EINVAL;
    }

    Q_ASSERT(format_index >= 0);
    snd_pcm_format_t alsa_format = _known_formats[format_index];
    m_bytes_per_sample = ((snd_pcm_format_physical_width(
	_known_formats[format_index])+7) >> 3) * m_tracks;

    err = snd_pcm_hw_params_test_format(m_handle, m_hw_params, alsa_format);
    if (err) {
	qWarning("RecordkALSA::setFormat(): format %u is not supported",
	    static_cast<int>(alsa_format));
	snd_output_close(output);
	return -EINVAL;
    }

    // activate the settings
    err = snd_pcm_hw_params_set_format(m_handle, m_hw_params, alsa_format);
    if (err < 0) {
	qWarning("Cannot set sample format: %s", snd_strerror(err));
	snd_output_close(output);
	return -EINVAL;
    }

    err = snd_pcm_hw_params_set_channels(m_handle, m_hw_params, m_tracks);
    if (err < 0) {
	qWarning("Cannot set channel count: %s", snd_strerror(err));
	snd_output_close(output);
	return -EINVAL;
    }

    unsigned int rrate = m_rate > 0 ?
	static_cast<unsigned int>(rint(m_rate)) : 0;
    err = snd_pcm_hw_params_set_rate_near(m_handle, m_hw_params, &rrate, 0);
    if (err < 0) {
	qWarning("Cannot set sample rate: %s", snd_strerror(err));
	snd_output_close(output);
	return -EINVAL;
    }
//     qDebug("   real rate = %u", rrate);
    if (m_rate * 1.05 < rrate || m_rate * 0.95 > rrate) {
	qWarning("rate is not accurate (requested = %iHz, got = %iHz)",
	         static_cast<int>(m_rate), static_cast<int>(rrate));
    }
    m_rate = rrate;

    if ((buffer_time) == 0 && (buffer_frames) == 0) {
	err = snd_pcm_hw_params_get_buffer_time_max(m_hw_params, &buffer_time, 0);
	if (buffer_time > 500000) buffer_time = 500000;
    }

    if ((period_time == 0) && (period_frames == 0)) {
	if (buffer_time > 0)
	    period_time = buffer_time / 4;
	else
	    period_frames = buffer_frames / 4;
    }

    if (period_time > 0) {
	err = snd_pcm_hw_params_set_period_time_near(m_handle, m_hw_params,
	                                             &period_time, 0);
    } else {
	err = snd_pcm_hw_params_set_period_size_near(m_handle, m_hw_params,
	                                             &period_frames, 0);
    }
    Q_ASSERT(err >= 0);
    if (buffer_time > 0) {
	err = snd_pcm_hw_params_set_buffer_time_near(m_handle, m_hw_params,
	                                             &buffer_time, 0);
    } else {
	err = snd_pcm_hw_params_set_buffer_size_near(m_handle, m_hw_params,
	                                             &buffer_frames);
    }
    Q_ASSERT(err >= 0);

//     qDebug("   setting hw_params");
    err = snd_pcm_hw_params(m_handle, m_hw_params);
    if (err < 0) {
	snd_pcm_dump(m_handle, output);
	snd_output_close(output);
	qWarning("Cannot set parameters: %s", snd_strerror(err));
	return err;
    }

    snd_pcm_hw_params_get_period_size(m_hw_params, &m_chunk_size, 0);
    snd_pcm_hw_params_get_buffer_size(m_hw_params, &buffer_size);
    if (m_chunk_size == buffer_size) {
	qWarning("Can't use period equal to buffer size (%lu == %lu)",
	         m_chunk_size, buffer_size);
	snd_output_close(output);
	return -EIO;
    }

    /* set software parameters */
    err = snd_pcm_sw_params_current(m_handle, m_sw_params);
    if (err < 0) {
	qWarning("Unable to determine current software parameters: %s",
	         snd_strerror(err));
	snd_output_close(output);
	return err;
    }

    err = snd_pcm_sw_params_set_avail_min(m_handle, m_sw_params, m_chunk_size);

    /* round up to closest transfer boundary */
    n = buffer_size;
    start_threshold = static_cast<snd_pcm_uframes_t>
                      (m_rate * start_delay / 1000000);
    if (start_delay <= 0) start_threshold += n;

    if (start_threshold < 1) start_threshold = 1;
    if (start_threshold > n) start_threshold = n;
    err = snd_pcm_sw_params_set_start_threshold(m_handle, m_sw_params,
                                                start_threshold);
    Q_ASSERT(err >= 0);
    stop_threshold = static_cast<snd_pcm_uframes_t>
                     (m_rate * stop_delay / 1000000);
    if (stop_delay <= 0) stop_threshold += buffer_size;

    err = snd_pcm_sw_params_set_stop_threshold(m_handle, m_sw_params,
                                               stop_threshold);
    Q_ASSERT(err >= 0);

    // write the software parameters to the recording device
    err = snd_pcm_sw_params(m_handle, m_sw_params);
    if (err < 0) {
	qDebug("   activating snd_pcm_sw_params FAILED");
	snd_pcm_dump(m_handle, output);
	qWarning("Unable to set software parameters: %s", snd_strerror(err));
    }

    // prepare the device for recording
    if ((err = snd_pcm_prepare(m_handle)) < 0) {
	snd_pcm_dump(m_handle, output);
	qWarning("cannot prepare interface for use: %s",snd_strerror(err));
    }

    if ((err = snd_pcm_start(m_handle)) < 0) {
	snd_pcm_dump(m_handle, output);
	qWarning("cannot start interface: %s",snd_strerror(err));
    }

    // resize our buffer and reset it
    Q_ASSERT(m_chunk_size);
    Q_ASSERT(m_bytes_per_sample);

//     snd_pcm_dump(m_handle, output);
    snd_output_close(output);

    return 0;
}

//***************************************************************************
int Kwave::RecordALSA::read(QByteArray &buffer, unsigned int offset)
{
    unsigned int length = buffer.size();

    if (!m_handle) return m_open_result; // file not opened / open has failed
    if (!length)   return 0;             // no buffer, nothing to do

    // we configure our device at a late stage, not on the fly like in OSS
    if (!m_initialized) {
	int err = initialize();
	if (err < 0) return err;
	m_initialized = true;
    }

    Q_ASSERT(m_chunk_size);
    if (!m_chunk_size) return 0;

    unsigned int chunk_bytes = m_chunk_size * m_bytes_per_sample;
    Q_ASSERT(chunk_bytes);
    if (!chunk_bytes) return 0;

    // align the buffer size to the chunk size if necessary
    unsigned int n = (length / chunk_bytes);
    if (length != (n * chunk_bytes)) {
	n++;
	length = n * chunk_bytes;
// 	qDebug("resizing buffer %p from %u to %u bytes",
// 	       buffer.data(), buffer.size(), length);
	buffer.resize(length);
    }

    Q_ASSERT(length >= offset);
    Q_ASSERT(m_rate > 0);
    unsigned int samples = (length - offset) / m_bytes_per_sample;

    // do not read more than one chunk at a time
    if (samples > m_chunk_size) samples = m_chunk_size;

#if 0
    // just for debugging: detect state changes of the device
    static snd_pcm_state_t last_state = SND_PCM_STATE_DISCONNECTED;
    snd_pcm_state_t state = snd_pcm_state(m_handle);
    if (state != last_state) {
	switch (state) {
	    case SND_PCM_STATE_OPEN:
		qDebug("SND_PCM_STATE_OPEN");
		break;
	    case SND_PCM_STATE_SETUP:
		qDebug("SND_PCM_STATE_SETUP");
		break;
	    case SND_PCM_STATE_PREPARED:
		qDebug("ND_PCM_STATE_PREPARED");
		break;
	    case SND_PCM_STATE_RUNNING:
		qDebug("SND_PCM_STATE_RUNNING");
		break;
	    case SND_PCM_STATE_XRUN:
		qDebug("SND_PCM_STATE_XRUN");
		break;
	    case SND_PCM_STATE_DRAINING:
		qDebug("SND_PCM_STATE_DRAINING");
		break;
	    case SND_PCM_STATE_PAUSED:
		qDebug("SND_PCM_STATE_PAUSED");
		break;
	    case SND_PCM_STATE_SUSPENDED:
		qDebug("SND_PCM_STATE_SUSPENDED");
		break;
	    case SND_PCM_STATE_DISCONNECTED:
		qDebug("SND_PCM_STATE_DISCONNECTED");
		break;
	}
	last_state = state;
    }
#endif

    // try to read as much as the device accepts
    Q_ASSERT(samples);
    Q_ASSERT(offset + samples <= static_cast<unsigned int>(buffer.size()));
    int r = snd_pcm_readi(m_handle, buffer.data() + offset, samples);

    // handle all negative result codes
    if (r == -EAGAIN) {
	unsigned int timeout = (m_rate > 0) ?
	    (((1000 * samples) / 4) / static_cast<unsigned int>(m_rate)) : 10U;
	snd_pcm_wait(m_handle, timeout);
	return -EAGAIN;
    } else if (r == -EPIPE) {
	// underrun -> start again
	qWarning("RecordALSA::read(), underrun");
	r = snd_pcm_prepare(m_handle);
	r = snd_pcm_start(m_handle);
	if (r < 0) {
	    qWarning("RecordALSA::read(), "\
		     "resume after underrun failed: %s",
		     snd_strerror(r));
	    return r;
	}
	qWarning("RecordALSA::read(), after underrun: resuming");
	return -EAGAIN; // try again
    } else if (r == -ESTRPIPE) {
	qWarning("RecordALSA::read(), suspended. "\
		    "trying to resume...");
	while ((r = snd_pcm_resume(m_handle)) == -EAGAIN)
	    return -EAGAIN; /* wait until suspend flag is released */
	if (r < 0) {
	    qWarning("RecordALSA::read(), resume failed, "\
		    "restarting stream.");
	    if ((r = snd_pcm_prepare(m_handle)) < 0) {
		qWarning("RecordALSA::read(), resume error: %s",
			    snd_strerror(r));
		return r;
	    }
	}
	qWarning("RecordALSA::read(), after suspend: resuming");
	return -EAGAIN; // try again
    } else if (r < 0) {
	qWarning("RecordALSA: read error: %s", snd_strerror(r));
	return r;
    }

    // no error, successfully read something:
    // advance in the buffer
//     qDebug("<<< after read, r=%d", r);
    Q_ASSERT(r <= static_cast<int>(samples));
    if (r > static_cast<int>(samples)) r = samples;

    return (r * m_bytes_per_sample);
}

//***************************************************************************
int Kwave::RecordALSA::close()
{
    // close the device handle

    if (m_handle) {
	snd_pcm_drop(m_handle);
	snd_pcm_hw_free(m_handle);
	snd_pcm_close(m_handle);
    }
    m_handle = 0;
    m_open_result = -EINVAL;

    // we need to re-initialize the next time
    m_initialized = false;

    // clear the list of supported formats, nothing open -> nothing supported
    m_supported_formats.clear();

    return 0;
}

//***************************************************************************
int Kwave::RecordALSA::detectTracks(unsigned int &min, unsigned int &max)
{
    min = max = 0;

    if (!m_handle || !m_hw_params) return -1;

    if (snd_pcm_hw_params_any(m_handle, m_hw_params) >= 0) {
	int err;
	if ((err = snd_pcm_hw_params_get_channels_min(m_hw_params, &min)) < 0)
	    qWarning("RecordALSA::detectTracks: min: %s",
		     snd_strerror(err));
	if ((err = snd_pcm_hw_params_get_channels_max(m_hw_params, &max)) < 0)
	    qWarning("RecordALSA::detectTracks: max: %s",
		     snd_strerror(err));
    }

//     qDebug("RecordALSA::detectTracks, min=%u, max=%u", min, max);
    return 0;
}

//***************************************************************************
int Kwave::RecordALSA::setTracks(unsigned int &tracks)
{
    if (tracks != m_tracks) m_initialized = false;
    m_tracks = tracks;
    return 0;
}

//***************************************************************************
int Kwave::RecordALSA::tracks()
{
    return m_tracks;
}

//***************************************************************************
QList<double> Kwave::RecordALSA::detectSampleRates()
{
    QList<double> list;
    int err;

    if (!m_handle || !m_hw_params) return list;

    if (!snd_pcm_hw_params_any(m_handle, m_hw_params) < 0) return list;

    static const unsigned int known_rates[] = {
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
    for (unsigned int i = 0; i < ELEMENTS_OF(known_rates); i++) {
	unsigned int rate = known_rates[i];

	err = snd_pcm_hw_params_test_rate(m_handle, m_hw_params, rate, 0);
	if (err < 0) continue;

	// do not produce duplicates
	if (list.contains(rate)) continue;

// 	qDebug("found rate %u Hz", rate);
	list.append(rate);
    }

    return list;
}

//***************************************************************************
int Kwave::RecordALSA::setSampleRate(double &new_rate)
{
    if (new_rate != m_rate) m_initialized = false;
    m_rate = new_rate;
    return 0;
}

//***************************************************************************
double Kwave::RecordALSA::sampleRate()
{
    return m_rate;
}

//***************************************************************************
int Kwave::RecordALSA::mode2format(int compression, int bits,
                                   Kwave::SampleFormat sample_format)
{
    // loop over all supported formats and keep only those that are
    // compatible with the given compression, bits and sample format
    foreach (int index, m_supported_formats)
    {
	const snd_pcm_format_t *fmt = &_known_formats[index];

	if (compression_of(*fmt) != compression) continue;
	if (snd_pcm_format_width(*fmt) != bits) continue;
	if (!(sample_format_of(*fmt) == sample_format)) continue;

	// mode is compatible
	// As the list of known formats is already sorted so that
	// the simplest formats come first, we don't have a lot
	// of work -> just take the first entry ;-)
// 	qDebug("RecordALSA::mode2format -> %d", index);
	return index;
    }

    qWarning("RecordALSA::mode2format -> no match found !?");
    return -1;
}

//***************************************************************************
QList<int> Kwave::RecordALSA::detectCompressions()
{
    QList<int> list;

    // try all known sample formats
    foreach(int it, m_supported_formats)
    {
	const snd_pcm_format_t *fmt = &(_known_formats[it]);
	int compression = compression_of(*fmt);

	// do not produce duplicates
	if (list.contains(compression)) continue;

// 	Kwave::CompressionType t;
// 	qDebug("found compression %d '%s'", compression,
// 	       t.name(t.findFromData(compression)).toLocal8Bit().data());
	list.append(compression);
    }

    return list;
}

//***************************************************************************
int Kwave::RecordALSA::setCompression(int new_compression)
{
    if (m_compression != new_compression) m_initialized = false;
    m_compression = new_compression;
    return m_compression;
}

//***************************************************************************
int Kwave::RecordALSA::compression()
{
    return m_compression;
}

//***************************************************************************
QList<unsigned int> Kwave::RecordALSA::supportedBits()
{
    QList<unsigned int> list;

    // try all known sample formats
    foreach(int it, m_supported_formats)
    {
	const snd_pcm_format_t *fmt = &(_known_formats[it]);
	const unsigned int bits = snd_pcm_format_width(*fmt);

	// 0  bits means invalid/does not apply
	if (!bits) continue;

	// only accept bits/sample if compression matches
	if (compression_of(*fmt) != m_compression) continue;

	// do not produce duplicates
	if (list.contains(bits)) continue;

// 	qDebug("found bits/sample %u", bits);
	list.append(bits);
    }

    return list;
}

//***************************************************************************
int Kwave::RecordALSA::setBitsPerSample(unsigned int new_bits)
{
    if (m_bits_per_sample != new_bits) m_initialized = false;
    m_bits_per_sample = new_bits;
    return 0;
}

//***************************************************************************
int Kwave::RecordALSA::bitsPerSample()
{
    return m_bits_per_sample;
}

//***************************************************************************
QList<Kwave::SampleFormat> Kwave::RecordALSA::detectSampleFormats()
{
    QList<Kwave::SampleFormat> list;

    // try all known sample formats
    foreach(int it, m_supported_formats)
    {
	const snd_pcm_format_t *fmt = &(_known_formats[it]);
	const Kwave::SampleFormat sample_format = sample_format_of(*fmt);

	// only accept bits/sample if compression types
	// and bits per sample match
	if (compression_of(*fmt) != m_compression) continue;
	if (snd_pcm_format_width(*fmt) != static_cast<int>(m_bits_per_sample))
	    continue;

	// do not produce duplicates
	if (list.contains(sample_format)) continue;

// 	Kwave::SampleFormat::Map sf;
// 	qDebug("found sample format %u ('%s')", (int)sample_format,
// 		sf.name(sf.findFromData(sample_format)).toLocal8Bit().data());

	list.append(sample_format);
    }

    return list;
}

//***************************************************************************
int Kwave::RecordALSA::setSampleFormat(Kwave::SampleFormat new_format)
{
    if (m_sample_format != new_format) m_initialized = false;
    m_sample_format = new_format;
    return 0;
}

//***************************************************************************
Kwave::SampleFormat Kwave::RecordALSA::sampleFormat()
{
    return m_sample_format;
}

//***************************************************************************
byte_order_t Kwave::RecordALSA::endianness()
{
    int index = mode2format(m_compression, m_bits_per_sample, m_sample_format);
    return (index >= 0) ? endian_of(_known_formats[index]) : UnknownEndian;
}

//***************************************************************************
QStringList Kwave::RecordALSA::supportedDevices()
{
    // re-validate the list if necessary
    scanDevices();

    QStringList list = m_device_list.keys();

    // move the default device to the start of the list
    if (list.contains(DEFAULT_DEVICE))
	list.move(list.indexOf(DEFAULT_DEVICE), 0);

    list.append(_("#TREE#"));
    return list;
}

//***************************************************************************
void Kwave::RecordALSA::scanDevices()
{
    snd_ctl_t *handle = 0;
    int card, err, dev;
    int idx;
    snd_ctl_card_info_t *info    = 0;
    snd_pcm_info_t      *pcminfo = 0;

    m_device_list.clear();

    card = -1;
    if (snd_card_next(&card) < 0 || card < 0) {
	qWarning("no soundcards found...");
	return;
    }

    snd_ctl_card_info_malloc(&info);
    snd_pcm_info_malloc(&pcminfo);

//     qDebug("**** List of RECORD Hardware Devices ****");
    while (card >= 0) {
	QString name;
	name = _("hw:%1");
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
	    snd_pcm_info_set_stream(pcminfo, SND_PCM_STREAM_CAPTURE);
	    if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
		if (err != -ENOENT)
		    qWarning("control digital audio info (%i): %s", card,
		             snd_strerror(err));
		continue;
	    }
	    count = snd_pcm_info_get_subdevices_count(pcminfo);

// 	    qDebug("card %i: %s [%s], device %i: %s [%s]",
// 		card,
// 		snd_ctl_card_info_get_id(info),
// 		snd_ctl_card_info_get_name(info),
// 		dev,
// 		snd_pcm_info_get_id(pcminfo),
// 		snd_pcm_info_get_name(pcminfo));

	    // add the device to the list
	    QString hw_device;
	    hw_device = _("hw:%1,%2");
	    hw_device = hw_device.arg(card).arg(dev);

	    QString card_name   = _(snd_ctl_card_info_get_name(info));
	    QString device_name = _(snd_pcm_info_get_name(pcminfo));

//  	    qDebug("  Subdevices: %i/%i\n",
// 		snd_pcm_info_get_subdevices_avail(pcminfo), count);
	    if (count > 1) {
		for (idx = 0; idx < static_cast<int>(count); idx++) {
		    snd_pcm_info_set_subdevice(pcminfo, idx);
		    if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
			qWarning("ctrl digital audio playback info (%i): %s",
			         card, snd_strerror(err));
		    } else {
			QString hwdev = hw_device + _(",%1").arg(idx);
			QString subdevice_name = _(
			    snd_pcm_info_get_subdevice_name(pcminfo));
			QString name =
			    i18n("Card %1: ", card_name) +
			    _("|sound_card||") +
			    i18n("Device %1: ", device_name) +
			    _("|sound_device||") +
			    i18n("Subdevice %1: ", subdevice_name) +
			    _("|sound_subdevice");
			qDebug("# '%s' -> '%s'", hwdev.toLocal8Bit().data(),
			       name.toLocal8Bit().data());
			m_device_list.insert(name, hwdev);
		    }
		}
	    } else {
		// no sub-devices
		QString name = QString(
		    i18n("Card %1: ", card_name) +
		    _("|sound_card||") +
		    i18n("Device %1: ", device_name) +
		    _("|sound_subdevice")
		)/*.arg(card).arg(dev)*/;
// 		qDebug("# '%s' -> '%s'", hw_device.data(), name.data());
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

    // per default: offer the dsnoop plugin if any slave devices exist
    if (!m_device_list.isEmpty()) {
	m_device_list.insert(DEFAULT_DEVICE, _("plug:dsnoop"));
    }

    snd_ctl_card_info_free(info);
    snd_pcm_info_free(pcminfo);

    /*
     * BUG: this call is allowed due to ALSA documentation, but causes
     *      SIGSEGV when closing the record device. Somehow the internal
     *      structures of the PCM devices get messed up :-(
     *      (THE, 2009-07-18)
     */
    /* snd_config_update_free_global(); */
}

//***************************************************************************
QString Kwave::RecordALSA::alsaDeviceName(const QString &name)
{
    if (m_device_list.isEmpty() || (name.length() &&
        !m_device_list.contains(name)))
    {
	scanDevices();
    }

    if (!m_device_list.contains(name)) {
	// maybe we already have a ALSA compatible name (like in init state)
	foreach (QString n, m_device_list.values())
	    if (n == name) return n;

	qWarning("RecordALSA::alsaDeviceName('%s') - NOT FOUND",
	         name.toLocal8Bit().data());
	return _("");
    }
    return m_device_list[name];
}

#endif /* HAVE_ALSA_SUPPORT */

//***************************************************************************
//***************************************************************************
