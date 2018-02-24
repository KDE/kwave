/*************************************************************************
          Record-Qt.cpp  -  device for audio recording via Qt Multimedia
                             -------------------
    begin                : Sun Mar 20 2016
    copyright            : (C) 2016 by Thomas Eschenbacher
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
#ifdef HAVE_QT_AUDIO_SUPPORT

#include <algorithm>
#include <limits>

#include <QApplication>
#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include <QAudioInput>
#include <QCursor>
#include <QFileInfo>
#include <QIODevice>
#include <QLatin1Char>
#include <QLocale>
#include <QString>
#include <QVariant>
#include <QtGlobal>

#include <KLocalizedString>

#include "libkwave/Compression.h"
#include "libkwave/SampleFormat.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"
#include "libkwave/memcpy.h"

#include "Record-Qt.h"

/** gui name of the default device */
#define DEFAULT_DEVICE (i18n("Default device") + _("|sound_note"))

/** timeout for read operation [ms] */
#define RECORD_POLL_TIMEOUT 200

/**
 * factor that determines how much the device device buffer should be larger
 * compared to the buffers used by the record plugin
 */
#define BUFFER_SIZE_OVERCOMMIT 2

//***************************************************************************
Kwave::RecordQt::RecordQt()
    :QObject(Q_NULLPTR),
     Kwave::RecordDevice(),
     m_lock(QMutex::Recursive),
     m_device_name_map(),
     m_available_devices(),
     m_input(Q_NULLPTR),
     m_source(Q_NULLPTR),
     m_sample_format(Kwave::SampleFormat::Unknown),
     m_tracks(0),
     m_rate(0.0),
     m_compression(Kwave::Compression::NONE),
     m_bits_per_sample(0),
     m_device(),
     m_initialized(false),
     m_sem(0)
{
    connect(this, SIGNAL(sigCreateRequested(QAudioFormat&,uint)),
            this, SLOT(createInMainThread(QAudioFormat&,uint)),
            Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(sigCloseRequested()),
            this, SLOT(closeInMainThread()),
            Qt::BlockingQueuedConnection);
}

//***************************************************************************
Kwave::RecordQt::~RecordQt()
{
    close();
}

//***************************************************************************
Kwave::SampleFormat::Format Kwave::RecordQt::sampleFormat()
{
    return m_sample_format;
}

//***************************************************************************
int Kwave::RecordQt::setSampleFormat(Kwave::SampleFormat::Format new_format)
{
    if (m_sample_format == new_format) return 0;
    close();
    m_sample_format = new_format;
    return 0;
}

//***************************************************************************
QList<Kwave::SampleFormat::Format> Kwave::RecordQt::detectSampleFormats()
{
    QList<Kwave::SampleFormat::Format> list;
    QMutexLocker _lock(&m_lock); // context: main thread

    const QAudioDeviceInfo info(deviceInfo(m_device));

    // no devices at all -> empty
    if (info.isNull()) return list;

    // iterate over all supported bits per sample
    foreach (QAudioFormat::SampleType t, info.supportedSampleTypes()) {
	switch (t) {
	    case QAudioFormat::SignedInt:
		list.append(Kwave::SampleFormat::Signed);
		break;
	    case QAudioFormat::UnSignedInt:
		list.append(Kwave::SampleFormat::Unsigned);
		break;
	    case QAudioFormat::Float:
		list.append(Kwave::SampleFormat::Float);
		break;
	    default:
		break;
	}
    }

    return list;
}

//***************************************************************************
Kwave::byte_order_t Kwave::RecordQt::endianness()
{
    Kwave::byte_order_t byte_order = Kwave::UnknownEndian;
    QMutexLocker _lock(&m_lock); // context: main thread

    const QAudioDeviceInfo info(deviceInfo(m_device));

    // no devices at all -> empty
    if (info.isNull()) return byte_order;

    // iterate over all supported bits per sample
    switch (info.preferredFormat().byteOrder()) {
	case QAudioFormat::LittleEndian:
	    byte_order = Kwave::LittleEndian;
	    break;
	case QAudioFormat::BigEndian:
	    byte_order = Kwave::BigEndian;
	    break;
	default:
	    break;
    }

    return byte_order;
}

//***************************************************************************
int Kwave::RecordQt::bitsPerSample()
{
    return m_bits_per_sample;
}

//***************************************************************************
int Kwave::RecordQt::setBitsPerSample(unsigned int new_bits)
{
    if (new_bits == m_bits_per_sample) return 0;

    close();
    m_bits_per_sample = new_bits;
    return 0;
}

//***************************************************************************
QList< unsigned int > Kwave::RecordQt::supportedBits()
{
    QList<unsigned int> list;
    QMutexLocker _lock(&m_lock); // context: main thread

    const QAudioDeviceInfo info(deviceInfo(m_device));

    // no devices at all -> empty
    if (info.isNull()) return list;

    // iterate over all supported bits per sample
    foreach (int bits, info.supportedSampleSizes()) {
	if (bits <= 0) continue;
	list.append(Kwave::toUint(bits));
    }

    std::sort(list.begin(), list.end(), std::less<unsigned int>());
    return list;
}

//***************************************************************************
Kwave::Compression::Type Kwave::RecordQt::compression()
{
    return m_compression;
}

//***************************************************************************
int Kwave::RecordQt::setCompression(Kwave::Compression::Type new_compression)
{
    if (new_compression != m_compression) {
	close();
	m_compression = new_compression;
    }
    return 0;
}

//***************************************************************************
QList<Kwave::Compression::Type> Kwave::RecordQt::detectCompressions()
{
    QList<Kwave::Compression::Type> list;
    list.append(Kwave::Compression::NONE);
    return list;
}

//***************************************************************************
double Kwave::RecordQt::sampleRate()
{
    return m_rate;
}

//***************************************************************************
int Kwave::RecordQt::setSampleRate(double& new_rate)
{
    if (qFuzzyCompare(new_rate, m_rate))  return 0;

    close();
    m_rate = new_rate;
    return 0;
}

//***************************************************************************
QList< double > Kwave::RecordQt::detectSampleRates()
{
    QList<double> list;
    QMutexLocker _lock(&m_lock); // context: main thread

    const QAudioDeviceInfo info(deviceInfo(m_device));

    // no devices at all -> empty
    if (info.isNull()) return list;

    // iterate over all supported sample sizes
    foreach (int rate, info.supportedSampleRates()) {
	if (rate <= 0) continue;
	list.append(static_cast<double>(rate));
    }

    std::sort(list.begin(), list.end(), std::less<double>());
    return list;
}

//***************************************************************************
int Kwave::RecordQt::tracks()
{
    return m_tracks;
}

//***************************************************************************
int Kwave::RecordQt::setTracks(unsigned int &tracks)
{
    if (tracks == m_tracks) return 0;
    if (tracks > 255) tracks = 255;

    close();
    m_tracks = static_cast<quint8>(tracks);
    return 0;
}

//***************************************************************************
int Kwave::RecordQt::detectTracks(unsigned int &min, unsigned int &max)
{
    QMutexLocker _lock(&m_lock); // context: main thread

    const QAudioDeviceInfo info(deviceInfo(m_device));

    max = std::numeric_limits<unsigned int>::min();
    min = std::numeric_limits<unsigned int>::max();

    // no devices at all -> empty
    if (info.isNull()) return -1;

    // iterate over all supported sample sizes
    foreach (int channels, info.supportedChannelCounts()) {
	if (channels <= 0) continue;
	unsigned int c = Kwave::toUint(channels);
	if (c < 1) continue;
	if (c < min) min = c;
	if (c > max) max = c;
    }

    return (max > 0) ? max : -1;
}

//***************************************************************************
QAudioDeviceInfo Kwave::RecordQt::deviceInfo(const QString &device) const
{
    // check for default device
    if (!device.length() || (device == DEFAULT_DEVICE))
	return QAudioDeviceInfo::defaultInputDevice();

    // check if the device name is known
    if (m_device_name_map.isEmpty() || !m_device_name_map.contains(device))
	return QAudioDeviceInfo();

    // translate the path into a Qt audio output device name
    // iterate over all available devices
    const QString dev_name = m_device_name_map[device];
    foreach (const QAudioDeviceInfo &dev, m_available_devices) {
	if (dev.deviceName() == dev_name)
	    return QAudioDeviceInfo(dev);
    }

    // fallen through: return empty info
    return QAudioDeviceInfo();
}

//***************************************************************************
void Kwave::RecordQt::closeInMainThread()
{
    if (m_source) {
	m_source->close();
        m_source = Q_NULLPTR;
    }
    if (m_input) {
	m_input->stop();
	m_input->reset();
	delete m_input;
        m_input = Q_NULLPTR;
    }

    m_initialized = false;
}

//***************************************************************************
int Kwave::RecordQt::close()
{
    QMutexLocker _lock(&m_lock);

    if (QThread::currentThread() == qApp->thread())
	closeInMainThread();
    else
	emit sigCloseRequested();

    return 0;
}

//***************************************************************************
int Kwave::RecordQt::read(QByteArray& buffer, unsigned int offset)
{
    if (buffer.isNull() || buffer.isEmpty())
	return 0; // no buffer, nothing to do

    int buffer_size = buffer.size();

    // we configure our device at a late stage, otherwise we would not
    // know the internal buffer size
    if (!m_initialized) {
	int err = initialize(buffer_size);
	if (err < 0) return -EAGAIN;
	m_initialized = true;
    }
    Q_ASSERT(m_source);
    Q_ASSERT(m_input);
    if (!m_source || !m_input)
	return -ENODEV;

    // adjust the buffer size if is has been changed in the plugin
    if ((buffer_size > 0) && (m_input->bufferSize() != buffer_size))
	m_input->setBufferSize(buffer_size * BUFFER_SIZE_OVERCOMMIT);

    // wait until some data gets available (with timeout)
    m_sem.tryAcquire(1, RECORD_POLL_TIMEOUT);

    char *p = buffer.data() + offset;
    unsigned int len = buffer.length() - offset;
    qint64 length = m_source->read(p, len);

    return (length < 1) ? -EAGAIN : Kwave::toInt(length);
}

//***************************************************************************
QString Kwave::RecordQt::open(const QString& device)
{

    // close the previous device
    close();

    QMutexLocker _lock(&m_lock);

    // make sure we have a valid list of devices
    scanDevices();

    const QAudioDeviceInfo info(deviceInfo(device));
    if (info.isNull()) {
	return QString::number(ENODEV);
    }

    m_device = device;
    return QString();
}

//***************************************************************************
QStringList Kwave::RecordQt::supportedDevices()
{
    QMutexLocker _lock(&m_lock); // context: main thread

    // re-validate the list if necessary
    if (m_device_name_map.isEmpty() || m_available_devices.isEmpty())
	scanDevices();

    QStringList list = m_device_name_map.keys();

    // move the "default" device to the start of the list
    if (list.contains(DEFAULT_DEVICE))
	list.move(list.indexOf(DEFAULT_DEVICE), 0);

    if (!list.isEmpty()) list.append(_("#TREE#"));

    return list;
}

//***************************************************************************
int Kwave::RecordQt::initialize(unsigned int buffer_size)
{
    // do sanity checks of the current parameters, otherwise Qt crashes
    // with floating point errors or similar
    if (m_rate < 1.0)          return -EINVAL;
    if (m_bits_per_sample < 1) return -EINVAL;
    if (m_tracks < 1)          return -EINVAL;
    if (!m_device.length())    return -EINVAL;

    const QAudioDeviceInfo info(deviceInfo(m_device));

    // find a supported sample format
    const QAudioFormat preferred_format(info.preferredFormat());
    QAudioFormat format(preferred_format);
    format.setSampleSize(Kwave::toInt(m_bits_per_sample));
    format.setChannelCount(Kwave::toInt(m_tracks));
    format.setSampleRate(Kwave::toInt(m_rate));
    format.setCodec(_("audio/pcm"));

    // find a replacement format with matching codec, tracks, bits and rate
    if (!format.isValid() || !info.isFormatSupported(format))
	format = info.nearestFormat(format);

    if (format.codec() != _("audio/pcm")) {
	qWarning("PCM encoding is not supported");
	return -EIO;
    }

    if (format.sampleSize() != Kwave::toInt(m_bits_per_sample)) {
	qWarning("%d bits per sample are not supported", m_bits_per_sample);
	return -EIO;
    }

    if (format.channelCount() != Kwave::toInt(m_tracks)) {
	qWarning("recording with %d channels is not supported", m_tracks);
	return -EIO;
    }

    if (format.sampleRate() != Kwave::toInt(m_rate)) {
	qWarning("sample rate %d Hz is not supported", Kwave::toInt(m_rate));
	return -EIO;
    }

    if ( (format.sampleType() != QAudioFormat::SignedInt) &&
	 (format.sampleType() != QAudioFormat::UnSignedInt) )
    {
	qWarning("integer sample format is not supported");
	return -EIO;
    }

    // create a new Qt output device
    if (QThread::currentThread() == qApp->thread())
	createInMainThread(format, buffer_size);
    else
	emit sigCreateRequested(format, buffer_size);

    return 0;
}

//***************************************************************************
void Kwave::RecordQt::createInMainThread(QAudioFormat &format,
                                         unsigned int buffer_size)
{
    QMutexLocker _lock(&m_lock);

    // reset the semaphore to zero
    m_sem.acquire(m_sem.available());

    // create a new audio device for the selected format
    m_input = new QAudioInput(format, this);
    Q_ASSERT(m_input);
    if (!m_input) return;
    connect(m_input, SIGNAL(notify()), this, SLOT(notified()));

    // set the buffer size, before starting to record
    m_input->setBufferSize(buffer_size * BUFFER_SIZE_OVERCOMMIT);

    // start recording engine
    m_source = m_input->start();
}

//***************************************************************************
void Kwave::RecordQt::notified()
{
    m_sem.release();
}

//***************************************************************************
void Kwave::RecordQt::scanDevices()
{
    m_available_devices.clear();
    m_device_name_map.clear();

    // get the list of available audio output devices from Qt
    foreach (const QAudioDeviceInfo &device,
	     QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
    {
	QString qt_name = device.deviceName();

	// for debugging: list all devices
// 	qDebug("    name='%s'", DBG(qt_name));

	// device name not available ?
	if (!qt_name.length()) {
	    qWarning("RecordQt::supportedDevices() "
		      "=> BUG: device with no name?");
	    continue;
	}

	QString gui_name = qt_name + _("|sound_note");
	if (m_device_name_map.contains(gui_name)) {
	    qWarning("RecordQt::supportedDevices() "
	    "=> BUG: duplicate device name: '%s'", DBG(gui_name));
	    continue;
	}

	m_available_devices.append(device);
	m_device_name_map[gui_name] = qt_name;
    }
}

#endif /* HAVE_QT_AUDIO_SUPPORT */

//***************************************************************************
//***************************************************************************
