/***************************************************************************
 *    Record-PulseAudio.h  -  device for audio recording via PulesAudio
 *                             -------------------
 *    begin                : Sun Okt 20 2013
 *    copyright            : (C) 2005 by Thomas Eschenbacher
 *    email                : Thomas.Eschenbacher@gmx.de
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

#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include <pulse/thread-mainloop.h>

#include <QtGui/QApplication>
#include <QtGui/QCursor>
#include <QtCore/QFileInfo>
#include <QtCore/QLatin1Char>
#include <QtCore/QLocale>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QtGlobal>

#include <klocale.h>
#include <kuser.h>
#include <kdebug.h>

#include "libkwave/Compression.h"
#include "libkwave/String.h"

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
static QString sampleFormatToString(pa_sample_format format)
{
/*    switch (format)
    {
	case PA_SAMPLE_U8:          return "Unsigned 8 Bit PCM.";
	case PA_SAMPLE_ALAW:        return "8 Bit a-Law ";
	case PA_SAMPLE_ULAW:        return "8 Bit mu-Law";
	case PA_SAMPLE_S16LE:       return "Signed 16 Bit PCM, little endian (PC).";
	case PA_SAMPLE_S16BE:       return "Signed 16 Bit PCM, big endian.";
	case PA_SAMPLE_FLOAT32LE:   return "32 Bit IEEE floating point, little endian (PC), range -1.0 to 1.0";
	case PA_SAMPLE_FLOAT32BE:   return "32 Bit IEEE floating point, big endian, range -1.0 to 1.0";
	case PA_SAMPLE_S32LE:       return "Signed 32 Bit PCM, little endian (PC).";
	case PA_SAMPLE_S32BE:       return "Signed 32 Bit PCM, big endian.";
	case PA_SAMPLE_S24LE:       return "Signed 24 Bit PCM packed, little endian (PC).";
	case PA_SAMPLE_S24BE:       return "Signed 24 Bit PCM packed, big endian.";
	case PA_SAMPLE_S24_32LE:    return "Signed 24 Bit PCM in LSB of 32 Bit words, little endian (PC).";
	case PA_SAMPLE_S24_32BE:    return "Signed 24 Bit PCM in LSB of 32 Bit words, big endian.";
	case PA_SAMPLE_MAX:         return "Upper limit of valid sample types.";
	case PA_SAMPLE_INVALID:     return "Invalid sample format";
    }

    return QString("Invalid value: %0").arg(format); */
    return QString(format);
}

//***************************************************************************
/** find out the SampleFormat of an PulseAudio format */
static Kwave::SampleFormat sample_format_of(pa_sample_format_t fmt)
{
    switch (fmt) {
	case PA_SAMPLE_FLOAT32LE:
	case PA_SAMPLE_FLOAT32BE:
	    return Kwave::SampleFormat::Float;
	case PA_SAMPLE_U8:
	    return Kwave::SampleFormat::Unsigned;
	default:
	    return Kwave::SampleFormat::Signed;
    }

    return Kwave::SampleFormat::Unknown;
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
static int compression_of(pa_sample_format_t fmt)
{
    switch (fmt) {
	case PA_SAMPLE_ULAW:
	    return Kwave::Compression::G711_ULAW;
	case PA_SAMPLE_ALAW:
	    return Kwave::Compression::G711_ALAW;
	default:
	    return Kwave::Compression::NONE;
    }
    return Kwave::Compression::NONE;
}

//***************************************************************************
static int bits_of(pa_sample_format_t fmt)
{
    switch (fmt) {
	case PA_SAMPLE_ULAW:
	case PA_SAMPLE_ALAW:
	case PA_SAMPLE_U8:
	    return 8;
	case PA_SAMPLE_S16LE:
	case PA_SAMPLE_S16BE:
	    return 16;
	case PA_SAMPLE_S24LE:
	case PA_SAMPLE_S24BE:
	case PA_SAMPLE_S24_32LE:
	case PA_SAMPLE_S24_32BE:
	    return 24;
	case PA_SAMPLE_S32LE:
	case PA_SAMPLE_S32BE:
	case PA_SAMPLE_FLOAT32LE:
	case PA_SAMPLE_FLOAT32BE:
	    return 32;
	default:
	    return 0;
    }
    return 0;
}

//***************************************************************************
Kwave::RecordPulseAudio::RecordPulseAudio()
    :Kwave::RecordDevice(),
    m_mainloop_thread(this, QVariant()),
    m_mainloop_lock(),
    m_mainloop_signal(),
    m_bytes_per_sample(0),
    m_sample_format(Kwave::SampleFormat::Unknown),
    m_tracks(0),
    m_rate(0.0),
    m_compression(0),
    m_bits_per_sample(0),
    m_supported_formats(),
    m_initialized(false),
    m_pa_proplist(0),
    m_pa_mainloop(0),
    m_pa_context(0),
    m_pa_stream(0),
    m_pa_device(),
    m_name(),
    m_device_list()
{
    QString name = i18n("Kwave record");
    m_name = name.toUtf8();
}

//***************************************************************************
Kwave::RecordPulseAudio::~RecordPulseAudio()
{
    kDebug() << "call ";
    close();
    disconnectFromServer();
    m_device_list.clear();
}

//***************************************************************************
void Kwave::RecordPulseAudio::pa_inputVolume_cb(pa_context *c, int success,
						void *userdata)
{
    Q_UNUSED(c);

    Kwave::RecordPulseAudio *record_plugin =
	reinterpret_cast<Kwave::RecordPulseAudio *>(userdata);
    Q_ASSERT(record_plugin);
    if (record_plugin) record_plugin->notifyInputVolume(c, success,
	               userdata);
}

//***************************************************************************
void Kwave::RecordPulseAudio::notifyInputVolume(pa_context *c, int success,
    void *userdata)
{
    Q_UNUSED(success);

    pa_context_get_source_info_by_index(
	c,
	pa_stream_get_device_index(m_pa_stream),
	pa_source_info_cb,
	userdata
    );
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
/*     kDebug() << " stream=" << static_cast<void *>(stream)
	     << " nbytes=" << nbytes; */
    Q_UNUSED(nbytes);

    Q_ASSERT(stream);
    Q_ASSERT(stream == m_pa_stream);
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
    switch (state) {
	case PA_STREAM_UNCONNECTED:
	    kDebug() << "    -> UNCONNECTED"; break;
	case PA_STREAM_CREATING:
	    kDebug() << "    -> CREATING"; break;
	case PA_STREAM_FAILED:
	    kDebug() << "    -> FAILED";
	case PA_STREAM_TERMINATED:
	    kDebug() << "    -> TERMINATED";
	case PA_STREAM_READY:
	    kDebug() << "    -> READY";
	    m_mainloop_signal.wakeAll();
	    break;
	default:
	    Q_ASSERT(0 && "?");
	    kDebug() << "    -> ???"; break;
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
    switch (pa_context_get_state(c))
    {
	case PA_CONTEXT_UNCONNECTED:
	    kDebug() << "RecordPulseAudio: PA_CONTEXT_UNCONNECTED!?";
	    break;
	case PA_CONTEXT_CONNECTING:
	    kDebug() << "RecordPulseAudio: PA_CONTEXT_CONNECTING...";
	    break;
	case PA_CONTEXT_AUTHORIZING:
	    kDebug() << "RecordPulseAudio: PA_CONTEXT_AUTHORIZING...";
	    break;
	case PA_CONTEXT_SETTING_NAME:
	    kDebug() << "RecordPulseAudio: PA_CONTEXT_SETTING_NAME...";
	    break;
	case PA_CONTEXT_READY:
	    kDebug() << "RecordPulseAudio: PA_CONTEXT_READY.";
	    m_mainloop_signal.wakeAll();
	    break;
	case PA_CONTEXT_TERMINATED:
	    kWarning() << "RecordPulseAudio: PA_CONTEXT_TERMINATED";
	    m_mainloop_signal.wakeAll();
	    break;
	case PA_CONTEXT_FAILED:
	    kWarning() << "RecordPulseAudio: PA_CONTEXT_FAILED";
	    m_mainloop_signal.wakeAll();
	    break;
    }
}

//***************************************************************************
int Kwave::RecordPulseAudio::mode2format(int compression, int bits,
                                   Kwave::SampleFormat sample_format)
{
    // loop over all supported formats and keep only those that are
    // compatible with the given compression, bits and sample format
    foreach (int index, m_supported_formats)
    {
	const pa_sample_format_t *fmt = &_known_formats[index];

	if (compression_of(*fmt) != compression) continue;
	if (bits_of(*fmt) != bits) continue;
	if (!(sample_format_of(*fmt) == sample_format)) continue;

	// mode is compatible
	// As the list of known formats is already sorted so that
	// the simplest formats come first, we don't have a lot
	// of work -> just take the first entry ;-)
	kDebug() << "RecordPulseAudio::mode2format -> " << index;
	return index;
    }

    kWarning() << "RecordPulesAudio::mode2format -> no match found !?";
    return -1;
}

//***************************************************************************
Kwave::byte_order_t Kwave::RecordPulseAudio::endianness()
{
    kDebug() << " call";
    int index = mode2format(m_compression, m_bits_per_sample, m_sample_format);
    return (index >= 0) ?
	endian_of(_known_formats[index]) : Kwave::UnknownEndian;
}

//***************************************************************************
Kwave::SampleFormat Kwave::RecordPulseAudio::sampleFormat()
{
    return m_sample_format;
}

//***************************************************************************
int Kwave::RecordPulseAudio::setSampleFormat(Kwave::SampleFormat new_format)
{
    if (m_sample_format != new_format) close();
    m_sample_format = new_format;
    return 0;
}

//***************************************************************************
QList< Kwave::SampleFormat > Kwave::RecordPulseAudio::detectSampleFormats()
{
    kDebug() << " call";
    QList<Kwave::SampleFormat> list;

    // try all known sample formats
    foreach(int it, m_supported_formats)
    {
	const pa_sample_format_t *fmt = &(_known_formats[it]);
	const Kwave::SampleFormat sample_format = sample_format_of(*fmt);

	// only accept bits/sample if compression types
	// and bits per sample match
	if (compression_of(*fmt) != m_compression) continue;
	if (bits_of(*fmt) != static_cast<int>(m_bits_per_sample))
	    continue;

	// do not produce duplicates
	if (list.contains(sample_format)) continue;

	Kwave::SampleFormat::Map sf;
	kDebug() << "found sample format " << static_cast<int>(sample_format)
		<< " (" << DBG(sf.name(sf.findFromData(sample_format)))
		<< ")";

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
    if (m_bits_per_sample != new_bits) close();
    m_bits_per_sample = new_bits;
    return 0;
}

//***************************************************************************
QList< unsigned int > Kwave::RecordPulseAudio::supportedBits()
{
    kDebug() << " call";
    QList<unsigned int> list;

    // try all known sample formats
    foreach(int it, m_supported_formats)
    {
	const pa_sample_format_t *fmt = &(_known_formats[it]);
	const unsigned int bits = bits_of(*fmt);

	// 0  bits means invalid/does not apply
	if (!bits) continue;

	// only accept bits/sample if compression matches
	if (compression_of(*fmt) != m_compression) continue;

	// do not produce duplicates
	if (list.contains(bits)) continue;

	kDebug() << "found bits/sample " << bits;
	list.append(bits);
    }

    return list;
}

//***************************************************************************
int Kwave::RecordPulseAudio::compression()
{
    return m_compression;
}

//***************************************************************************
int Kwave::RecordPulseAudio::setCompression(int new_compression)
{
    if (m_compression != new_compression) close();
    m_compression = new_compression;
    return m_compression;
}

//***************************************************************************
QList< int > Kwave::RecordPulseAudio::detectCompressions()
{
    kDebug() << " call";
    QList<int> list;

    // try all known sample formats
    foreach(int it, m_supported_formats)
    {
	const pa_sample_format_t *fmt = &(_known_formats[it]);
	int compression = compression_of(*fmt);

	// do not produce duplicates
	if (list.contains(compression)) continue;

	Kwave::Compression t(compression);
	kDebug() << "found compression " << compression << " '"
	       << DBG(t.name()) << "'";
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
    if (new_rate != m_rate) close();
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

    // try all known sample rates
    for (unsigned int i = 0; i < ELEMENTS_OF(known_rates); i++) {
	list.append(known_rates[i]);
    }

    return list;
}

//***************************************************************************
int Kwave::RecordPulseAudio::tracks()
{
    return m_tracks;
}

//***************************************************************************
int Kwave::RecordPulseAudio::setTracks(unsigned int& tracks)
{
    if (tracks != m_tracks) close();
    m_tracks = tracks;
    return 0;
}

//***************************************************************************
int Kwave::RecordPulseAudio::detectTracks(unsigned int& min, unsigned int& max)
{
    kDebug() << " call";
    min = 1;
    max = PA_CHANNELS_MAX;
    return 0;
}

//***************************************************************************
int Kwave::RecordPulseAudio::close()
{
    kDebug() << " call close stream " << static_cast<void *>(m_pa_stream);

    if(m_pa_stream) {
	pa_stream_drop(m_pa_stream);
	pa_stream_unref(m_pa_stream);
    }
    m_pa_stream = 0;

    // set hourglass cursor, we are waiting...
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    // we need to re-initialize the next time
    m_initialized = false;

    QApplication::restoreOverrideCursor();
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
    if(readableSize > freeBytes) {
	int additional_size = readableSize - freeBytes;
	buffer.resize(length + additional_size);
    }

    size_t readLength = 0;
    if(readableSize > 0) {
	const void *audioBuffer;
	pa_stream_peek(m_pa_stream, &audioBuffer, &readLength);

	char *data = buffer.data() + offset;
	memcpy(data, audioBuffer, readLength);

	pa_stream_drop(m_pa_stream);

    } else {
	m_mainloop_lock.unlock();
	return -EAGAIN;
    }
    m_mainloop_lock.unlock();

    return static_cast<int>(readLength);
}
//***************************************************************************
int Kwave::RecordPulseAudio::initialize(uint32_t buffer_size)
{
    Q_ASSERT(!m_initialized);

    // make sure that we are connected to the sound server
    if (!connectToServer()) {
	kDebug() << "Connecting to the PulseAudio server failed.";
	return -1;
    }

    int format_index = mode2format(m_compression, m_bits_per_sample,
                                   m_sample_format);
    Q_ASSERT(format_index >= 0);
    if (format_index < 0) {
	Kwave::SampleFormat::Map sf;

	kWarning() << "format: no matching format for compression '"
		    << DBG(sf.description(sf.findFromData(m_sample_format), true))
		    << "', "
		    << m_bits_per_sample
		    << " bits/sample, format '"
		    << DBG(Kwave::Compression(m_compression).name())
		    << "'";
	return -EINVAL;
    }

    Q_ASSERT(format_index >= 0);

    pa_sample_spec sample_spec;
    sample_spec.channels = m_tracks;
    sample_spec.format = _known_formats[format_index];
    sample_spec.rate = static_cast<quint32>(m_rate);

    if(!pa_sample_spec_valid(&sample_spec)) {
	Kwave::SampleFormat::Map sf;

	kWarning() << "no valid pulse audio format: '"
		    << DBG(sampleFormatToString(_known_formats[format_index]))
		    << "', Rate: '"
		    << m_rate
		    << "', Channels: '"
		    << m_tracks
		    << "'";
	return -EINVAL;
    }

    // run with mainloop locked from here on...
    m_mainloop_lock.lock();

    pa_channel_map channel_map;
    pa_channel_map_init_extend(&channel_map, sample_spec.channels, PA_CHANNEL_MAP_DEFAULT);

    if (!pa_channel_map_compatible(&channel_map, &sample_spec)) {
        kWarning() << "Channel map doesn't match sample specification!";
    }

    // create a new stream
    m_pa_stream = pa_stream_new(
	m_pa_context,
	m_name.constData(),
	&sample_spec,
	&channel_map);

    if (!m_pa_stream) {
	m_mainloop_lock.unlock();
	kWarning() << "Failed to create a PulseAudio stream "
	            << _(pa_strerror(pa_context_errno(m_pa_context)));
	return -1;
    }
    kDebug() << "PulseAudio record - stream created as "
	<< static_cast<void *>(m_pa_stream);

    pa_stream_set_state_callback(m_pa_stream, pa_stream_state_cb, this);
    pa_stream_set_read_callback(m_pa_stream, pa_read_cb, this);

    int flags = 0;

    pa_buffer_attr attr;
    attr.fragsize  = buffer_size;
    attr.maxlength = static_cast<uint32_t>(-1);
    attr.minreq    = static_cast<uint32_t>(-1);
    attr.prebuf    = static_cast<uint32_t>(-1);
    attr.tlength   = static_cast<uint32_t>(-1);
    flags |= PA_STREAM_ADJUST_LATENCY;

    // connect the stream in record mode
    int result = pa_stream_connect_record(
	m_pa_stream,
	m_pa_device.constData(),
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
	m_pa_stream = 0;
	kWarning() << "Failed to open a PulseAudio stream for record "
	            << _(pa_strerror(pa_context_errno(m_pa_context)));
	return -1;
    }

    m_initialized = true;
    return 0;
}

//***************************************************************************
int Kwave::RecordPulseAudio::open(const QString& device)
{
    kDebug() << " Device(" << DBG(device) << ")";

    // close the previous device
    if (m_pa_stream) close();

    if (!m_device_list.contains(device)) {
	kDebug() << "The PulseAudio device '"
	<< device.section(QLatin1Char('|'), 0, 0)
	<< "' is unknown or no longer connected";
	return -1;
    }
    QString pa_device = m_device_list[device].m_name;
    kDebug() << " Pulse Audio Device (" << DBG(pa_device) <<")";

    if(!pa_device.length()) return -ENOENT;

    m_pa_device = pa_device.toUtf8();

    m_supported_formats.clear();
    for(unsigned int i=0; i < ELEMENTS_OF(_known_formats); ++i) {
	m_supported_formats.append(i);
    }

    return 0;
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
QString Kwave::RecordPulseAudio::fileFilter()
{
    return Kwave::RecordDevice::fileFilter();
}

//***************************************************************************
void Kwave::RecordPulseAudio::run_wrapper(const QVariant &params)
{
    Q_UNUSED(params);
    m_mainloop_lock.lock();
    pa_mainloop_run(m_pa_mainloop, 0);
    m_mainloop_lock.unlock();
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
                     __(QLocale::system().name()));
    pa_proplist_sets(m_pa_proplist, PA_PROP_APPLICATION_NAME,
                     __(qApp->applicationName()));
    pa_proplist_sets(m_pa_proplist, PA_PROP_APPLICATION_ICON_NAME,
                     "kwave");
    pa_proplist_sets(m_pa_proplist, PA_PROP_APPLICATION_PROCESS_BINARY,
                     "kwave");
    pa_proplist_setf(m_pa_proplist, PA_PROP_APPLICATION_PROCESS_ID,
                    "%ld", static_cast<long int>(qApp->applicationPid()));
    KUser user;
    pa_proplist_sets(m_pa_proplist, PA_PROP_APPLICATION_PROCESS_USER,
                     __(user.loginName()));
    pa_proplist_sets(m_pa_proplist, PA_PROP_APPLICATION_VERSION,
                     __(qApp->applicationVersion()));

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
	0,                                  // server
	static_cast<pa_context_flags_t>(0), // flags
	0                                   // API
    );
    if (error < 0)
    {
	kWarning() << "RecordPulseAudio: pa_contect_connect failed ("
	           << pa_strerror(pa_context_errno(m_pa_context))
		   << ")";
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
		kDebug() << "RecordPulseAudio: context is ready :-)";
		failed = false;
	    }
	}
	m_mainloop_lock.unlock();

	if (failed) {
	    kWarning() << "RecordPulseAudio: context FAILED ("
	               << pa_strerror(pa_context_errno(m_pa_context))
		       << "):-(";
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
    // stop the main loop
    m_mainloop_thread.cancel();
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
	m_pa_context  = 0;
    }

    // stop and free the main loop
    if (m_pa_mainloop) {
	pa_mainloop_free(m_pa_mainloop);
	m_pa_mainloop = 0;
	kDebug() << "RecordPulseAudio: mainloop freed";
    }

    // release the property list
    if (m_pa_proplist) {
	pa_proplist_free(m_pa_proplist);
	m_pa_proplist = 0;
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
    Q_UNUSED(c);
    Q_ASSERT(c == m_pa_context);
    if (eol == 0) {
#if 1
	kDebug() << " [" << info->index
	<< "] source='" << info->name << "' (" << info->description
	<< ") driver='" << info->driver
	<< "' card=" << info->card
	<< ", ports=" << info->n_ports
	<< ", volume=" << pa_sw_volume_to_linear(pa_cvolume_avg(&info->volume));

	for (uint32_t p = 0; p < info->n_ports; p++) {
	    kDebug() << "                     [" << p
	    << "] - '" << info->ports[p]->name
	    << "' (" << info->ports[p]->description
	    << "), prio=" << info->ports[p]->priority << ((info->ports[p] == info->active_port) ? " <*>" : "");
	}
#endif
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
    pa_operation *op_source_info = pa_context_get_source_info_list(
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
    kDebug() << "----------------------------------------";
    QMap<QString, source_info_t> list;

    // first entry == default device
    source_info_t i;
    pa_sample_spec s;
    s.format   = PA_SAMPLE_INVALID;
    s.rate     = 0;
    s.channels = 0;
    i.m_name        = QString();
    i.m_description = _("(server default)");
    i.m_driver      = QString();
    i.m_card        = -1;
    i.m_sample_spec = s;
    list[i18n("(Use server default)") + _("|sound_note")] = i;

    foreach (QString source, m_device_list.keys()) {
	QString name        = m_device_list[source].m_name;
	QString description = m_device_list[source].m_description;
	QString driver      = m_device_list[source].m_driver;

	// if the name is not unique, add the internal source name
	int unique = true;
	foreach (QString s, m_device_list.keys()) {
	    if (s == source) continue;
	    if ((m_device_list[s].m_description == description) &&
		(m_device_list[s].m_driver      == driver))
	    {
		unique = false;
		break;
	    }
	}
	if (!unique) description += _(" [") + name + _("]");

	// mangle the driver name, e.g.
	// "module-alsa-sink.c" -> "alsa sink"
	QFileInfo f(driver);
	driver = f.baseName();
	driver.replace(_("-"), _(" "));
	driver.replace(_("_"), _(" "));
	if (driver.toLower().startsWith(_("module ")))
	    driver.remove(0, 7);
	description.prepend(driver + _("|sound_card||"));

	// add the leaf node
	if (m_device_list[source].m_card != PA_INVALID_INDEX)
	    description.append(_("|sound_device"));
	else
	    description.append(_("|sound_note"));

	kDebug() << "supported device: '"<< DBG(description) << "'";
	list.insert(description, m_device_list[source]);
    }
    kDebug() << "----------------------------------------";

    m_device_list = list;
    m_mainloop_lock.unlock();
}

#endif /* HAVE_PULSEAUDIO_SUPPORT */

//***************************************************************************
//***************************************************************************
