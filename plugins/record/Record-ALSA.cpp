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

#include "Record-ALSA.h"

/** initializer for the list of devices */
QMap<QString, QString> RecordALSA::m_device_list;

/** list of known formats */
typedef struct alsa_sample_format_t {
    snd_pcm_format_t alsa_format;      /**< ALSA format identifier          */
    SampleFormat sample_format;        /**< sample format                   */
    unsigned int bits_per_sample;      /**< resolution [bits/sample]        */
    unsigned int bytes_per_sample;     /**< bytes per sample, for storage   */
    byte_order_t endian;               /**< endianness, big/little/cpu      */
    int compression;                   /**< compression mode                */
} alsa_sample_format_t;

/* define some endian dependend symbols that are missing in ALSA */
#if defined(ENDIANESS_BIG)
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
 *       comes first in the list. When searching for a format that matches
 *       a given set of parameters, the first entry is taken.
 *
 *       The sort order should be:
 *       - compression:      none -> ulaw -> alaw -> adpcm -> mpeg ...
 *       - bits per sample:  ascending
 *       - sample format:    signed -> unsigned -> float -> double ...
 *       - endianness:       cpu -> little -> big
 *       - bytes per sample: ascending
 */
static const alsa_sample_format_t _known_formats[] =
{
    /* 8 bit */
    { SND_PCM_FORMAT_S8, SampleFormat::Signed,   8, 1,
                         CpuEndian, AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_U8, SampleFormat::Unsigned, 8, 1,
                         CpuEndian, AF_COMPRESSION_NONE },

    /* 16 bit */
    { SND_PCM_FORMAT_S16,    SampleFormat::Signed,   16, 2,
                             CpuEndian,         AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_S16_LE, SampleFormat::Signed,   16, 2,
                             LittleEndian,      AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_S16_BE, SampleFormat::Signed,   16, 2,
                             BigEndian,         AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_U16,    SampleFormat::Unsigned, 16, 2,
                             CpuEndian,         AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_U16_LE, SampleFormat::Unsigned, 16, 2,
                             LittleEndian,      AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_U16_BE, SampleFormat::Unsigned, 16, 2,
                             BigEndian,         AF_COMPRESSION_NONE },

    /* 18 bit / 3 byte */
    { _SND_PCM_FORMAT_S18_3,  SampleFormat::Signed,   18, 3,
                              CpuEndian,         AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_S18_3LE, SampleFormat::Signed,   18, 3,
                              LittleEndian,      AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_S18_3BE, SampleFormat::Signed,   18, 3,
                              BigEndian,         AF_COMPRESSION_NONE },
    { _SND_PCM_FORMAT_U18_3,  SampleFormat::Unsigned, 18, 3,
                              CpuEndian,         AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_U18_3LE, SampleFormat::Unsigned, 18, 3,
                              LittleEndian,      AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_U18_3BE, SampleFormat::Unsigned, 18, 3,
                              BigEndian,         AF_COMPRESSION_NONE },

    /* 20 bit / 3 byte */
    { _SND_PCM_FORMAT_S20_3,  SampleFormat::Signed,   20, 3,
                              CpuEndian,         AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_S20_3LE, SampleFormat::Signed,   20, 3,
                              LittleEndian,      AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_S20_3BE, SampleFormat::Signed,   20, 3,
                              BigEndian,         AF_COMPRESSION_NONE },
    { _SND_PCM_FORMAT_U20_3,  SampleFormat::Unsigned, 20, 3,
                              CpuEndian,         AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_U20_3LE, SampleFormat::Unsigned, 20, 3,
                              LittleEndian,      AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_U20_3BE, SampleFormat::Unsigned, 20, 3,
                              BigEndian,         AF_COMPRESSION_NONE },

    /* 24 bit / 3 byte */
    { _SND_PCM_FORMAT_S24_3,  SampleFormat::Signed,   24, 3,
                              CpuEndian,         AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_S24_3LE, SampleFormat::Signed,   24, 3,
                              LittleEndian,      AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_S24_3BE, SampleFormat::Signed,   24, 3,
                              BigEndian,         AF_COMPRESSION_NONE },
    { _SND_PCM_FORMAT_U24_3,  SampleFormat::Unsigned, 24, 3,
                              CpuEndian,         AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_U24_3LE, SampleFormat::Unsigned, 24, 3,
                              LittleEndian,      AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_U24_3BE, SampleFormat::Unsigned, 24, 3,
                              BigEndian,         AF_COMPRESSION_NONE },

    /* 24 bit / 4 byte */
    { SND_PCM_FORMAT_S24,    SampleFormat::Signed,   24, 4,
                             CpuEndian,         AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_S24_LE, SampleFormat::Signed,   24, 4,
                             LittleEndian,      AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_S24_BE, SampleFormat::Signed,   24, 4,
                             BigEndian,         AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_U24,    SampleFormat::Unsigned, 24, 4,
                             CpuEndian,         AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_U24_LE, SampleFormat::Unsigned, 24, 4,
                             LittleEndian,      AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_U24_BE, SampleFormat::Unsigned, 24, 4,
                             BigEndian,         AF_COMPRESSION_NONE },

    /* 32 bit */
    { SND_PCM_FORMAT_S32,    SampleFormat::Signed,   32, 4,
                             CpuEndian,         AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_S32_LE, SampleFormat::Signed,   32, 4,
                             LittleEndian,      AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_S32_BE, SampleFormat::Signed,   32, 4,
                             BigEndian,         AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_U32,    SampleFormat::Unsigned, 32, 4,
                             CpuEndian,         AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_U32_LE, SampleFormat::Unsigned, 32, 4,
                             LittleEndian,      AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_U32_BE, SampleFormat::Unsigned, 32, 4,
                             BigEndian,         AF_COMPRESSION_NONE },

    /* float, 32 bit */
    { SND_PCM_FORMAT_FLOAT,    SampleFormat::Float,   32, 4,
                               CpuEndian,         AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_FLOAT_LE, SampleFormat::Float,   32, 4,
                               LittleEndian,      AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_FLOAT_BE, SampleFormat::Float,   32, 4,
                               BigEndian,         AF_COMPRESSION_NONE },

    /* float, 64 bit */
    { SND_PCM_FORMAT_FLOAT64,    SampleFormat::Double,  64, 8,
                                 CpuEndian,         AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_FLOAT64_LE, SampleFormat::Double,  64, 8,
                                 LittleEndian,      AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_FLOAT64_BE, SampleFormat::Double,  64, 8,
                                 BigEndian,         AF_COMPRESSION_NONE },

#if 0
    /* IEC958 subframes (not supported) */
    { SND_PCM_FORMAT_IEC958_SUBFRAME,    SampleFormat::Unknown, 0, 0,
                                 CpuEndian,         AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_IEC958_SUBFRAME_LE, SampleFormat::Unknown, 0, 0,
                                 LittleEndian, AF_COMPRESSION_NONE },
    { SND_PCM_FORMAT_IEC958_SUBFRAME_BE, SampleFormat::Unknown, 0, 0,
                                 BigEndian,         AF_COMPRESSION_NONE },
#endif

    /* G711 ULAW */
    { SND_PCM_FORMAT_MU_LAW, SampleFormat::Unknown, 0, 0,
                             CpuEndian, AF_COMPRESSION_G711_ULAW },

    /* G711 ALAW */
    { SND_PCM_FORMAT_A_LAW, SampleFormat::Unknown, 0, 0,
                            CpuEndian, AF_COMPRESSION_G711_ALAW },

    /*
     * some exotic formats, does anyone really use them
     * for recording ?
     * -> omit support for them, this makes life easier ;-)
     */
#if 0
    /* IMA ADPCM, 3 or 4 bytes per sample (not supported) */
    { SND_PCM_FORMAT_IMA_ADPCM, SampleFormat::Unknown, 0, 4,
                                CpuEndian, AF_COMPRESSION_MS_ADPCM },

    /* MPEG (not supported) */
    { SND_PCM_FORMAT_MPEG, SampleFormat::Unknown , 0, 0,
                           CpuEndian, CompressionType::MPEG_LAYER_I },

    /* GSM */
    { SND_PCM_FORMAT_GSM, SampleFormat::Unknown , 0, 0,
                          CpuEndian, AF_COMPRESSION_GSM },

    /* special (not supported) */
    { SND_PCM_FORMAT_SPECIAL, SampleFormat::Unknown , 0, 0,
                              CpuEndian, AF_COMPRESSION_NONE }
#endif
};

//***************************************************************************
RecordALSA::RecordALSA()
    :RecordDevice(), m_handle(0), m_tracks(0), m_rate(0.0), m_compression(0),
     m_bits_per_sample(0), m_sample_format(SampleFormat::Unknown),
     m_supported_formats(), m_initialized(false)
{
}

//***************************************************************************
RecordALSA::~RecordALSA()
{
    close();
}

//***************************************************************************
void RecordALSA::detectSupportedFormats()
{
    // start with an empty list
    m_supported_formats.clear();

    Q_ASSERT(m_handle);
    if (!m_handle) return;

    snd_pcm_t *pcm = m_handle;
    int err;
    snd_pcm_hw_params_t *p;

    snd_pcm_hw_params_alloca(&p);
    if (!p) return;

    if (!snd_pcm_hw_params_any(pcm, p) < 0) return;

    // try all known formats
    qDebug("--- list of supported formats --- ");
    const unsigned int count =
	sizeof(_known_formats) / sizeof(_known_formats[0]);
    for (unsigned int i=0; i < count; i++) {
	// test the sample format
	snd_pcm_format_t format = _known_formats[i].alsa_format;
	err = snd_pcm_hw_params_test_format(pcm, p, format);
	if (err < 0) continue;

	const alsa_sample_format_t *fmt = &(_known_formats[i]);

	// eliminate duplicate alsa sample formats (e.g. BE/LE)
	QValueListIterator<int> it;
	for (it = m_supported_formats.begin();
	     it != m_supported_formats.end(); ++it)
	{
	    const alsa_sample_format_t *f = &_known_formats[*it];
	    if (f->alsa_format == fmt->alsa_format) {
		fmt = 0;
		break;
	    }
	}
	if (!fmt) continue;

	CompressionType t;
	SampleFormat::Map sf;
	qDebug("#%2u, %2d, %2u bit [%u byte], %s, '%s', '%s'",
	    i,
	    fmt->alsa_format,
	    fmt->bits_per_sample,
	    fmt->bytes_per_sample,
	    fmt->endian == CpuEndian ? "CPU" :
	    (fmt->endian == LittleEndian ? "LE " : "BE "),
	    sf.name(sf.findFromData(fmt->sample_format)).local8Bit().data(),
	    t.name(t.findFromData(fmt->compression)).local8Bit().data());

	m_supported_formats.append(i);
    }
    qDebug("--------------------------------- ");

}

//***************************************************************************
int RecordALSA::open(const QString &device)
{
    int err;

    qDebug("RecordALSA::open(%s)", device.local8Bit().data());

    // close the previous device
    if (m_handle) snd_pcm_close(m_handle);
    m_handle = 0;

    // translate verbose name to internal ALSA name
    QString alsa_device = alsaDeviceName(device);
    qDebug("RecordALSA::open -> '%s'", alsa_device.local8Bit().data());

    if (!alsa_device.length()) return 0;

    // workaround for bug in ALSA
    // if the device name ends with "," -> invalid name
    if (alsa_device.endsWith(",")) return 0;

    // open the device in case it's not already open
    err = snd_pcm_open(&m_handle, alsa_device.local8Bit().data(),
	                   SND_PCM_STREAM_CAPTURE,
	                   SND_PCM_NONBLOCK);
    if (err < 0) {
	m_handle = 0;
	qWarning("RecordALSA::openDevice('%s') - failed, err=%d (%s)",
	         alsa_device.local8Bit().data(),
	         err, snd_strerror(err));
	return err;
    }

    // now we can detect all supported formats
    detectSupportedFormats();

    return 0;
}

//***************************************************************************
int RecordALSA::initialize()
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

    qDebug("RecordALSA::initialize");

//     m_bufbase     = bufbase;
//     m_buffer_size = 0;
//     m_buffer_used = 0;

    Q_ASSERT(m_handle);
    if (!m_handle) return -EBADF; // file not opened

    err = snd_output_stdio_attach(&output, stderr, 0);
    if (err < 0) {
	qWarning("Output failed: %s", snd_strerror(err));
    }

    snd_pcm_hw_params_alloca(&hw_params);
    if ((err = snd_pcm_hw_params_any(m_handle, hw_params)) < 0) {
	qWarning("Cannot initialize hardware parameters: %s",
	         snd_strerror(err));
	snd_output_close(output);
	return -EIO;
    }

    err = snd_pcm_hw_params_set_access(m_handle, hw_params,
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
	CompressionType t;
	SampleFormat::Map sf;

	qWarning("RecordkALSA::setFormat(): no matching format for "\
	         "compression '%s', %d bits/sample, format '%s'",
	         sf.name(sf.findFromData(m_sample_format)).local8Bit().data(),
	         m_bits_per_sample,
	         t.name(t.findFromData(m_compression)).local8Bit().data());

	snd_output_close(output);
	return -EINVAL;
    }

    Q_ASSERT(format_index >= 0);
    snd_pcm_format_t alsa_format = _known_formats[format_index].alsa_format;

    err = snd_pcm_hw_params_test_format(m_handle, hw_params, alsa_format);
    Q_ASSERT(!err);
    if (err) {
	qWarning("RecordkALSA::setFormat(): format %u is not supported",
	    (int)alsa_format);
	snd_output_close(output);
	return -EINVAL;
    }

    // activate the settings
    err = snd_pcm_hw_params_set_format(m_handle, hw_params, alsa_format);
    Q_ASSERT(!err);
    if (err < 0) {
	qWarning("Cannot set sample format: %s", snd_strerror(err));
	snd_output_close(output);
	return -EINVAL;
    }

    err = snd_pcm_hw_params_set_channels(m_handle, hw_params, m_tracks);
    if (err < 0) {
	qWarning("Cannot set channel count: %s", snd_strerror(err));
	snd_output_close(output);
	return -EINVAL;
    }

    unsigned int rrate = m_rate > 0 ? (unsigned int)rint(m_rate) : 0;
    err = snd_pcm_hw_params_set_rate_near(m_handle, hw_params, &rrate, 0);
    if (err < 0) {
	qWarning("Cannot set sample rate: %s", snd_strerror(err));
	snd_output_close(output);
	return -EINVAL;
    }
    qDebug("   real rate = %u", rrate);
    if (m_rate * 1.05 < rrate || m_rate * 0.95 > rrate) {
	qWarning("rate is not accurate (requested = %iHz, got = %iHz)",
	         (int)m_rate, (int)rrate);
	qWarning("         please, try the plug plugin (-Dplug:%s)",
	         snd_pcm_name(m_handle));
    }
    m_rate = rrate;

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

//     snd_pcm_hw_params_get_period_size(hw_params, &m_chunk_size, 0);
//     snd_pcm_hw_params_get_buffer_size(hw_params, &buffer_size);
//     if (m_chunk_size == buffer_size) {
// 	qWarning("Can't use period equal to buffer size (%lu == %lu)",
// 	         m_chunk_size, buffer_size);
// 	snd_output_close(output);
// 	return -EIO;
//     }
//
//     /* set software parameters */
//     snd_pcm_sw_params_alloca(&sw_params);
//     err = snd_pcm_sw_params_current(m_handle, sw_params);
//     if (err < 0) {
// 	qWarning("Unable to determine current software parameters: %s",
// 	         snd_strerror(err));
// 	snd_output_close(output);
// 	return err;
//     }
//
//     err = snd_pcm_sw_params_get_xfer_align(sw_params, &xfer_align);
//     if (err < 0) {
// 	qWarning("Unable to obtain xfer align: %s", snd_strerror(err));
// 	snd_output_close(output);
// 	return err;
//     }
//     if (g_sleep_min) xfer_align = 1;
//
//     err = snd_pcm_sw_params_set_sleep_min(m_handle, sw_params, g_sleep_min);
//     Q_ASSERT(err >= 0);
//     if (avail_min < 0)
// 	n = m_chunk_size;
//     else
// 	n = (snd_pcm_uframes_t)((double)rate * avail_min / 1000000);
//
//     err = snd_pcm_sw_params_set_avail_min(m_handle, sw_params, n);
//
//     /* round up to closest transfer boundary */
//     n = (buffer_size / xfer_align) * xfer_align;
//     start_threshold = (snd_pcm_uframes_t)
//                       ((double)rate * start_delay / 1000000);
//     if (start_delay <= 0) start_threshold += n;
//
//     if (start_threshold < 1) start_threshold = 1;
//     if (start_threshold > n) start_threshold = n;
//     err = snd_pcm_sw_params_set_start_threshold(m_handle, sw_params,
//                                                 start_threshold);
//     Q_ASSERT(err >= 0);
//     stop_threshold = (snd_pcm_uframes_t)
//                      ((double)rate * stop_delay / 1000000);
//     if (stop_delay <= 0) stop_threshold += buffer_size;
//
//     err = snd_pcm_sw_params_set_stop_threshold(m_handle, sw_params,
//                                                stop_threshold);
//     Q_ASSERT(err >= 0);
//
//     err = snd_pcm_sw_params_set_xfer_align(m_handle, sw_params, xfer_align);
//     Q_ASSERT(err >= 0);
//
//     // write the software parameters to the recording device
//     err = snd_pcm_sw_params(m_handle, sw_params);
//     if (err < 0) {
// 	qDebug("   activating snd_pcm_sw_params FAILED");
// 	snd_pcm_dump(m_handle, output);
// 	qWarning("Unable to set software parameters: %s", snd_strerror(err));
//     }

//     // prepare the device for recording
//     if ((err = snd_pcm_prepare(m_handle)) < 0) {
// 	snd_pcm_dump(m_handle, output);
// 	qWarning("cannot prepare interface for use: %s",snd_strerror(err));
//     }
//
//     // resize our buffer and reset it
//     Q_ASSERT(m_chunk_size);
//     Q_ASSERT(m_bytes_per_sample);
//     unsigned int chunk_bytes = m_chunk_size * m_bytes_per_sample;
//     Q_ASSERT(chunk_bytes);
//     if (!chunk_bytes) return 0;
//     unsigned int n = (unsigned int)(ceil((float)(1 << m_bufbase) /
//                                          (float)chunk_bytes));
//     if (n < 1) n = 1;
//     m_buffer_size = n * m_chunk_size * m_bytes_per_sample;
//     m_buffer.resize(m_buffer_size);
//     m_buffer_size = m_buffer.size();
//
//     qDebug("RecordALSA::open: OK, buffer resized to %u bytes",
//            m_buffer_size);

    snd_pcm_dump(m_handle, output);
    snd_output_close(output);

    return 0;
}

//***************************************************************************
int RecordALSA::read(char *buffer, unsigned int length)
{
    if (!m_handle) return -EBADF; // file not opened
    Q_ASSERT(buffer);
    if (!buffer)   return -EINVAL; // buffer is null pointer
    if (!length)   return 0;       // no buffer, nothing to do

    // we configure our device at a later stage, not on the fly
    if (!m_initialized) {
	int err = initialize();
	if (err < 0) return err;
    }

    /* ### TODO ### */
    return -EBADF;
}

//***************************************************************************
int RecordALSA::close()
{
    // close the device handle
    if (m_handle) snd_pcm_close(m_handle);
    m_handle = 0;

    // we need to re-initialize the next time
    m_initialized = false;

    // clear the list of supported formats, nothing open -> nothing supported
    m_supported_formats.clear();

    return 0;
}

//***************************************************************************
int RecordALSA::detectTracks(unsigned int &min, unsigned int &max)
{
    snd_pcm_t *pcm = m_handle;
    min = max = 0;
    snd_pcm_hw_params_t *p;

    if (!pcm) return -1;

    snd_pcm_hw_params_alloca(&p);
    if (!p) return -1;

    if (snd_pcm_hw_params_any(pcm, p) >= 0) {
	int err;
	if ((err = snd_pcm_hw_params_get_channels_min(p, &min)) < 0)
	    qWarning("RecordALSA::detectTracks: min: %s",
		     snd_strerror(err));
	if ((err = snd_pcm_hw_params_get_channels_max(p, &max)) < 0)
	    qWarning("RecordALSA::detectTracks: max: %s",
		     snd_strerror(err));
    }

//  qDebug("RecordALSA::detectChannels, min=%u, max=%u", min, max);
    return 0;
}

//***************************************************************************
int RecordALSA::setTracks(unsigned int &tracks)
{
    m_tracks = tracks;
    return 0;
}

//***************************************************************************
int RecordALSA::tracks()
{
    return m_tracks;
}

//***************************************************************************
QValueList<double> RecordALSA::detectSampleRates()
{
    QValueList<double> list;
    snd_pcm_t *pcm = m_handle;
    int err;
    snd_pcm_hw_params_t *p;

    if (!pcm) return list;

    snd_pcm_hw_params_alloca(&p);
    if (!p) return list;

    if (!snd_pcm_hw_params_any(pcm, p) < 0) return list;

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
    for (unsigned int i=0; i < sizeof(known_rates)/sizeof(unsigned int); i++) {
	unsigned int rate = known_rates[i];

	err = snd_pcm_hw_params_test_rate(pcm, p, rate, 0);
	if (err < 0) continue;

	// do not produce duplicates
	if (list.contains(rate)) continue;

	qDebug("found rate %u Hz", rate);
	list.append(rate);
    }

    return list;
}

//***************************************************************************
int RecordALSA::setSampleRate(double &new_rate)
{
    m_rate = new_rate;
    return 0;
}

//***************************************************************************
double RecordALSA::sampleRate()
{
    return m_rate;
}

//***************************************************************************
int RecordALSA::mode2format(int compression, int bits,
                            SampleFormat sample_format)
{
    // loop over all supported formats and keep only those that are
    // compatible with the given compression, bits and sample format
    QValueListIterator<int> it;
    for (it = m_supported_formats.begin();
         it != m_supported_formats.end(); ++it)
    {
	const int index = *it;
	const alsa_sample_format_t *fmt = &_known_formats[index];

	if (fmt->compression != compression) continue;
	if ((int)fmt->bits_per_sample != bits) continue;
	if (!(fmt->sample_format == sample_format)) continue;

	// mode is compatible
	// As the list of known formats is already sorted so that
	// the simplest formats come first, we don't have a lot
	// of work -> just take the first entry ;-)
	qDebug("RecordALSA::mode2format -> %d", index);
	return index;
    }

    qWarning("RecordALSA::mode2format -> no match found !?");
    return -1;
}

//***************************************************************************
QValueList<int> RecordALSA::detectCompressions()
{
    QValueList<int> list;

    // try all known sample formats
    QValueListIterator<int> it;
    for (it = m_supported_formats.begin();
         it != m_supported_formats.end(); ++it)
    {
	const alsa_sample_format_t *fmt = &(_known_formats[*it]);
	int compression = fmt->compression;

	// do not produce duplicates
	if (list.contains(compression)) continue;

	CompressionType t;
	qDebug("found compression %d '%s'", compression,
	       t.name(t.findFromData(compression)).local8Bit().data());
	list.append(compression);
    }

    return list;
}

//***************************************************************************
int RecordALSA::setCompression(int new_compression)
{
    m_compression = new_compression;
    return m_compression;
}

//***************************************************************************
int RecordALSA::compression()
{
    return m_compression;
}

//***************************************************************************
QValueList <unsigned int> RecordALSA::supportedBits()
{
    QValueList <unsigned int> list;

    // try all known sample formats
    QValueListIterator<int> it;
    for (it = m_supported_formats.begin();
         it != m_supported_formats.end(); ++it)
    {
	const alsa_sample_format_t *fmt = &(_known_formats[*it]);
	const unsigned int bits = fmt->bits_per_sample;

	// 0  bits means invalid/does not apply
	if (!bits) continue;

	// only accept bits/sample if compression matches
	if (fmt->compression != m_compression) continue;

	// do not produce duplicates
	if (list.contains(bits)) continue;

	qDebug("found bits/sample %u", bits);
	list.append(bits);
    }

    return list;
}

//***************************************************************************
int RecordALSA::setBitsPerSample(unsigned int new_bits)
{
    m_bits_per_sample = new_bits;
    return 0;
}

//***************************************************************************
int RecordALSA::bitsPerSample()
{
    return m_bits_per_sample;
}

//***************************************************************************
QValueList<SampleFormat> RecordALSA::detectSampleFormats()
{
    QValueList<SampleFormat> list;

    // try all known sample formats
    QValueListIterator<int> it;
    for (it = m_supported_formats.begin();
         it != m_supported_formats.end(); ++it)
    {
	const alsa_sample_format_t *fmt = &(_known_formats[*it]);
	const SampleFormat sample_format = fmt->sample_format;

	// only accept bits/sample if compression types
	// and bits per sample match
	if (fmt->compression != m_compression) continue;
	if (fmt->bits_per_sample != m_bits_per_sample) continue;

	// do not produce duplicates
	if (list.contains(sample_format)) continue;

	SampleFormat::Map sf;
	qDebug("found sample format %u ('%s')", (int)sample_format,
		sf.name(sf.findFromData(sample_format)).local8Bit().data());

	list.append(sample_format);
    }

    return list;
}

//***************************************************************************
int RecordALSA::setSampleFormat(SampleFormat new_format)
{
    m_sample_format = new_format;
    return 0;
}

//***************************************************************************
SampleFormat RecordALSA::sampleFormat()
{
    return m_sample_format;
}

//***************************************************************************
byte_order_t RecordALSA::endianness()
{
    int index = mode2format(m_compression, m_bits_per_sample, m_sample_format);
    return (index >= 0) ? _known_formats[index].endian : UnknownEndian;
}

//***************************************************************************
QStringList RecordALSA::supportedDevices()
{
    QStringList list;

    // re-validate the list if necessary
    scanDevices();

    QMap<QString, QString>::Iterator it;
    for (it = m_device_list.begin(); it != m_device_list.end(); ++it)
	list.append(it.key());

    list.append("#TREE#");
    return list;
}

//***************************************************************************
void RecordALSA::scanDevices()
{
    snd_ctl_t *handle = 0;
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

    qDebug("**** List of RECORD Hardware Devices ****");
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
	    hw_device = "plughw:%1,%2";
	    hw_device = hw_device.arg(card).arg(dev);

	    QString card_name   = snd_ctl_card_info_get_name(info);
	    QString device_name = snd_pcm_info_get_name(pcminfo);

//  	    qDebug("  Subdevices: %i/%i\n",
// 		snd_pcm_info_get_subdevices_avail(pcminfo), count);
	    if (count > 1) {
		for (idx = 0; idx < (int)count; idx++) {
		    snd_pcm_info_set_subdevice(pcminfo, idx);
		    if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
			qWarning("ctrl digital audio playback info (%i): %s",
			         card, snd_strerror(err));
		    } else {
			QString hwdev = hw_device + QString(",%1").arg(idx);
			QString subdevice_name =
			    snd_pcm_info_get_subdevice_name(pcminfo);
			QString name = QString(
			    i18n("card %1: ") + card_name +
			    "|sound_card||" +
			    i18n("device %2: ") + device_name +
			    "|sound_device||" +
			    i18n("subdevice %3: ") + subdevice_name +
			    "|sound_subdevice"
			).arg(card).arg(dev).arg(idx);
			qDebug("# '%s' -> '%s'", hwdev.data(), name.data());
			m_device_list.insert(name, hwdev);
		    }
		}
	    } else {
		// no sub-devices
		QString name = QString(
		    i18n("card %1: ") + card_name + "|sound_card||" +
		    i18n("device %2: ") + device_name + "|sound_subdevice"
		).arg(card).arg(dev);
		qDebug("# '%s' -> '%s'", hw_device.data(), name.data());
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

//     // per default: offer the dmix plugin if slave devices exist
//     if (!m_device_list.isEmpty())
//         m_device_list.insert(i18n("DMIX plugin")+QString("|sound_note"),
//                              "plug:dmix");

    snd_config_update_free_global();
}

//***************************************************************************
QString RecordALSA::alsaDeviceName(const QString &name)
{
    if (m_device_list.isEmpty() || (name.length() &&
        !m_device_list.contains(name)))
    {
	scanDevices();
    }

    if (!m_device_list.contains(name)) {
	qWarning("RecordALSA::alsaDeviceName('%s') - NOT FOUND", name.data());
	return "";
    }
    return m_device_list[name];
}

#endif /* HAVE_ALSA_SUPPORT */

//***************************************************************************
//***************************************************************************
