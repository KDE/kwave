/***************************************************************************
        PlayBack-Qt.cpp  -  playback device for Qt Multimedia
                             -------------------
    begin                : Thu Nov 12 2015
    copyright            : (C) 2015 by Thomas Eschenbacher
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

#include <errno.h>

#include <algorithm>
#include <new>

#include <QApplication>
#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioSink>
#include <QMediaDevices>
#include <QObject>
#include <QSysInfo>
#include <QThread>

#include <KLocalizedString>

#include "libkwave/SampleEncoderLinear.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"

#include "PlayBack-Qt.h"

/** gui name of the default device */
#define DEFAULT_DEVICE (i18n("Default device") + _("|sound_note"))

//***************************************************************************
Kwave::PlayBackQt::PlayBackQt()
    :QObject(), Kwave::PlayBackDevice(),
     m_lock(),
     m_device_name_map(),
     m_available_devices(),
     m_output(Q_NULLPTR),
     m_buffer_size(0),
     m_encoder(Q_NULLPTR)
{
}

//***************************************************************************
Kwave::PlayBackQt::~PlayBackQt()
{
    close();
}

//***************************************************************************
void Kwave::PlayBackQt::createEncoder(const QAudioFormat &format)
{
    // discard the old encoder
    delete m_encoder;
    m_encoder = Q_NULLPTR;

    // get the sample format
    Kwave::SampleFormat::Format sample_format = Kwave::SampleFormat::Unknown;
    unsigned int bits = 0;
    switch (format.sampleFormat()) {
        case QAudioFormat::UInt8:
            sample_format = Kwave::SampleFormat::Unsigned;
            bits = 8;
            break;
        case QAudioFormat::Int16:
            sample_format = Kwave::SampleFormat::Signed;
            bits = 16;
            break;
        case QAudioFormat::Int32:
            sample_format = Kwave::SampleFormat::Signed;
            bits = 32;
            break;
        case QAudioFormat::Float:
            sample_format = Kwave::SampleFormat::Float;
            bits = 32;
            break;
        default:
            sample_format = Kwave::SampleFormat::Unknown;
            break;
    }
    if (sample_format == Kwave::SampleFormat::Unknown) {
        qWarning("PlayBackQt: unsupported sample format %d",
                 static_cast<int>(format.sampleFormat()));
        return;
    }

    // create the sample encoder
    m_encoder = new(std::nothrow)
        Kwave::SampleEncoderLinear(sample_format, bits, Kwave::CpuEndian);
}

//***************************************************************************
QString Kwave::PlayBackQt::open(const QString &device, double rate,
                                unsigned int channels, unsigned int bits,
                                unsigned int bufbase)
{
    qDebug("PlayBackQt::open(device='%s', rate=%0.1f,channels=%u, bits=%u, "
           "bufbase=%u)", DBG(device), rate, channels, bits, bufbase);

    if ((rate < double(1.0f))  || !channels || !bits || !bufbase)
        return i18n("One or more invalid/out of range arguments.");

    // close the previous device
    close();

    QMutexLocker _lock(&m_lock); // context: main thread

    // make sure we have a valid list of devices
    scanDevices();

    const QAudioDevice info(getDevice(device));
    if (info.isNull()) {
        return i18n("The audio device '%1' is unknown or no longer connected",
                    device.section(QLatin1Char('|'), 0, 0));
    }

    // find a supported sample format
    QAudioFormat format(info.preferredFormat());
    switch (Kwave::toInt(bits)) {
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
            return i18n("%1 bits per sample are not supported", bits);
    }
    format.setChannelCount(Kwave::toInt(channels));
    format.setSampleRate(Kwave::toInt(rate));

    if (!format.isValid() || !info.isFormatSupported(format))
        return i18n("format not supported");

    // create a sample encoder
    createEncoder(format);
    Q_ASSERT(m_encoder);
    if (!m_encoder) return i18n("Out of memory");

    // create a new Qt output device
    m_output = new(std::nothrow) QAudioSink(format, Q_NULLPTR);
    Q_ASSERT(m_output);
    if (!m_output) return i18n("Out of memory");

    // connect the state machine and the notification engine
    connect(m_output, SIGNAL(stateChanged(QAudio::State)),
            this,     SLOT(stateChanged(QAudio::State)));

    // calculate the buffer size in bytes
    if (bufbase < 8)
        bufbase = 8;
    m_buffer_size = (1U << bufbase);
    qDebug("    buffer size = %u", m_buffer_size);

    m_buffer.start(m_buffer_size, 0);

    // open the output device for writing
    m_output->start(&m_buffer);

    // calculate an appropriate timeout, based on the buffer size
    unsigned int bytes_per_frame = m_encoder->rawBytesPerSample() * channels;
    unsigned int buffer_size = qMax<int>(m_buffer_size, m_output->bufferSize());
    unsigned int buffer_frames =
        ((buffer_size * 2) + (bytes_per_frame - 1)) / bytes_per_frame;
    int timeout = qMax(Kwave::toInt((1000 * buffer_frames) / rate), 100);
    qDebug("    timeout = %d ms", timeout);
    m_buffer.setTimeout(timeout);

    if (m_output->error() != QAudio::NoError) {
        qDebug("error no: %d", int(m_output->error()));
        return i18n("Opening the Qt Multimedia device '%1' failed", device);
    }

    return QString();
}

//***************************************************************************
int Kwave::PlayBackQt::write(const Kwave::SampleArray &samples)
{
    QByteArray frame;
    {
        QMutexLocker _lock(&m_lock); // context: worker thread

        if (!m_encoder || !m_output) return -EIO;

        int bytes_per_sample = m_encoder->rawBytesPerSample();
        int bytes_raw        = samples.size() * bytes_per_sample;

        frame.resize(bytes_raw);
        frame.fill(char(0));
        m_encoder->encode(samples, samples.size(), frame);
    }

    qint64 written = m_buffer.writeData(frame.constData(), frame.size());
    return (written == frame.size()) ? 0 : -EAGAIN;
}

//***************************************************************************
void Kwave::PlayBackQt::stateChanged(QAudio::State state)
{
    Q_ASSERT(m_output);
    if (!m_output) return;

    if (m_output->error() != QAudio::NoError) {
        qDebug("PlaybBackQt::stateChanged(%d), ERROR=%d, "
               "buffer free=%lld",
               static_cast<int>(state),
               static_cast<int>(m_output->error()),
               m_output->bytesFree()
        );
    }
    switch (state) {
        case QAudio::ActiveState:
            qDebug("PlaybBackQt::stateChanged(ActiveState)");
            break;
        case QAudio::SuspendedState:
            qDebug("PlaybBackQt::stateChanged(SuspendedState)");
            break;
        case QAudio::StoppedState: {
            qDebug("PlaybBackQt::stateChanged(StoppedState)");
            break;
        }
        case QAudio::IdleState:
            qDebug("PlaybBackQt::stateChanged(IdleState)");
            break;
        default:
            qWarning("PlaybBackQt::stateChanged(%d)",
                     static_cast<int>(state));
            break;
    }
}

//***************************************************************************
QAudioDevice Kwave::PlayBackQt::getDevice(const QString &device) const
{
    // check for default device
    if (!device.length() || (device == DEFAULT_DEVICE))
        return QMediaDevices::defaultAudioOutput();

    // check if the device name is known
    if (m_device_name_map.isEmpty() || !m_device_name_map.contains(device))
        return QAudioDevice();

    // translate the path into a Qt audio output device id
    // iterate over all available devices
    const QByteArray dev_id = m_device_name_map[device];
    for (const QAudioDevice &dev : m_available_devices) {
        if (dev.id() == dev_id)
            return QAudioDevice(dev);
    }

    // fallen through: return null device
    return QAudioDevice();
}

//***************************************************************************
int Kwave::PlayBackQt::close()
{
    qDebug("Kwave::PlayBackQt::close()");

    QMutexLocker _lock(&m_lock); // context: main thread

    if (m_output && m_encoder) {
        int pad_bytes_cnt   = m_output->bufferSize();
        int bytes_per_frame = m_output->format().bytesPerFrame();
        if ((pad_bytes_cnt > 0) && (bytes_per_frame > 0)) {
            unsigned int pad_samples_cnt = pad_bytes_cnt / bytes_per_frame;
            Kwave::SampleArray pad_samples(pad_samples_cnt);
            QByteArray pad_bytes(pad_bytes_cnt, char(0));
            m_encoder->encode(pad_samples, pad_samples_cnt, pad_bytes);
            m_buffer.drain(pad_bytes);
        }
        m_output->stop();
        m_buffer.stop();

        // stopping the engine might block, so we need to do this unlocked
        qDebug("Kwave::PlayBackQt::close() - flushing..., state=%d",
            m_output->state());
        while (m_output && (m_output->state() != QAudio::StoppedState)) {
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        }
        qDebug("Kwave::PlayBackQt::close() - flushing done.");
    }

    if (m_output) {
        // WARNING: QAudioOutput::~QAudioOutput() calls processEvents() !!!
        m_output->deleteLater();
        m_output = Q_NULLPTR;
    }

    delete m_encoder;
    m_encoder = Q_NULLPTR;

    m_device_name_map.clear();
    m_available_devices.clear();

    qDebug("Kwave::PlayBackQt::close() - DONE");
    return 0;
}

//***************************************************************************
QStringList Kwave::PlayBackQt::supportedDevices()
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
void Kwave::PlayBackQt::scanDevices()
{
    m_device_name_map.clear();

    // get the list of available audio output devices from Qt
    m_available_devices = QMediaDevices::audioOutputs();
    for (const QAudioDevice &device : m_available_devices)
    {
        QByteArray qt_name = device.id();

        // for debugging: list all devices
//      qDebug("name='%s'", DBG(qt_name));

        // device name not available ?
        if (!qt_name.length()) {
            qWarning("PlayBackQt::supportedDevices() "
                      "=> BUG: device with no name?");
            continue;
        }

        QString gui_name = device.description() + _("|sound_note");
        if (m_device_name_map.contains(gui_name)) {
            qWarning("PlayBackQt::supportedDevices() "
            "=> BUG: duplicate device name: '%s'", DBG(gui_name));
            continue;
        }

        m_device_name_map[gui_name] = qt_name;
    }
}

//***************************************************************************
QString Kwave::PlayBackQt::fileFilter()
{
    return _("");
}

//***************************************************************************
QList<unsigned int> Kwave::PlayBackQt::supportedBits(const QString &device)
{
    QMutexLocker _lock(&m_lock); // context: main thread

    QList<unsigned int> list;
    const QAudioDevice info(getDevice(device));

    // no devices at all -> empty list
    if (info.isNull()) return list;

    // iterate over all supported sample sizes
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
            bits = 32;
            break;
        case QAudioFormat::Float:
            bits = 32;
            break;
        default:
            bits = 0;
            break;
        }
        if (!list.contains(bits) && (bits > 0))
            list <<  Kwave::toUint(bits);
    }

    std::sort(list.begin(), list.end(), std::greater<unsigned int>());
    return list;
}

//***************************************************************************
int Kwave::PlayBackQt::detectChannels(const QString &device,
                                      unsigned int &min, unsigned int &max)
{
    QMutexLocker _lock(&m_lock); // context: main thread

    const QAudioDevice info(getDevice(device));

    // no devices at all -> empty
    if (info.isNull()) return -1;

    max = info.maximumChannelCount();
    min = info.minimumChannelCount();

    return (max > 0) ? max : -1;
}

//***************************************************************************
//***************************************************************************

//***************************************************************************
Kwave::PlayBackQt::Buffer::Buffer()
    :QIODevice(),
     m_lock(),
     m_sem_free(0),
     m_sem_filled(0),
     m_raw_buffer(),
     m_timeout(1000),
     m_pad_data(),
     m_pad_ofs(0)
{
}

//***************************************************************************
Kwave::PlayBackQt::Buffer::~Buffer()
{
}

//***************************************************************************
void Kwave::PlayBackQt::Buffer::start(unsigned int buf_size, int timeout)
{
    m_raw_buffer.clear();
    m_sem_filled.acquire(m_sem_filled.available());
    m_sem_free.acquire(m_sem_free.available());
    m_sem_free.release(buf_size);
    m_timeout = timeout;
    m_pad_data.clear();
    m_pad_ofs = 0;

    open(QIODevice::ReadOnly);
}

//***************************************************************************
void Kwave::PlayBackQt::Buffer::setTimeout(int timeout)
{
    QMutexLocker _lock(&m_lock); // context: main thread
    m_timeout = timeout;
    qDebug("Kwave::PlayBackQt::Buffer::setTimeout(%d)", timeout);
}

//***************************************************************************
void Kwave::PlayBackQt::Buffer::drain(const QByteArray &padding)
{
    m_pad_data = padding;
    m_pad_ofs  = 0;
}

//***************************************************************************
void Kwave::PlayBackQt::Buffer::stop()
{
    close();
}

//***************************************************************************
qint64 Kwave::PlayBackQt::Buffer::readData(char *data, qint64 len)
{
    qint64 read_bytes = -1;
    qint64 requested  = len;

//     qDebug("Kwave::PlayBackQt::Buffer::readData(..., len=%lld", requested);
    if (len == 0) return  0;
    if (len  < 0) return -1;

    while (len > 0) {
        int count = qMin<int>(
            qMax(m_sem_filled.available(), 1),
            Kwave::toInt(len)
        );
        if (Q_LIKELY(m_sem_filled.tryAcquire(count, m_timeout))) {
            QMutexLocker _lock(&m_lock); // context: qt streaming engine

            m_sem_free.release(count);
            if (read_bytes < 0) read_bytes = 0;
            read_bytes += count;
            len        -= count;
            while (count--)
                *(data++) = m_raw_buffer.dequeue();
        } else break;
    }

    // if we are at the end of the stream: do some padding to satisfy Qt
    while ( (read_bytes < requested) &&
            !m_pad_data.isEmpty() && (m_pad_ofs < m_pad_data.size()) )
    {
        *(data++) = 0;
        read_bytes++;
        m_pad_ofs++;
    }

    if (read_bytes != requested)
        qDebug("Kwave::PlayBackQt::Buffer::readData(...) -> read=%lld/%lld",
               read_bytes, requested);

    return read_bytes;
}

//***************************************************************************
qint64 Kwave::PlayBackQt::Buffer::writeData(const char *data, qint64 len)
{

    qint64 written_bytes = 0;

    int count = Kwave::toInt(len);
    if (Q_LIKELY(m_sem_free.tryAcquire(count, m_timeout))) {
        QMutexLocker _lock(&m_lock); // context: kwave worker thread
        m_sem_filled.release(count);
        written_bytes += count;
        while (count--)
            m_raw_buffer.enqueue(*(data++));
    }

    return written_bytes;
}

//***************************************************************************
qint64 Kwave::PlayBackQt::Buffer::bytesAvailable() const
{
    return QIODevice::bytesAvailable() +
           m_sem_filled.available() +
           m_pad_data.size() -
           m_pad_ofs;
}

#endif /* HAVE_QT_AUDIO_SUPPORT */

//***************************************************************************
//***************************************************************************

#include "moc_PlayBack-Qt.cpp"
