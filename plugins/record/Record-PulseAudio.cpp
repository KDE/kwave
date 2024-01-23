/*************************************************************************
  Record-PulseAudio.cpp  -  device for audio recording via PulesAudio
                             -------------------
    begin                : Sun Okt 20 2013
    copyright            : (C) 2014 by Joerg-Christian Boehme
    email                : joerg@chaosdorf.de
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
#ifdef HAVE_PULSEAUDIO_SUPPORT

#include <errno.h>
#include <signal.h>
#include <unistd.h>

#include <limits>

#include <pulse/thread-mainloop.h>

#include <QApplication>
#include <QCursor>
#include <QFileInfo>
#include <QLatin1Char>
#include <QLocale>
#include <QString>
#include <QVariant>
#include <QtGlobal>

#include <KLocalizedString>
#include <KUser>

#include "libkwave/Compression.h"
#include "libkwave/SampleFormat.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"
#include "libkwave/memcpy.h"

#include "Record-PulseAudio.h"

/** helper macro: returns the number of elements in an array */
#define ELEMENTS_OF(__array__) (sizeof(__array__) / sizeof(__array__[0]))

/**
 * timeout for the device scan [ms]
 * @see scanDevices()
 */
#define TIMEOUT_WAIT_DEVICE_SCAN 10000

/**
 * timeout to wait for the connection to the server [ms]
 * @see connectToServer()
 */
#define TIMEOUT_CONNECT_TO_SERVER 20000

/**
 * timeout to wait for record [ms]
 * @see open()
 */
#define TIMEOUT_CONNECT_RECORD 10000

/**
 * timeout to wait for disconnecting the recording stream [ms]
 * @see close()
 */
#define TIMEOUT_DISCONNECT_STREAM 10000

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
static const pa_sample_format_t _known_formats[] =
{
    /* 8 bit */
    PA_SAMPLE_U8,

    /* 16 bit */
    PA_SAMPLE_S16LE, PA_SAMPLE_S16BE,

    /* 24 bit */
    PA_SAMPLE_S24LE, PA_SAMPLE_S24BE,

    /* 24 bit in LSB of 32 bit */
    PA_SAMPLE_S24_32LE, PA_SAMPLE_S24_32BE,

    /* 32 bit */
    PA_SAMPLE_S32LE, PA_SAMPLE_S32BE,

    /* float 32 bit */
    PA_SAMPLE_FLOAT32LE, PA_SAMPLE_FLOAT32BE,

    /* ULAW */
    PA_SAMPLE_ULAW,

    /* ALAW */
    PA_SAMPLE_ALAW
};

//***************************************************************************
/** find out the SampleFormat of an PulseAudio format */
static Kwave::SampleFormat::Format sample_format_of(pa_sample_format_t fmt)
{
    Kwave::SampleFormat::Format sampleFormat = Kwave::SampleFormat::Unknown;
    switch (fmt) {
        case PA_SAMPLE_FLOAT32LE: /* FALLTHROUGH */
        case PA_SAMPLE_FLOAT32BE:
            sampleFormat = Kwave::SampleFormat::Float;
            break;
        case PA_SAMPLE_U8:
            sampleFormat = Kwave::SampleFormat::Unsigned;
            break;
        default:
            sampleFormat = Kwave::SampleFormat::Signed;
            break;
    }
    return sampleFormat;
}

//***************************************************************************
/** find out the endianness of an PulseAudio format */
static Kwave::byte_order_t endian_of(pa_sample_format_t fmt)
{
    if (pa_sample_format_is_le(fmt) == 1)
        return Kwave::LittleEndian;
    if (pa_sample_format_is_be(fmt) == 1)
        return Kwave::BigEndian;
    return Kwave::CpuEndian;
}

//***************************************************************************
static Kwave::Compression::Type compression_of(pa_sample_format_t fmt)
{
    Kwave::Compression::Type compression = Kwave::Compression::NONE;
    switch (fmt) {
        case PA_SAMPLE_ULAW:
            compression = Kwave::Compression::G711_ULAW;
            break;
        case PA_SAMPLE_ALAW:
            compression = Kwave::Compression::G711_ALAW;
            break;
        default:
            compression = Kwave::Compression::NONE;
            break;
    }
    return compression;
}

//***************************************************************************
static int bits_of(pa_sample_format_t fmt)
{
    int bits = 0;
    switch (fmt) {
        case PA_SAMPLE_ULAW:    /* FALLTHROUGH */
        case PA_SAMPLE_ALAW:    /* FALLTHROUGH */
        case PA_SAMPLE_U8:
            bits = 8;
            break;
        case PA_SAMPLE_S16LE:    /* FALLTHROUGH */
        case PA_SAMPLE_S16BE:
            bits = 16;
            break;
        case PA_SAMPLE_S24LE:    /* FALLTHROUGH */
        case PA_SAMPLE_S24BE:    /* FALLTHROUGH */
        case PA_SAMPLE_S24_32LE: /* FALLTHROUGH */
        case PA_SAMPLE_S24_32BE:
            bits = 24;
            break;
        case PA_SAMPLE_S32LE:     /* FALLTHROUGH */
        case PA_SAMPLE_S32BE:     /* FALLTHROUGH */
        case PA_SAMPLE_FLOAT32LE: /* FALLTHROUGH */
        case PA_SAMPLE_FLOAT32BE:
            bits = 32;
            break;
        default:
            bits = 0;
            break;
    }
    return bits;
}

//***************************************************************************
Kwave::RecordPulseAudio::RecordPulseAudio()
    :Kwave::RecordDevice(),
    m_mainloop_thread(this, QVariant()),
    m_mainloop_lock(),
    m_mainloop_signal(),
    m_sample_format(Kwave::SampleFormat::Unknown),
    m_tracks(0),
    m_rate(0.0),
    m_compression(Kwave::Compression::NONE),
    m_bits_per_sample(0),
    m_supported_formats(),
    m_initialized(false),
    m_pa_proplist(Q_NULLPTR),
    m_pa_mainloop(Q_NULLPTR),
    m_pa_context(Q_NULLPTR),
    m_pa_stream(Q_NULLPTR),
    m_pa_device(),
    m_name(i18n("Kwave record")),
    m_device_list()
{
}

//***************************************************************************
Kwave::RecordPulseAudio::~RecordPulseAudio()
{
    disconnectFromServer();
    m_device_list.clear();
}

//***************************************************************************
void Kwave::RecordPulseAudio::detectSupportedFormats(const QString &device)
{
    // start with an empty list
    m_supported_formats.clear();

    // lookup in the device list
    if (!m_device_list.contains(device))
        return;

    const pa_sample_spec     &sampleSpec = m_device_list[device].m_sample_spec;
    const pa_sample_format_t &formatSpec = sampleSpec.format;

    // try all known formats
    qDebug("--- list of supported formats --- ");
    for(unsigned int i = 0; i < ELEMENTS_OF(_known_formats); ++i) {
        const pa_sample_format_t &fmt = _known_formats[i];

        if (formatSpec < _known_formats[i])
            continue;

        // TODO: avoid duplicate entries that differ only in endianness,
        //       prefer our own (native) endianness

        Kwave::Compression t(compression_of(fmt));
        Kwave::SampleFormat::Map sf;
        qDebug("#%2u, %2u bit [%u byte], %s, '%s', '%s'",
            i,
            bits_of(fmt),
            (bits_of(fmt) + 7) >> 3,
            endian_of(fmt) == Kwave::CpuEndian ? "CPU" :
            (endian_of(fmt) == Kwave::LittleEndian ? "LE " : "BE "),
            DBG(sf.description(sf.findFromData(sample_format_of(fmt)), true)),
            DBG(t.name())
        );

        m_supported_formats.append(fmt);
    }
    qDebug("--------------------------------- ");
}

//***************************************************************************
void Kwave::RecordPulseAudio::pa_read_cb(pa_stream *p, size_t nbytes,
                                         void *userdata)
{
    Kwave::RecordPulseAudio *record_plugin =
        reinterpret_cast<Kwave::RecordPulseAudio *>(userdata);
    Q_ASSERT(record_plugin);
    if (record_plugin) record_plugin->notifyRead(p, nbytes);
}

//***************************************************************************
void Kwave::RecordPulseAudio::notifyRead(pa_stream *stream, size_t nbytes)
{
    Q_UNUSED(nbytes)
    Q_ASSERT(stream);

    if (!stream || (stream != m_pa_stream)) return;

    m_mainloop_signal.wakeAll();
}

//***************************************************************************
void Kwave::RecordPulseAudio::pa_stream_state_cb(pa_stream *p, void *userdata)
{
    Kwave::RecordPulseAudio *record_plugin =
        reinterpret_cast<Kwave::RecordPulseAudio *>(userdata);
    Q_ASSERT(record_plugin);
    if (record_plugin) record_plugin->notifyStreamState(p);
}

//***************************************************************************
void Kwave::RecordPulseAudio::notifyStreamState(pa_stream* stream)
{
    Q_ASSERT(stream);
    if (!stream || (stream != m_pa_stream)) return;

    pa_stream_state_t state = pa_stream_get_state(stream);

#ifdef DEBUG
    #define DBG_CASE(x) case x: qDebug("RecordPulseAudio -> " #x ); break
    switch (state)
    {
        DBG_CASE(PA_STREAM_CREATING);
        DBG_CASE(PA_STREAM_UNCONNECTED);
        DBG_CASE(PA_STREAM_FAILED);
        DBG_CASE(PA_STREAM_TERMINATED);
        DBG_CASE(PA_STREAM_READY);
    }
    #undef DBG_CASE
#endif /* DEBUG */

    switch (state) {
        case PA_STREAM_CREATING:
            break;
        case PA_STREAM_UNCONNECTED: /* FALLTHROUGH */
        case PA_STREAM_FAILED:      /* FALLTHROUGH */
        case PA_STREAM_TERMINATED:  /* FALLTHROUGH */
        case PA_STREAM_READY:
            m_mainloop_signal.wakeAll();
            break;
        default:
            Q_ASSERT(0 && "?");
            break;
    }
}

//***************************************************************************
void Kwave::RecordPulseAudio::pa_context_notify_cb(pa_context* c, void* userdata)
{
    Kwave::RecordPulseAudio *record_plugin =
        reinterpret_cast<Kwave::RecordPulseAudio *>(userdata);
    Q_ASSERT(record_plugin);
    if (record_plugin) record_plugin->notifyContext(c);
}

//***************************************************************************
void Kwave::RecordPulseAudio::notifyContext(pa_context *c)
{
    Q_ASSERT(c == m_pa_context);
    const pa_context_state_t state = pa_context_get_state(c);

#ifdef DEBUG
    #define DBG_CASE(x) case x: qDebug("RecordPulseAudio -> " #x ); break
    switch (state)
    {
        DBG_CASE(PA_CONTEXT_UNCONNECTED);
        DBG_CASE(PA_CONTEXT_CONNECTING);
        DBG_CASE(PA_CONTEXT_AUTHORIZING);
        DBG_CASE(PA_CONTEXT_SETTING_NAME);
        DBG_CASE(PA_CONTEXT_READY);
        DBG_CASE(PA_CONTEXT_TERMINATED);
        DBG_CASE(PA_CONTEXT_FAILED);
    }
    #undef DBG_CASE
#endif /* DEBUG */

    switch (state)
    {
        case PA_CONTEXT_UNCONNECTED: /* FALLTHROUGH */
        case PA_CONTEXT_CONNECTING:  /* FALLTHROUGH */
        case PA_CONTEXT_AUTHORIZING: /* FALLTHROUGH */
        case PA_CONTEXT_SETTING_NAME:
            break;
        case PA_CONTEXT_READY:       /* FALLTHROUGH */
        case PA_CONTEXT_TERMINATED:  /* FALLTHROUGH */
        case PA_CONTEXT_FAILED:
            m_mainloop_signal.wakeAll();
            break;
    }
}

//***************************************************************************
pa_sample_format_t Kwave::RecordPulseAudio::mode2format(
    int compression, int bits, Kwave::SampleFormat::Format sample_format)
{
    // loop over all supported formats and keep only those that are
    // compatible with the given compression, bits and sample format
    foreach (const pa_sample_format_t &fmt, m_supported_formats)
    {
        if (compression_of(fmt) != compression) continue;
        if (bits_of(fmt) != bits) continue;
        if (!(sample_format_of(fmt) == sample_format)) continue;

        // mode is compatible
        // As the list of known formats is already sorted so that
        // the simplest formats come first, we don't have a lot
        // of work -> just take the first entry ;-)
        return fmt;
    }

    qWarning("RecordPulesAudio::mode2format -> no match found !?");
    return PA_SAMPLE_INVALID;
}

//***************************************************************************
Kwave::byte_order_t Kwave::RecordPulseAudio::endianness()
{
    pa_sample_format_t fmt = mode2format(m_compression, m_bits_per_sample,
                                         m_sample_format);
    return (fmt != PA_SAMPLE_INVALID) ?
        endian_of(fmt) : Kwave::UnknownEndian;
}

//***************************************************************************
Kwave::SampleFormat::Format Kwave::RecordPulseAudio::sampleFormat()
{
    return m_sample_format;
}

//***************************************************************************
int Kwave::RecordPulseAudio::setSampleFormat(
    Kwave::SampleFormat::Format new_format)
{
    if (m_sample_format == new_format)
        return 0;
    close();
    m_sample_format = new_format;
    return 0;
}

//***************************************************************************
QList<Kwave::SampleFormat::Format> Kwave::RecordPulseAudio::detectSampleFormats()
{
    QList<Kwave::SampleFormat::Format> list;

    // try all known sample formats
    foreach (const pa_sample_format_t &fmt, m_supported_formats)
    {
        const Kwave::SampleFormat::Format sample_format = sample_format_of(fmt);

        // only accept bits/sample if compression types
        // and bits per sample match
        if (compression_of(fmt) != m_compression) continue;
        if (bits_of(fmt) != Kwave::toInt(m_bits_per_sample))
            continue;

        // do not produce duplicates
        if (list.contains(sample_format)) continue;

        list.append(sample_format);
    }

    return list;
}

//***************************************************************************
int Kwave::RecordPulseAudio::bitsPerSample()
{
    return m_bits_per_sample;
}

//***************************************************************************
int Kwave::RecordPulseAudio::setBitsPerSample(unsigned int new_bits)
{
    if (m_bits_per_sample == new_bits)
        return 0;
    close();
    m_bits_per_sample = new_bits;
    return 0;
}

//***************************************************************************
QList< unsigned int > Kwave::RecordPulseAudio::supportedBits()
{
    QList<unsigned int> list;

    // try all known sample formats
    foreach(const pa_sample_format_t &fmt, m_supported_formats)
    {
        const unsigned int bits = bits_of(fmt);

        // 0  bits means invalid/does not apply
        if (!bits) continue;

        // only accept bits/sample if compression matches
        if (compression_of(fmt) != m_compression) continue;

        // do not produce duplicates
        if (list.contains(bits)) continue;

        list.append(bits);
    }

    return list;
}

//***************************************************************************
Kwave::Compression::Type Kwave::RecordPulseAudio::compression()
{
    return m_compression;
}

//***************************************************************************
int Kwave::RecordPulseAudio::setCompression(
    Kwave::Compression::Type new_compression
)
{
    if (m_compression != new_compression) {
        close();
        m_compression = new_compression;
    }
    return 0;
}

//***************************************************************************
QList<Kwave::Compression::Type> Kwave::RecordPulseAudio::detectCompressions()
{
    QList<Kwave::Compression::Type> list;

    // try all known sample formats
    foreach (const pa_sample_format_t &fmt, m_supported_formats)
    {
        Kwave::Compression::Type compression = compression_of(fmt);

        // do not produce duplicates
        if (list.contains(compression)) continue;

        Kwave::Compression t(compression);
        list.append(compression);
    }

    return list;
}

//***************************************************************************
double Kwave::RecordPulseAudio::sampleRate()
{
    return m_rate;
}

//***************************************************************************
int Kwave::RecordPulseAudio::setSampleRate(double& new_rate)
{
    if (qFuzzyCompare(new_rate, m_rate))
        return 0;
    close();
    m_rate = new_rate;
    return 0;
}

//***************************************************************************
QList< double > Kwave::RecordPulseAudio::detectSampleRates()
{
    QList<double> list;

    static const unsigned int known_rates[] = {
          1,
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
        192000 // AC97
    };

    pa_sample_spec sampleSpec = m_device_list[m_device].m_sample_spec;
    uint32_t rate = sampleSpec.rate;
    for (unsigned int i = 0; i < ELEMENTS_OF(known_rates); i++) {
        if(known_rates[i] <= rate) {
            list.append(known_rates[i]);
        }
    }

    return list;
}

//***************************************************************************
int Kwave::RecordPulseAudio::tracks()
{
    return m_tracks;
}

//***************************************************************************
int Kwave::RecordPulseAudio::setTracks(unsigned int &tracks)
{
    const quint8 max_tracks = std::numeric_limits<quint8>::max();

    if (tracks > max_tracks) {
        tracks = max_tracks;
        return -1;
    }

    if (tracks == m_tracks)
        return 0;

    close();
    m_tracks = static_cast<quint8>(tracks);

    return 0;
}

//***************************************************************************
int Kwave::RecordPulseAudio::detectTracks(unsigned int &min, unsigned int &max)
{
    pa_sample_spec sampleSpec = m_device_list[m_device].m_sample_spec;
    unsigned int channels = sampleSpec.channels;

    min = 1;
    max = qBound<unsigned int>(min, channels, PA_CHANNELS_MAX);
    return 0;
}

//***************************************************************************
int Kwave::RecordPulseAudio::close()
{
    if (m_pa_stream) {
        pa_stream_drop(m_pa_stream);

        m_mainloop_lock.lock();
        pa_stream_disconnect(m_pa_stream);
        qDebug("RecordPulseAudio::close() - waiting for stream disconnect...");
        m_mainloop_signal.wait(&m_mainloop_lock, TIMEOUT_DISCONNECT_STREAM);
        m_mainloop_lock.unlock();
        qDebug("RecordPulseAudio::close() - stream disconnect DONE");

        pa_stream_unref(m_pa_stream);
    }
    m_pa_stream = Q_NULLPTR;

    // we need to re-initialize the next time
    m_initialized = false;
    return 0;
}

//***************************************************************************
int Kwave::RecordPulseAudio::read(QByteArray& buffer, unsigned int offset)
{
    if (buffer.isNull() || buffer.isEmpty())
        return 0; // no buffer, nothing to do

    unsigned int length = buffer.size();

    // we configure our device at a late stage, not on the fly like in OSS
    if (!m_initialized) {
        int err = initialize(length);
        if (err < 0) return err;
    }

    m_mainloop_lock.lock();

    size_t freeBytes = length - offset;
    size_t readableSize = pa_stream_readable_size(m_pa_stream);
    if (readableSize > freeBytes) {
        size_t additional_size = readableSize - freeBytes;
        buffer.resize(static_cast<int>(length + additional_size));
    }

    size_t readLength = 0;
    if (readableSize > 0) {
        const void *audioBuffer = Q_NULLPTR;
        pa_stream_peek(m_pa_stream, &audioBuffer, &readLength);

        if (offset + readLength > Kwave::toUint(buffer.length())) {
            pa_stream_drop(m_pa_stream);
            m_mainloop_lock.unlock();
            return -EIO; // peek returned invalid length
        }

        char *data = buffer.data() + offset;
        if (audioBuffer) {
            MEMCPY(data, audioBuffer, readLength); // real data
        } else {
            memset(data, 0x00, readLength); // there was a gap
        }

        pa_stream_drop(m_pa_stream);

    } else {
        m_mainloop_lock.unlock();
        return -EAGAIN;
    }
    m_mainloop_lock.unlock();

    return Kwave::toInt(readLength);
}

//***************************************************************************
int Kwave::RecordPulseAudio::initialize(uint32_t buffer_size)
{
    Q_ASSERT(!m_initialized);

    // make sure that we are connected to the sound server
    if (!connectToServer()) {
        qWarning("Connecting to the PulseAudio server failed!");
        return -1;
    }

    pa_sample_format_t fmt = mode2format(m_compression, m_bits_per_sample,
                                         m_sample_format);
    if (fmt == PA_SAMPLE_INVALID) {
        Kwave::SampleFormat::Map sf;

        qWarning("format: no matching format for compression '%s', "
            "%d bits/sample, format '%s'",
            DBG(sf.description(sf.findFromData(m_sample_format), true)),
            m_bits_per_sample,
            DBG(Kwave::Compression(m_compression).name())
        );
        return -EINVAL;
    }

    pa_sample_spec sample_spec;
    sample_spec.channels = m_tracks;
    sample_spec.format = fmt;
    sample_spec.rate = static_cast<quint32>(m_rate);

    if(!pa_sample_spec_valid(&sample_spec)) {
        Kwave::SampleFormat::Map sf;

        qWarning("no valid pulse audio format: %d, rate: %0.3g, channels: %d",
                 static_cast<int>(fmt), m_rate, m_tracks);
        return -EINVAL;
    }

    // run with mainloop locked from here on...
    m_mainloop_lock.lock();

    pa_channel_map channel_map;
    pa_channel_map_init_extend(&channel_map, sample_spec.channels,
                               PA_CHANNEL_MAP_DEFAULT);

    if (!pa_channel_map_compatible(&channel_map, &sample_spec)) {
        qWarning("Channel map doesn't match sample specification!");
    }

    // create a new stream
    m_pa_stream = pa_stream_new(
        m_pa_context,
        m_name.toUtf8().constData(),
        &sample_spec,
        &channel_map);

    if (!m_pa_stream) {
        m_mainloop_lock.unlock();
        qWarning("Failed to create a PulseAudio stream %s",
                 pa_strerror(pa_context_errno(m_pa_context)));
        return -1;
    }

    pa_stream_set_state_callback(m_pa_stream, pa_stream_state_cb, this);
    pa_stream_set_read_callback(m_pa_stream, pa_read_cb, this);

    pa_buffer_attr attr;
    attr.fragsize  = buffer_size;
    attr.maxlength = static_cast<uint32_t>(-1);
    attr.minreq    = static_cast<uint32_t>(-1);
    attr.prebuf    = static_cast<uint32_t>(-1);
    attr.tlength   = static_cast<uint32_t>(-1);
    int flags      = PA_STREAM_ADJUST_LATENCY;

    // connect the stream in record mode
    int result = pa_stream_connect_record(
        m_pa_stream,
        m_pa_device.toUtf8().constData(),
        &attr,
        static_cast<pa_stream_flags_t>(flags));

    if (result >= 0) {
        m_mainloop_signal.wait(&m_mainloop_lock, TIMEOUT_CONNECT_RECORD);
        if (pa_stream_get_state(m_pa_stream) != PA_STREAM_READY)
            result = -1;
    }

    m_mainloop_lock.unlock();

    if (result < 0) {
        pa_stream_unref(m_pa_stream);
        m_pa_stream = Q_NULLPTR;
        qWarning("Failed to open a PulseAudio stream for record %s",
                 pa_strerror(pa_context_errno(m_pa_context)));
        return -1;
    }

    m_initialized = true;
    return 0;
}

//***************************************************************************
QString Kwave::RecordPulseAudio::open(const QString& device)
{
    // close the previous device
    if (m_pa_stream) close();

    QString pa_device;
    if (m_device_list.contains(device))
        pa_device = m_device_list[device].m_name;

    if (!pa_device.length())
        return QString::number(ENODEV);

    m_pa_device = pa_device;
    m_device    = device;

    // detect all formats the device knows
    detectSupportedFormats(device);

    return QString();
}

//***************************************************************************
QStringList Kwave::RecordPulseAudio::supportedDevices()
{
    QStringList list;

    // re-validate the list if necessary
    scanDevices();

    if (!m_pa_mainloop || !m_pa_context) return list;

    list = m_device_list.keys();
    if (!list.isEmpty()) list.prepend(_("#TREE#"));

    return list;
}

//***************************************************************************
void Kwave::RecordPulseAudio::run_wrapper(const QVariant &params)
{
    Q_UNUSED(params)
    m_mainloop_lock.lock();
    pa_mainloop_run(m_pa_mainloop, Q_NULLPTR);
    m_mainloop_lock.unlock();
    qDebug("RecordPulseAudio::run_wrapper - done.");
}

//***************************************************************************
static int poll_func(struct pollfd *ufds, unsigned long nfds,
                     int timeout, void *userdata)
{
    Kwave::RecordPulseAudio *dev =
        static_cast<Kwave::RecordPulseAudio *>(userdata);
    Q_ASSERT(dev);
    if (!dev) return -1;

    return dev->mainloopPoll(ufds, nfds, timeout);
}

//***************************************************************************
int Kwave::RecordPulseAudio::mainloopPoll(struct pollfd *ufds,
                                            unsigned long int nfds,
                                            int timeout)
{
    m_mainloop_lock.unlock();
    int retval = poll(ufds, nfds, timeout);
    m_mainloop_lock.lock();

    return retval;
}

//***************************************************************************
bool Kwave::RecordPulseAudio::connectToServer()
{
    if (m_pa_context) return true; // already connected

    // set hourglass cursor, we are waiting...
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    // create a property list for this application
    m_pa_proplist = pa_proplist_new();
    Q_ASSERT(m_pa_proplist);

    pa_proplist_sets(m_pa_proplist, PA_PROP_APPLICATION_LANGUAGE,
                     UTF8(QLocale::system().name()));
    pa_proplist_sets(m_pa_proplist, PA_PROP_APPLICATION_NAME,
                     UTF8(qApp->applicationName()));
    pa_proplist_sets(m_pa_proplist, PA_PROP_APPLICATION_ICON_NAME,
                     "kwave");
    pa_proplist_sets(m_pa_proplist, PA_PROP_APPLICATION_PROCESS_BINARY,
                     "kwave");
    pa_proplist_setf(m_pa_proplist, PA_PROP_APPLICATION_PROCESS_ID,
                    "%ld", static_cast<long int>(qApp->applicationPid()));
    KUser user;
    pa_proplist_sets(m_pa_proplist, PA_PROP_APPLICATION_PROCESS_USER,
                     UTF8(user.loginName()));
    pa_proplist_sets(m_pa_proplist, PA_PROP_APPLICATION_VERSION,
                     UTF8(qApp->applicationVersion()));

    pa_proplist_sets(m_pa_proplist, PA_PROP_MEDIA_ROLE, "production");

    // ignore SIGPIPE in this context
#ifdef HAVE_SIGNAL_H
    signal(SIGPIPE, SIG_IGN);
#endif

    m_pa_mainloop = pa_mainloop_new();
    Q_ASSERT(m_pa_mainloop);
    pa_mainloop_set_poll_func(m_pa_mainloop, poll_func, this);

    m_pa_context = pa_context_new_with_proplist(
        pa_mainloop_get_api(m_pa_mainloop),
        "Kwave",
        m_pa_proplist
    );

    // set the callback for getting informed about the context state
    pa_context_set_state_callback(m_pa_context, pa_context_notify_cb, this);

    // connect to the pulse audio server server
    bool failed = false;
    int error = pa_context_connect(
        m_pa_context,                       // context
        Q_NULLPTR,                          // server
        static_cast<pa_context_flags_t>(0), // flags
        Q_NULLPTR                           // API
    );
    if (error < 0)
    {
        qWarning("RecordPulseAudio: pa_contect_connect failed (%s)",
                  pa_strerror(pa_context_errno(m_pa_context)));
        failed = true;
    }

    if (!failed) {
        m_mainloop_lock.lock();
        m_mainloop_thread.start();

        // wait until the context state is either connected or failed
        failed = true;
        if ( m_mainloop_signal.wait(&m_mainloop_lock,
                                    TIMEOUT_CONNECT_TO_SERVER) )
        {
            if (pa_context_get_state(m_pa_context) == PA_CONTEXT_READY) {
                failed = false;
            }
        }
        m_mainloop_lock.unlock();

        if (failed) {
            qWarning("RecordPulseAudio: context FAILED (%s):-(",
                     pa_strerror(pa_context_errno(m_pa_context)));
        }
    }

    // if the connection failed, clean up
    if (failed) {
        disconnectFromServer();
    }

    QApplication::restoreOverrideCursor();

    return !failed;
}

//***************************************************************************
void Kwave::RecordPulseAudio::disconnectFromServer()
{
    close();

    // stop the main loop
    m_mainloop_thread.isInterruptionRequested();
    if (m_pa_mainloop) {
        m_mainloop_lock.lock();
        pa_mainloop_quit(m_pa_mainloop, 0);
        m_mainloop_lock.unlock();
    }
    m_mainloop_thread.stop();

    // disconnect the pulse context
    if (m_pa_context) {
        pa_context_disconnect(m_pa_context);
        pa_context_unref(m_pa_context);
        m_pa_context  = Q_NULLPTR;
    }

    // stop and free the main loop
    if (m_pa_mainloop) {
        pa_mainloop_free(m_pa_mainloop);
        m_pa_mainloop = Q_NULLPTR;
    }

    // release the property list
    if (m_pa_proplist) {
        pa_proplist_free(m_pa_proplist);
        m_pa_proplist = Q_NULLPTR;
    }

}

//***************************************************************************
void Kwave::RecordPulseAudio::pa_source_info_cb(pa_context *c,
                                                const pa_source_info *info,
                                                int eol, void *userdata)
{
    Kwave::RecordPulseAudio *record_plugin =
        reinterpret_cast<Kwave::RecordPulseAudio *>(userdata);
    Q_ASSERT(record_plugin);
    if (record_plugin) record_plugin->notifySourceInfo(c, info, eol);
}

//***************************************************************************
void Kwave::RecordPulseAudio::notifySourceInfo(pa_context *c,
                                               const pa_source_info *info,
                                               int eol)
{
    Q_UNUSED(c)
    Q_ASSERT(c == m_pa_context);

    if (eol == 0) {
        source_info_t i;
        i.m_name        = QString::fromUtf8(info->name);
        i.m_description = QString::fromUtf8(info->description);
        i.m_driver      = QString::fromUtf8(info->driver);
        i.m_card        = info->card;
        i.m_sample_spec = info->sample_spec;

        QString name    = QString::number(m_device_list.count());
        m_device_list[name] = i;
    } else {
        m_mainloop_signal.wakeAll();
    }
}

//***************************************************************************
void Kwave::RecordPulseAudio::scanDevices()
{
    if (!m_pa_context) connectToServer();
    if (!m_pa_context) return;

    // fetch the device list from the PulseAudio server
    m_mainloop_lock.lock();
    m_device_list.clear();
    const pa_operation *op_source_info = pa_context_get_source_info_list(
        m_pa_context,
        pa_source_info_cb,
        this
    );
    if (op_source_info) {
        // set hourglass cursor, we have a long timeout...
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        m_mainloop_signal.wait(&m_mainloop_lock, TIMEOUT_WAIT_DEVICE_SCAN);
        QApplication::restoreOverrideCursor();
    }

    // create a list with final names
    QMap<QString, source_info_t> list;
    for (QMap<QString, source_info_t>::const_iterator it =
         m_device_list.constBegin();
         it != m_device_list.constEnd(); ++it)
    {
        const QString       &source     = it.key();
        const source_info_t &inf        = it.value();
        const QString       &name       = inf.m_name;
        QString             description = inf.m_description;
        QString             driver      = inf.m_driver;

        // if the name is not unique, add the internal source name
        for (QMap<QString, source_info_t>::const_iterator it2 =
             m_device_list.constBegin();
             it2 != m_device_list.constEnd(); ++it2)
        {
            const QString       &s = it2.key();
            const source_info_t &i = it2.value();
            if (s == source) continue;
            if ((i.m_description == description) &&
                (i.m_driver      == driver))
            {
                // not unique
                description += _(" [") + name + _("]");
                break;
            }
        }

        // mangle the driver name, e.g.
        // "module-alsa-sink.c" -> "alsa sink"
        QFileInfo f(driver);
        driver = f.baseName();
        driver.replace(_("-"), _(" "));
        driver.replace(_("_"), _(" "));
        if (driver.startsWith(_("module "), Qt::CaseInsensitive))
            driver.remove(0, 7);
        description.prepend(driver + _("|sound_card||"));

        // add the leaf node
        if (inf.m_card != PA_INVALID_INDEX)
            description.append(_("|sound_device"));
        else
            description.append(_("|sound_note"));

        list.insert(description, *it);
    }

    m_device_list.clear();
    m_device_list = list;
    m_mainloop_lock.unlock();
}

#endif /* HAVE_PULSEAUDIO_SUPPORT */

//***************************************************************************
//***************************************************************************
