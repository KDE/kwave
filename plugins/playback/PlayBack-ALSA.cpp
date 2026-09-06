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
 ***************************************************************************/

#include "config.h"
#ifdef HAVE_ALSA_SUPPORT

#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <new>

#include <QLatin1Char>
#include <QString>
#include <QtGlobal>

#include <KLocalizedString>

#include "libkwave/Compression.h"
#include "libkwave/SampleEncoderLinear.h"
#include "libkwave/SampleFormat.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"
#include "libkwave/memcpy.h"

#include "PlayBack-ALSA.h"

QMap<QString, QString> Kwave::PlayBackALSA::m_device_list;

/** gui name of the null device */
#define NULL_DEVICE (i18n("Null device") + _("|sound_note"))

//***************************************************************************

/* define some endian dependent symbols that are missing in ALSA */
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
// big endian
#define SND_PCM_FORMAT_S18_3 SND_PCM_FORMAT_S18_3BE
#define SND_PCM_FORMAT_U18_3 SND_PCM_FORMAT_U18_3BE
#define SND_PCM_FORMAT_U20_3 SND_PCM_FORMAT_U20_3BE
#define SND_PCM_FORMAT_S20_3 SND_PCM_FORMAT_S20_3BE
#define SND_PCM_FORMAT_U24_3 SND_PCM_FORMAT_U24_3BE
#define SND_PCM_FORMAT_S24_3 SND_PCM_FORMAT_S24_3BE

#else
// little endian
#define SND_PCM_FORMAT_S18_3 SND_PCM_FORMAT_S18_3LE
#define SND_PCM_FORMAT_U18_3 SND_PCM_FORMAT_U18_3LE
#define SND_PCM_FORMAT_U20_3 SND_PCM_FORMAT_U20_3LE
#define SND_PCM_FORMAT_S20_3 SND_PCM_FORMAT_S20_3LE
#define SND_PCM_FORMAT_U24_3 SND_PCM_FORMAT_U24_3LE
#define SND_PCM_FORMAT_S24_3 SND_PCM_FORMAT_S24_3LE

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
    SND_PCM_FORMAT_S18_3, SND_PCM_FORMAT_S18_3LE, SND_PCM_FORMAT_S18_3BE,
    SND_PCM_FORMAT_U18_3, SND_PCM_FORMAT_U18_3LE, SND_PCM_FORMAT_U18_3BE,

    /* 20 bit / 3 byte */
    SND_PCM_FORMAT_S20_3, SND_PCM_FORMAT_S20_3LE, SND_PCM_FORMAT_S20_3BE,
    SND_PCM_FORMAT_U20_3, SND_PCM_FORMAT_U20_3LE, SND_PCM_FORMAT_U20_3BE,

    /* 24 bit / 3 byte */
    SND_PCM_FORMAT_S24_3, SND_PCM_FORMAT_S24_3LE, SND_PCM_FORMAT_S24_3BE,
    SND_PCM_FORMAT_U24_3, SND_PCM_FORMAT_U24_3LE, SND_PCM_FORMAT_U24_3BE,

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

    /* G711 ULAW */
    SND_PCM_FORMAT_MU_LAW,

    /* G711 ALAW */
    SND_PCM_FORMAT_A_LAW,
};

//***************************************************************************
/** find out the SampleFormat of an ALSA format */
static Kwave::SampleFormat::Format sample_format_of(snd_pcm_format_t fmt)
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
    } else if (fmt == SND_PCM_FORMAT_A_LAW)
        return Kwave::SampleFormat::Unsigned;
    else if (fmt == SND_PCM_FORMAT_MU_LAW)
        return Kwave::SampleFormat::Unsigned;

    return Kwave::SampleFormat::Unknown;
}

//***************************************************************************
static Kwave::byte_order_t endian_of(snd_pcm_format_t fmt)
{
    if (snd_pcm_format_little_endian(fmt) == 1)
        return Kwave::LittleEndian;
    if (snd_pcm_format_big_endian(fmt) == 1)
        return Kwave::BigEndian;
    return Kwave::CpuEndian;
}

//***************************************************************************
static Kwave::Compression::Type compression_of(snd_pcm_format_t fmt)
{
    Kwave::Compression::Type c = Kwave::Compression::NONE;
    switch (fmt) {
        case SND_PCM_FORMAT_MU_LAW:
            c = Kwave::Compression::G711_ULAW;    break;
        case SND_PCM_FORMAT_A_LAW:
            c = Kwave::Compression::G711_ALAW;    break;
        default:
            break;
    }
    return c;
}

//***************************************************************************
static inline unsigned int bits_of(snd_pcm_format_t fmt)
{
    return snd_pcm_format_width(fmt);
}

//***************************************************************************
/**
 * Wrapper for avoiding use of the ALSA alloca based functions which do not
 * work well with -fstack-protector of newer GCC versions, and also avoids
 * potential memory leaks when using pure ALSA malloc/free based functions.
 */
template <class T, int (allocator)(T**), void (deleter)(T*)>
        class AlsaMallocWrapper
{
public:
    /** constructor, creates an object using the allocator function */
    AlsaMallocWrapper()
        :m_data(nullptr)
    {
        allocator(&m_data);
    }

    /** destructor, frees the object using the deleter function */
    virtual ~AlsaMallocWrapper()
    {
        if (m_data) deleter(m_data);
        m_data = nullptr;
    }

    /** conversion operator, returns the ALSA object */
    inline operator T *&() { return m_data; }

private:
    T *m_data; /**< allocated ALSA object */
};

//***************************************************************************

/** wrapper for AlsaMallocWrapper */
#define ALSA_MALLOC_WRAPPER(__t__) \
    AlsaMallocWrapper<__t__##_t, __t__##_malloc, __t__##_free>

//***************************************************************************
Kwave::PlayBackALSA::PlayBackALSA()
    :Kwave::PlayBackDevice(),
    m_device_name(),
    m_handle(nullptr),
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
    m_encoder(nullptr)
{
}

//***************************************************************************
Kwave::PlayBackALSA::~PlayBackALSA()
{
    close();
}

//***************************************************************************
int Kwave::PlayBackALSA::setFormat(snd_pcm_hw_params_t *hw_params,
                                   unsigned int bits)
{
    qDebug("PlayBackALSA::setFormat(..., bits=%u)", bits);
    m_format = SND_PCM_FORMAT_UNKNOWN;
    m_bits = 0;
    m_bytes_per_sample = 0;
    delete m_encoder;
    m_encoder = nullptr;

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

    m_encoder = new(std::nothrow) Kwave::SampleEncoderLinear(
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
int Kwave::PlayBackALSA::mode2format(int bits)
{
    // loop over all supported formats and keep only those that are
    // compatible with the given compression, bits and sample format
    for (int index : m_supported_formats) {
        const snd_pcm_format_t *fmt = &_known_formats[index];

        if (snd_pcm_format_width(*fmt) != bits) continue;

        // mode is compatible
        // As the list of known formats is already sorted so that
        // the simplest formats come first, we don't have a lot
        // of work -> just take the first entry ;-)
//      qDebug("PlayBackALSA::mode2format -> %d", index);
        return index;
    }

    qWarning("PlayBackALSA::mode2format -> no match found !?");
    return -1;
}

//***************************************************************************
void Kwave::PlayBackALSA::detectSupportedFormats(const QString &device)
{
    // start with an empty list
    m_supported_formats.clear();

    ALSA_MALLOC_WRAPPER(snd_pcm_hw_params) p;

    if (!p) return;

    snd_pcm_t *pcm = openDevice(device);
    if (!pcm) return;

    if (snd_pcm_hw_params_any(pcm, p) < 0) {
        if (pcm != m_handle) snd_pcm_close(pcm);
        return;
    }

    // try all known formats
    qDebug("--- list of supported formats --- ");
    for (const snd_pcm_format_t &fmt : _known_formats) {
        // test the sample format
        int err = snd_pcm_hw_params_test_format(pcm, p, fmt);
        if (err < 0) continue;

        // eliminate duplicate ALSE sample formats
        if (m_supported_formats.contains(fmt))
            continue;

        qDebug("detected[%3d]: comp=%3d, bits=%2u, fmt=%3d",
               int(fmt),
               int(compression_of(fmt)),
               bits_of(fmt),
               int(sample_format_of(fmt))
        );

        m_supported_formats.append(fmt);
    }
    qDebug("--------------------------------- ");

    if (pcm != m_handle) snd_pcm_close(pcm);
}

//***************************************************************************
int Kwave::PlayBackALSA::openDevice(const QString &device, unsigned int rate,
                                    unsigned int channels, unsigned int bits)
{
    int err;
    snd_output_t *output = nullptr;
    ALSA_MALLOC_WRAPPER(snd_pcm_hw_params) hw_params;
    ALSA_MALLOC_WRAPPER(snd_pcm_sw_params) sw_params;
    snd_pcm_uframes_t buffer_size;
    unsigned period_time = 0; // period time in us
    unsigned buffer_time = 0; // ring buffer length in us
    snd_pcm_uframes_t period_frames = 0;
    snd_pcm_uframes_t buffer_frames = 0;
    snd_pcm_uframes_t start_threshold, stop_threshold;

    m_chunk_size = 0;
    if (m_handle) snd_pcm_close(m_handle);
    m_handle = nullptr;

    // translate verbose name to internal ALSA name
    QString alsa_device = alsaDeviceName(device);
    qDebug("PlayBackALSA::openDevice() - opening ALSA device '%s', "
           "%uHz %u channels, %u bit",
           DBG(alsa_device.split(_("|")).at(0)), rate, channels, bits);

    // workaround for bug in ALSA
    // if the device name ends with "," -> invalid name
    if (alsa_device.endsWith(_(","))) return -ENODEV;

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
    err = snd_pcm_hw_params_set_rate_near(m_handle, hw_params, &rrate,
                                          nullptr);
    if (err < 0) {
        qWarning("Cannot set sample rate: %s", snd_strerror(err));
        snd_output_close(output);
        return err;
    }
    qDebug("   real rate = %u", rrate);
    if (static_cast<float>(rate) * 1.05f < static_cast<float>(rrate) ||
        static_cast<float>(rate) * 0.95f > static_cast<float>(rrate))
    {
        qWarning("rate is not accurate (requested = %uHz, got = %uHz)",
                  rate, rrate);
        qWarning("         please, try the plug plugin (-Dplug:%s)",
                 snd_pcm_name(m_handle));
    }

    err = snd_pcm_hw_params_get_buffer_time_max(hw_params, &buffer_time,
                                                nullptr);
    Q_ASSERT(err >= 0);
    if (buffer_time > 500000) buffer_time = 500000;
    if (buffer_time > 0)
        period_time = buffer_time / 4;
    else
        period_frames = buffer_frames / 4;

    if (period_time > 0) {
        err = snd_pcm_hw_params_set_period_time_near(m_handle, hw_params,
                                                     &period_time, nullptr);
    } else {
        err = snd_pcm_hw_params_set_period_size_near(m_handle, hw_params,
                                                     &period_frames, nullptr);
    }
    Q_ASSERT(err >= 0);
    if (buffer_time > 0) {
        err = snd_pcm_hw_params_set_buffer_time_near(m_handle, hw_params,
                                                     &buffer_time, nullptr);
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

    snd_pcm_hw_params_get_period_size(hw_params, &m_chunk_size, nullptr);
    snd_pcm_hw_params_get_buffer_size(hw_params, &buffer_size);
    if (m_chunk_size == buffer_size) {
        qWarning("Can't use period equal to buffer size (%lu == %lu)",
                 m_chunk_size, buffer_size);
        snd_output_close(output);
        return -EIO;
    }

    /* set software parameters */
    err = snd_pcm_sw_params_current(m_handle, sw_params);
    if (err < 0) {
        qWarning("Unable to determine current software parameters: %s",
                 snd_strerror(err));
        snd_output_close(output);
        return err;
    }

    err = snd_pcm_sw_params_set_avail_min(m_handle, sw_params, m_chunk_size);
    Q_ASSERT(err >= 0);

    /* round up to closest transfer boundary */
    start_threshold = 1;
    err = snd_pcm_sw_params_set_start_threshold(m_handle, sw_params,
                                                start_threshold);
    Q_ASSERT(err >= 0);
    stop_threshold = buffer_size;

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

    snd_pcm_dump(m_handle, output);
    snd_output_close(output);

    // prepare the device for playback
    if ((err = snd_pcm_prepare(m_handle)) < 0) {
        snd_pcm_dump(m_handle, output);
        qWarning("cannot prepare interface for use: %s", snd_strerror(err));
    }

    return 0;
}

//***************************************************************************
QString Kwave::PlayBackALSA::open(const QString &device, double rate,
                                  unsigned int channels, unsigned int bits,
                                  unsigned int bufbase)
{
    qDebug("PlayBackALSA::open(device=%s, rate=%0.1f, channels=%u, bits=%u, "
           "bufbase=%u)", DBG(device), rate, channels, bits, bufbase);

    m_device_name = device;
    m_rate        = rate;
    m_channels    = channels;
    m_bits        = 0;
    m_bufbase     = bufbase;
    m_buffer_size = 0;
    m_buffer_used = 0;

    // close the previous device
    if (m_handle) snd_pcm_close(m_handle);
    m_handle = nullptr;
    delete m_encoder;
    m_encoder = nullptr;

    // initialize the list of supported formats
    detectSupportedFormats(device);

    int err = openDevice(device, Kwave::toUint(rate), channels, bits);
    if (err) {
        QString reason;
        switch (err) {
            case ENOENT:
            case ENODEV:
            case ENXIO:
            case EIO:
                reason = i18n("I/O error. Maybe the driver\n"\
                "is not present in your kernel or it is not\n"\
                "properly configured.");
                break;
            case EBUSY:
                reason = i18n(
                "The device is busy. Maybe some other application is\n"\
                "currently using it. Please try again later.\n"\
                "(Hint: you might find out the name and process ID of\n"\
                "the program by calling: \"fuser -v %1\"\n"\
                "on the command line.)",
                m_device_name);
                break;
            default:
                reason = i18n("Opening the device '%1' failed: %2",
                    device.section(QLatin1Char('|'), 0, 0),
                    QString::fromLocal8Bit(snd_strerror(err)));
        }
        return reason;
    }

    // resize our buffer and reset it
    Q_ASSERT(m_chunk_size);
    Q_ASSERT(m_bytes_per_sample);
    unsigned int chunk_bytes = Kwave::toUint(m_chunk_size) * m_bytes_per_sample;
    Q_ASSERT(chunk_bytes);
    if (!chunk_bytes) return QString();
    unsigned int n = Kwave::toUint(ceil(
        static_cast<double>(1 << m_bufbase) /
        static_cast<double>(chunk_bytes)));
    if (n < 1) n = 1;
    m_buffer_size = n * chunk_bytes;
    m_buffer.resize(m_buffer_size);
    m_buffer_size = static_cast<unsigned int>(m_buffer.size());

//     qDebug("PlayBackALSA::open: OK, buffer resized to %u bytes",
//            m_buffer_size);

    return QString();
}

//***************************************************************************
int Kwave::PlayBackALSA::write(const Kwave::SampleArray &samples)
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
    MEMCPY(m_buffer.data() + m_buffer_used, raw.constData(), bytes);
    m_buffer_used += bytes;

    // write buffer to device if full
    if (m_buffer_used >= m_buffer_size) return flush();

    return 0;
}

//***************************************************************************
int Kwave::PlayBackALSA::flush()
{
    if (!m_buffer_used) return 0; // nothing to do
    Q_ASSERT(m_channels);
    Q_ASSERT(m_bytes_per_sample);
    if (!m_channels || !m_bytes_per_sample) return -EINVAL;

    if (m_handle) {
        snd_pcm_uframes_t samples = m_buffer_used / m_bytes_per_sample;
        unsigned int buffer_samples = m_buffer_size / m_bytes_per_sample;
        unsigned int timeout = (m_rate > 0) ?
            3 * ((1000 * buffer_samples) /
            Kwave::toUint(m_rate)) : 1000U;
        quint8 *p = reinterpret_cast<quint8 *>(m_buffer.data());

        // pad the buffer with silence if necessary
        if (samples < m_chunk_size) {
            snd_pcm_format_set_silence(m_format,
                m_buffer.data() + samples * m_bytes_per_sample,
                Kwave::toUint((m_chunk_size - samples) * m_channels));
            samples = m_chunk_size;
            qDebug("--- added silence ---");
        }

        while (samples > 0) {
            // try to write as much as the device accepts
            int r = Kwave::toInt(snd_pcm_writei(m_handle, p, samples));
            if ((r == -EAGAIN) || ((r >= 0) && (r < Kwave::toInt(samples))))
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
                Q_ASSERT(r <= Kwave::toInt(samples));
                p       += r * m_bytes_per_sample;
                samples -= r;
            }
        }
    }

    m_buffer_used = 0;
    return 0;
}

//***************************************************************************
int Kwave::PlayBackALSA::close()
{
    flush();

    // close the device handle
    if (m_handle) snd_pcm_close(m_handle);
    m_handle = nullptr;

    // get rid of the old sample encoder
    delete m_encoder;
    m_encoder = nullptr;

    // clear the list of supported formats, nothing open -> nothing supported
    m_supported_formats.clear();

    return 0;
}

//***************************************************************************
/**
 * Check whether a PCM hint name is a relevant candidate for playback.
 * Filters out raw hw/plughw names (scanned separately above) and
 * names that are added manually further down (dmix, default, null),
 * to avoid duplicate or redundant entries.
 */
static bool isRelevantPlaybackHint(const QString &device_name)
{
    if (device_name.startsWith(_("hw:")) ||
        device_name.startsWith(_("plughw:")))
        return false; // already covered by the hardware scan above

    static const QStringList exact_blacklist = {
        _("null"), _("upmix"), _("vdownmix")
    };
    if (exact_blacklist.contains(device_name))
        return false; // added manually below, or not useful here

    static const QStringList prefix_blacklist = {
        _("dsnoop"), _("surround"), _("hdmi"), _("iec958"),
        _("speex"), _("ladspa"), _("oss"), _("usbstream")
    };
    for (const QString &prefix : prefix_blacklist)
        if (device_name.startsWith(prefix))
            return false;

    if (device_name.contains(_("rate")))
        return false;

    return true;
}

//***************************************************************************
/**
 * Try to open and configure a PCM device for playback with a
 * minimal, realistic parameter set. A successful snd_pcm_open()
 * alone is not enough: some PipeWire pseudo nodes open fine but
 * have no real sink behind them and fail later during hw_params
 * negotiation. This weeds those out so they are never offered to
 * the user in the first place.
 */
static bool probePlaybackDevice(const QString &device_name)
{
    snd_pcm_t *pcm = nullptr;
    int err = snd_pcm_open(&pcm, device_name.toLocal8Bit().data(),
                           SND_PCM_STREAM_PLAYBACK,
                           SND_PCM_NONBLOCK);

    // device exists but is busy: assume it's usable, we simply
    // cannot verify hw_params negotiation right now
    if ((err == -EBUSY) || (err == -EAGAIN))
        return true;

    if (err < 0)
        return false; // could not even open it -> not usable

    snd_pcm_hw_params_t *hw = nullptr;
    snd_pcm_hw_params_malloc(&hw);
    bool usable = false;

    if (hw && (snd_pcm_hw_params_any(pcm, hw) >= 0)) {
        unsigned int rate = 44100;
        snd_pcm_hw_params_set_access(pcm, hw,
                                     SND_PCM_ACCESS_RW_INTERLEAVED);
        snd_pcm_hw_params_set_format(pcm, hw,
                                     SND_PCM_FORMAT_S16_LE);
        snd_pcm_hw_params_set_channels(pcm, hw, 2);
        snd_pcm_hw_params_set_rate_near(pcm, hw, &rate, nullptr);

        // the real check: can these params actually be
        // committed, or does the device only pretend to work?
        usable = (snd_pcm_hw_params(pcm, hw) >= 0);
    }

    if (hw)
        snd_pcm_hw_params_free(hw);
    snd_pcm_close(pcm);

    return usable;
}

//***************************************************************************
/** build a human readable label for a virtual/plugin PCM device */
static QString virtualPlaybackLabel(const QString &description,
                                    const QString &device_name)
{
    if (description.isEmpty())
        return device_name;

    QString desc = description;
    desc.replace(_("\n"), _(" - "));
    return QString(_("%1 (%2)")).arg(desc, device_name);
}

//***************************************************************************
void Kwave::PlayBackALSA::scanDevices()
{
    snd_ctl_t *handle;
    int card, err, dev;
    int idx;
    ALSA_MALLOC_WRAPPER(snd_ctl_card_info) info;
    ALSA_MALLOC_WRAPPER(snd_pcm_info) pcminfo;

    m_device_list.clear();

    // 1. scan physical hardware devices, build kwave ui tree
    card = -1;
    if (snd_card_next(&card) < 0 || card < 0)
        qWarning("no soundcards found...");

    while (card >= 0) {
        QString name;
        name = _("hw:%1");
        name = name.arg(card);
        if ((err = snd_ctl_open(&handle, name.toLocal8Bit().data(),
                                0)) < 0) {
            qWarning("control open (%i): %s", card,
                     snd_strerror(err));
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
            if (snd_ctl_pcm_next_device(handle, &dev) < 0)
                qWarning("snd_ctl_pcm_next_device");
            if (dev < 0)
                break;
            snd_pcm_info_set_device(pcminfo, dev);
            snd_pcm_info_set_subdevice(pcminfo, 0);
            snd_pcm_info_set_stream(pcminfo,
                                    SND_PCM_STREAM_PLAYBACK);
            if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
                if (err != -ENOENT)
                    qWarning("control digital audio info (%i): "
                             "%s", card, snd_strerror(err));
                continue;
            }
            count = snd_pcm_info_get_subdevices_count(pcminfo);

            QString hw_device;
            hw_device = _("plughw:%1,%2");
            hw_device = hw_device.arg(card).arg(dev);

            QString card_name = _(snd_ctl_card_info_get_name(info));
            QString device_name = _(snd_pcm_info_get_name(pcminfo));

            if (count <= 1) {
                // no sub-devices
                QString full_name = QString(
                    i18n("Card %1: ", card) +
                         card_name + _("|sound_card||") +
                    i18n("Device %1: ", dev) +
                          device_name + _("|sound_subdevice")
                );
                m_device_list.insert(full_name, hw_device);
                continue;
            }

            for (idx = 0; idx < Kwave::toInt(count); idx++) {
                snd_pcm_info_set_subdevice(pcminfo, idx);
                if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
                    qWarning("ctrl digital audio playback info "
                             "(%i): %s", card, snd_strerror(err));
                    continue;
                }
                QString hwdev = hw_device + _(",%1").arg(idx);
                QString subdevice_name =
                    _(snd_pcm_info_get_subdevice_name(pcminfo));
                QString full_name = QString(
                    i18n("Card %1: ", card) + card_name +
                    _("|sound_card||") +
                    i18n("Device %1: ", dev) + device_name +
                    _("|sound_device||") +
                    i18n("Subdevice %1: ", idx) + subdevice_name +
                    _("|sound_subdevice")
                );
                m_device_list.insert(full_name, hwdev);
            }
        }
        snd_ctl_close(handle);
next_card:
        if (snd_card_next(&card) < 0) {
            qWarning("snd_card_next failed");
            break;
        }
    }

    // 2. scan logical/virtual pcm devices under a dedicated
    //    sub-tree, skipping anything that isn't actually usable
    //    for playback
    void **hints = nullptr;
    if ((snd_device_name_hint(-1, "pcm", &hints) == 0) && hints) {
        for (void **h = hints; *h != nullptr; ++h) {
            char *name_raw = snd_device_name_get_hint(*h, "NAME");
            char *desc_raw = snd_device_name_get_hint(*h, "DESC");
            char *ioid_raw = snd_device_name_get_hint(*h, "IOID");

            const QString device_name = name_raw ?
                QString::fromLocal8Bit(name_raw) : QString();
            const QString description = desc_raw ?
                QString::fromLocal8Bit(desc_raw).trimmed() :
                QString();
            const QString io_direction = ioid_raw ?
                QString::fromLocal8Bit(ioid_raw) : QString();

            if (name_raw) free(name_raw);
            if (desc_raw) free(desc_raw);
            if (ioid_raw) free(ioid_raw);

            // skip pure capture-only devices
            if (io_direction == _("Input"))
                continue;

            if (device_name.isEmpty())
                continue;

            if (!isRelevantPlaybackHint(device_name))
                continue;

            // opens fine but not really usable? skip it, so we
            // never offer a device that would fail once selected
            if (!probePlaybackDevice(device_name))
                continue;

            QString label = virtualPlaybackLabel(description,
                                                 device_name);

            // place virtual devices in a separate subtree node
            QString full_name =
                i18n("Virtual / ALSA Plugins") +
                _("|sound_card||") +
                label +
                _("|sound_device");
            m_device_list.insert(full_name, device_name);
        }

        snd_device_name_free_hint(hints);
    }

    m_device_list.insert(NULL_DEVICE, _("null"));
}

//***************************************************************************
QString Kwave::PlayBackALSA::alsaDeviceName(const QString &name)
{
    if (m_device_list.isEmpty() || (name.length() &&
        !m_device_list.contains(name)))
    {
//      qDebug("### RESCAN ### (list.count=%d, name='%s')",
//             m_device_list.count(), name.data());
        scanDevices();
    }

    if (!m_device_list.contains(name)) {
        // maybe we already have a ALSA compatible name (like in init state)
        for (QMap<QString, QString>::const_iterator
             it(m_device_list.constBegin()); it != m_device_list.constEnd();
             ++it)
        {
            const QString &n = it.value();
            if (n == name) return n;
        }

        qWarning("PlayBackALSA::alsaDeviceName('%s') - NOT FOUND", DBG(name));
        return _("");
    }
    return m_device_list[name];
}

//***************************************************************************
QStringList Kwave::PlayBackALSA::supportedDevices()
{
    // re-validate the list if necessary
    scanDevices();

    QStringList list = m_device_list.keys();

    // move "default" or "null" to the start of the list
    if (list.contains(NULL_DEVICE))
        list.move(list.indexOf(NULL_DEVICE), 0);

    if (!list.isEmpty()) list.append(_("#TREE#"));

    return list;
}

//***************************************************************************
QString Kwave::PlayBackALSA::fileFilter()
{
    return _("");
}

//***************************************************************************
snd_pcm_t *Kwave::PlayBackALSA::openDevice(const QString &device)
{
    snd_pcm_t *pcm = m_handle;

//     qDebug("PlayBackALSA::openDevice(%s)", device.local8Bit().data());

    // translate verbose name to internal ALSA name
    QString alsa_device = alsaDeviceName(device);

    if (!alsa_device.length()) return nullptr;

    // workaround for bug in ALSA
    // if the device name ends with "," -> invalid name
    if (alsa_device.endsWith(_(","))) return nullptr;

    if (!pcm) {
        // open the device in case it's not already open
        int err = snd_pcm_open(&pcm, alsa_device.toLocal8Bit().data(),
                               SND_PCM_STREAM_PLAYBACK,
                               SND_PCM_NONBLOCK);
        if (err < 0) {
            pcm = nullptr;
            qWarning("PlayBackALSA::openDevice('%s') - failed, err=%d (%s)",
                     DBG(alsa_device), err, snd_strerror(err));
        }
    }

    return pcm;
}

//***************************************************************************
QList<unsigned int> Kwave::PlayBackALSA::supportedBits(const QString &device)
{
    detectSupportedFormats(device);

    // try all known sample formats
    QList<unsigned int> list;
    for (const snd_pcm_format_t &fmt : m_supported_formats) {
        const unsigned int bits = snd_pcm_format_width(fmt);

        // 0  bits means invalid/does not apply
        if (!bits) continue;

        // do not produce duplicates
        if (list.contains(bits)) continue;

        list.append(bits);
    }
    return list;
}

//***************************************************************************
int Kwave::PlayBackALSA::detectChannels(const QString &device,
                                        unsigned int &min, unsigned int &max)
{
    min = max = 0;
    ALSA_MALLOC_WRAPPER(snd_pcm_hw_params) p;

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
