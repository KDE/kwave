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
#include <new>

#include <QApplication>
#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioSource>
#include <QCursor>
#include <QFileInfo>
#include <QIODevice>
#include <QLatin1Char>
#include <QLocale>
#include <QMediaDevices>
#include <QString>
#include <QThread>
#include <QVariant>
#include <QtGlobal>

#include <KLocalizedString>

#include "libkwave/Compression.h"
#include "libkwave/SampleFormat.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"

#include "Record-Qt.h"

/** gui name of the default device */
#define DEFAULT_DEVICE (i18n("Default device") + _("|sound_note"))

/**
 * factor that determines how much the device device buffer should be larger
 * compared to the buffers used by the record plugin
 */
#define BUFFER_SIZE_OVERCOMMIT 2

//***************************************************************************
Kwave::RecordQt::RecordQt()
    :QObject(nullptr),
     Kwave::RecordDevice(),
     m_lock(),
     m_device_name_map(),
     m_available_devices(),
     m_input(nullptr),
     m_source(nullptr),
     m_sample_format(Kwave::SampleFormat::Unknown),
     m_tracks(0),
     m_rate(0.0),
     m_compression(Kwave::Compression::NONE),
     m_bits_per_sample(0),
     m_device(),
     m_initialized(false)
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

    const QAudioDevice info(getDevice(m_device));

    // no devices at all -> empty
    if (info.isNull()) return list;

    // add sample formats compatible with current bit per sample
    auto supported_formats = info.supportedSampleFormats();
    switch (Kwave::toInt(m_bits_per_sample)) {
        case 8:
            if (supported_formats.contains(QAudioFormat::UInt8)) {
                list.append(Kwave::SampleFormat::Unsigned);
            }
            break;
        case 16:
            if (supported_formats.contains(QAudioFormat::Int16)) {
                list.append(Kwave::SampleFormat::Signed);
            }
            break;
        case 32:
            if (supported_formats.contains(QAudioFormat::Int32)) {
                list.append(Kwave::SampleFormat::Signed);
            }
            if (supported_formats.contains(QAudioFormat::Float)) {
                list.append(Kwave::SampleFormat::Float);
            }
            break;
        default:
            break;
    }

    return list;
}

//***************************************************************************
Kwave::byte_order_t Kwave::RecordQt::endianness()
{
    return Kwave::CpuEndian;
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

    const QAudioDevice info(getDevice(m_device));

    // no devices at all -> empty
    if (info.isNull()) return list;

    // iterate over all supported bits per sample
    unsigned int bits = 0;
    for (QAudioFormat::SampleFormat format : info.supportedSampleFormats()) {
        switch (format) {
            case QAudioFormat::UInt8:
                bits = 8;
                break;
            case QAudioFormat::Int16:
                bits = 16;
                break;
            case QAudioFormat::Int32:
            case QAudioFormat::Float:
                bits = 32;
                break;
            default:
                bits = 0;
                break;
        }
        if (!list.contains(bits) && (bits > 0))
            list.append(bits);
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

    const QAudioDevice info(getDevice(m_device));

    // no devices at all -> empty
    if (info.isNull()) return list;

    static const int known_rates[] = {
        8000,
        9600,
        11025,
        16000,
        22050,
        24000,
        32000,
        44100,
        48000,
        64000,
        88200,
        96000,
        128000,
        192000
    };

    for (const int rate : known_rates) {
        if (rate < info.minimumSampleRate() || rate > info.maximumSampleRate()) {
            continue;
        }

        QAudioFormat format = info.preferredFormat();
        format.setSampleRate(rate);
        if (info.isFormatSupported(format)) {
            list.append(rate);
        }
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

    const QAudioDevice info(getDevice(m_device));

    min = info.minimumChannelCount();
    max = info.maximumChannelCount();

    return (max > 0) ? max : -1;
}

//***************************************************************************
QAudioDevice Kwave::RecordQt::getDevice(const QString &device) const
{
    // check for default device
    if (!device.length() || (device == DEFAULT_DEVICE))
        return QMediaDevices::defaultAudioInput();

    // check if the device name is known
    if (m_device_name_map.isEmpty() || !m_device_name_map.contains(device))
        return QAudioDevice();

    // translate the path into a Qt audio output device name
    // iterate over all available devices
    const QByteArray dev_id = m_device_name_map[device];
    for (const QAudioDevice &dev : m_available_devices) {
        if (dev.id() == dev_id)
            return QAudioDevice(dev);
    }

    // fallen through: return empty info
    return QAudioDevice();
}

//***************************************************************************
void Kwave::RecordQt::closeInMainThread()
{
    if (m_source) {
        m_source->close();
        m_source = nullptr;
    }
    if (m_input) {
        m_input->stop();
        m_input->reset();
        delete m_input;
        m_input = nullptr;
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
int Kwave::RecordQt::read(QByteArray &buffer, unsigned int offset)
{
    if (buffer.isNull() || buffer.isEmpty())
        return 0; // no buffer, nothing to do

    unsigned int buffer_size = static_cast<unsigned int>(buffer.size());

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

    char  *p      = buffer.data()   + offset;
    qint64 len    = buffer.length() - offset;
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

    const QAudioDevice info(getDevice(device));
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

    const QAudioDevice info(getDevice(m_device));

    // find a supported sample format
    QAudioFormat format(info.preferredFormat());
    switch (Kwave::toInt(m_bits_per_sample)) {
        case 8:
            format.setSampleFormat(QAudioFormat::UInt8);
            break;
        case 16:
            format.setSampleFormat(QAudioFormat::Int16);
            break;
        case 32:
            if (format.sampleFormat() == QAudioFormat::Float) {
                break;
            } else {
                format.setSampleFormat(QAudioFormat::Int32);
            }
            break;
        default:
            qWarning("%u bits per sample are not supported",
                     m_bits_per_sample);
            return -EIO;
    }
    format.setChannelCount(Kwave::toInt(m_tracks));
    format.setSampleRate(Kwave::toInt(m_rate));

    if (!format.isValid() || !info.isFormatSupported(format)) {
        qWarning("format not supported");
        return -EIO;
    }


    // create a new Qt output device
    if (QThread::currentThread() == qApp->thread())
        createInMainThread(format, buffer_size);
    else
        emit sigCreateRequested(format, buffer_size);

    return (m_source && m_input) ? 0 : -EAGAIN;
}

//***************************************************************************
void Kwave::RecordQt::createInMainThread(QAudioFormat &format,
                                         unsigned int buffer_size)
{
    QMutexLocker _lock(&m_lock);

    // create a new audio device for the selected format
    m_input = new(std::nothrow) QAudioSource(format, this);
    Q_ASSERT(m_input);
    if (!m_input) return;

    // set the buffer size, before starting to record
    m_input->setBufferSize(buffer_size * BUFFER_SIZE_OVERCOMMIT);

    // start recording engine
    m_source = m_input->start();
}

//***************************************************************************
void Kwave::RecordQt::scanDevices()
{
    m_available_devices.clear();
    m_device_name_map.clear();

    // get the list of available audio output devices from Qt
    for (const QAudioDevice &device : QMediaDevices::audioInputs())
    {
        QByteArray qt_name = device.id();

        // for debugging: list all devices
//      qDebug("    name='%s'", DBG(qt_name));

        // device name not available ?
        if (!qt_name.length()) {
            qWarning("RecordQt::supportedDevices() "
                      "=> BUG: device with no name?");
            continue;
        }

        QString gui_name = device.description() + _("|sound_note");
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

#include "moc_Record-Qt.cpp"
