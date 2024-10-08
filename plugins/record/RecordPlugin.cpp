/*************************************************************************
       RecordPlugin.cpp  -  plugin for recording audio data
                             -------------------
    begin                : Wed Jul 09 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
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

#include <errno.h>
#include <math.h>
#include <stdlib.h>

#include <new>

#include <QApplication>
#include <QCursor>
#include <QDateTime>
#include <QList>
#include <QStringList>
#include <QVariant>
#include <QtGlobal>

#include <KAboutData>
#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>
#include <kxmlgui_version.h>

#include "libkwave/Compression.h"
#include "libkwave/FileInfo.h"
#include "libkwave/InsertMode.h"
#include "libkwave/MessageBox.h"
#include "libkwave/PluginManager.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleFIFO.h"
#include "libkwave/SampleFormat.h"
#include "libkwave/SignalManager.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"
#include "libkwave/Writer.h"

#include "Record-ALSA.h"
#include "Record-OSS.h"
#include "Record-PulseAudio.h"
#include "Record-Qt.h"
#include "RecordDevice.h"
#include "RecordDialog.h"
#include "RecordPlugin.h"
#include "RecordThread.h"
#include "SampleDecoderLinear.h"

KWAVE_PLUGIN(record, RecordPlugin)

#define OPEN_RETRY_TIME 1000 /**< time interval for trying to open [ms] */

//***************************************************************************
Kwave::RecordPlugin::RecordPlugin(QObject *parent, const QVariantList &args)
    :Kwave::Plugin(parent, args),
     m_method(Kwave::RECORD_NONE),
     m_device_name(),
     m_controller(),
     m_state(Kwave::REC_EMPTY),
     m_device(nullptr),
     m_dialog(nullptr),
     m_thread(nullptr),
     m_decoder(nullptr),
     m_prerecording_queue(),
     m_writers(nullptr),
     m_buffers_recorded(0),
     m_inhibit_count(0),
     m_trigger_value(),
     m_retry_timer()
{
    m_retry_timer.setSingleShot(true);
    connect(&m_retry_timer, SIGNAL(timeout()),
            this, SLOT(retryOpen()),
            Qt::QueuedConnection
        );
}

//***************************************************************************
Kwave::RecordPlugin::~RecordPlugin()
{
    Q_ASSERT(!m_dialog);
    delete m_dialog;
    m_dialog = nullptr;

    Q_ASSERT(!m_thread);
    delete m_thread;
    m_thread = nullptr;

    Q_ASSERT(!m_decoder);
    delete m_decoder;
    m_decoder = nullptr;

    delete m_device;
    m_device = nullptr;
}

//***************************************************************************
QStringList *Kwave::RecordPlugin::setup(QStringList &previous_params)
{
    Kwave::RecordDialog::Mode mode = Kwave::RecordDialog::SETTINGS_DEFAULT;

    qDebug("RecordPlugin::setup(%s)", DBG(previous_params.join(_(","))));

    // if we have only one parameter, then we got called with a specific
    // mode, e.g. "show format settings only"
    if (previous_params.count() == 1) {
        const QString m = previous_params[0].toLower();

        if (m == _("format"))
            mode = Kwave::RecordDialog::SETTINGS_FORMAT;
        else if (m == _("source"))
            mode = Kwave::RecordDialog::SETTINGS_SOURCE;
        else if (m == _("start_now"))
            mode = Kwave::RecordDialog::START_RECORDING;

        // get previous parameters for the setup dialog
        previous_params = manager().defaultParams(name());
        qDebug("RecordPlugin::setup(%s) - MODE=%d",
               DBG(previous_params.join(_(","))), static_cast<int>(mode));
    }

    // create the setup dialog
    m_dialog = new(std::nothrow) Kwave::RecordDialog(
        parentWidget(), previous_params, &m_controller, mode
    );
    Q_ASSERT(m_dialog);
    if (!m_dialog) return nullptr;

    // create the lowlevel recording thread
    m_thread = new(std::nothrow) Kwave::RecordThread();
    Q_ASSERT(m_thread);
    if (!m_thread) {
        delete m_dialog;
        m_dialog = nullptr;
        return nullptr;
    }

    // connect some signals of the setup dialog
    connect(m_dialog, SIGNAL(sigMethodChanged(Kwave::record_method_t)),
            this,     SLOT(setMethod(Kwave::record_method_t)));
    connect(m_dialog, SIGNAL(sigDeviceChanged(QString)),
            this,     SLOT(setDevice(QString)));

    connect(m_dialog, SIGNAL(sigTracksChanged(uint)),
            this,     SLOT(changeTracks(uint)));
    connect(m_dialog, SIGNAL(sampleRateChanged(double)),
            this,     SLOT(changeSampleRate(double)));
    connect(m_dialog, SIGNAL(sigCompressionChanged(Kwave::Compression::Type)),
            this,     SLOT(changeCompression(Kwave::Compression::Type)));
    connect(m_dialog, SIGNAL(sigBitsPerSampleChanged(uint)),
            this,     SLOT(changeBitsPerSample(uint)));
    connect(m_dialog,
            SIGNAL(sigSampleFormatChanged(Kwave::SampleFormat::Format)),
            this,
            SLOT(changeSampleFormat(Kwave::SampleFormat::Format)));
    connect(m_dialog, SIGNAL(sigBuffersChanged()),
            this,     SLOT(buffersChanged()));
    connect(this,     SIGNAL(sigRecordedSamples(sample_index_t)),
            m_dialog, SLOT(setRecordedSamples(sample_index_t)));

    connect(m_dialog,      SIGNAL(sigTriggerChanged(bool)),
            &m_controller, SLOT(enableTrigger(bool)));
    m_controller.enableTrigger(
        m_dialog->params().record_trigger_enabled ||
        m_dialog->params().start_time_enabled
    );

    connect(m_dialog, SIGNAL(sigPreRecordingChanged(bool)),
            &m_controller, SLOT(enablePrerecording(bool)));
    connect(m_dialog, SIGNAL(sigPreRecordingChanged(bool)),
            this, SLOT(prerecordingChanged(bool)));
    m_controller.enablePrerecording(m_dialog->params().pre_record_enabled);

    // connect the record controller and this
    connect(&m_controller, SIGNAL(sigReset(bool&)),
            this,          SLOT(resetRecording(bool&)));
    connect(&m_controller, SIGNAL(sigStartRecord()),
            this,          SLOT(startRecording()));
    connect(&m_controller, SIGNAL(sigStopRecord(int)),
            &m_controller, SLOT(deviceRecordStopped(int)));
    connect(&m_controller, SIGNAL(stateChanged(Kwave::RecordState)),
            this,          SLOT(stateChanged(Kwave::RecordState)));

    // connect record controller and record thread
    connect(m_thread,      SIGNAL(stopped(int)),
            &m_controller, SLOT(deviceRecordStopped(int)));

    // connect us to the record thread
    connect(m_thread, SIGNAL(stopped(int)),
            this,     SLOT(recordStopped(int)));
    connect(m_thread, SIGNAL(bufferFull()),
            this,     SLOT(processBuffer()),
            Qt::QueuedConnection);

    // dummy init -> disable format settings
    m_dialog->setSupportedTracks(0, 0);

    // activate the recording method
    setMethod(m_dialog->params().method);

    // directly start recording if requested
    if (mode == Kwave::RecordDialog::START_RECORDING)
        m_controller.actionStart();

    QStringList *list = new(std::nothrow) QStringList();
    Q_ASSERT(list);
    if (list && (m_dialog->exec() == QDialog::Accepted) && m_dialog) {
        // user has pressed "OK"
        *list = m_dialog->params().toList();
    } else {
        // user pressed "Cancel"
        delete list;
        list = nullptr;
    }

    /* de-queue all buffers that are pending and remove the record thread */
    if (m_thread) {
        m_thread->stop();
        while (m_thread->queuedBuffers())
            processBuffer();
        delete m_thread;
        m_thread = nullptr;
    }

    delete m_decoder;
    m_decoder = nullptr;

    delete m_dialog;
    m_dialog = nullptr;

    // flush away all prerecording buffers
    m_prerecording_queue.clear();

    // enable undo again if we recorded something
    if (!signalManager().isEmpty())
        signalManager().enableUndo();

    return list;
}

//***************************************************************************
void Kwave::RecordPlugin::notice(QString message)
{
    Q_ASSERT(m_dialog);
    if (m_dialog) m_dialog->message(message);
}

//***************************************************************************
void Kwave::RecordPlugin::closeDevice()
{
    if (m_retry_timer.isActive()) m_retry_timer.stop();

    if (m_device) {
        m_device->close();
        delete m_device;
        m_device = nullptr;
    }
}

//***************************************************************************
void Kwave::RecordPlugin::setMethod(Kwave::record_method_t method)
{
    Q_ASSERT(m_dialog);
    if (!m_dialog) return;

    InhibitRecordGuard _lock(*this); // don't record while settings change
    qDebug("RecordPlugin::setMethod(%d)", static_cast<int>(method));

    // change the recording method (class RecordDevice)
    if ((method != m_method) || !m_device) {
        delete m_device;
        m_device = nullptr;
        bool searching = false;

        // use the previous device
        QString section = _("plugin ") + name();
        KConfigGroup cfg = KSharedConfig::openConfig()->group(section);

        // restore the previous device
        QString device = cfg.readEntry(
            _("last_device_%1").arg(static_cast<int>(method)));
//          qDebug("<<< %d -> '%s'", static_cast<int>(method), device.data());
        m_device_name = device;

        do {
            switch (method) {
#ifdef HAVE_OSS_SUPPORT
                case Kwave::RECORD_OSS:
                    m_device = new(std::nothrow) Kwave::RecordOSS();
                    Q_ASSERT(m_device);
                    break;
#endif /* HAVE_OSS_SUPPORT */

#ifdef HAVE_ALSA_SUPPORT
                case Kwave::RECORD_ALSA:
                    m_device = new(std::nothrow) Kwave::RecordALSA();
                    Q_ASSERT(m_device);
                    break;
#endif /* HAVE_ALSA_SUPPORT */

#ifdef HAVE_PULSEAUDIO_SUPPORT
                case Kwave::RECORD_PULSEAUDIO:
                    m_device = new(std::nothrow) Kwave::RecordPulseAudio();
                    Q_ASSERT(m_device);
                    break;
#endif /* HAVE_PULSEAUDIO_SUPPORT */

#ifdef HAVE_QT_AUDIO_SUPPORT
                case Kwave::RECORD_QT:
                    m_device = new(std::nothrow) Kwave::RecordQt();
                    Q_ASSERT(m_device);
                    break;
#endif /* HAVE_QT_AUDIO_SUPPORT */
                default:
                    qDebug("unsupported recording method (%d)",
                        static_cast<int>(method));
                    if (!searching) {
                        // start trying all other methods
                        searching = true;
                        method = Kwave::RECORD_NONE;
                        ++method;
                        continue;
                    } else {
                        // try next method
                        ++method;
                    }
                    qDebug("unsupported recording method - trying next (%d)",
                           static_cast<int>(method));
                    if (method != Kwave::RECORD_INVALID) continue;
            }
            break;
        } while (true);
    }
    Q_ASSERT(m_device);

    // if we found no recording method
    if (method == Kwave::RECORD_INVALID) {
        qWarning("found no valid recording method");
    }

    // take the change in the method
    m_method = method;

    // activate the cange in the dialog
    m_dialog->setMethod(method);

    // set list of supported devices
    QStringList supported_devices;
    Q_ASSERT(m_device);
    if (m_device) supported_devices = m_device->supportedDevices();
    m_dialog->setSupportedDevices(supported_devices);

    // set current device (again), no matter if supported or not,
    // the dialog will take care of this.
    setDevice(m_device_name);

    // check the filter for the "select..." dialog. If it is
    // empty, the "select" dialog will be disabled
    QString file_filter;
    if (m_device) file_filter = m_device->fileFilter();
    m_dialog->setFileFilter(file_filter);
}

//***************************************************************************
void Kwave::RecordPlugin::retryOpen()
{
    qDebug("RecordPlugin::retryOpen()");
    setDevice(m_device_name);
}

//***************************************************************************
void Kwave::RecordPlugin::setDevice(const QString &device)
{
    Q_ASSERT(m_dialog);
    Q_ASSERT(m_device);
    if (!m_dialog || !m_device) return;

    InhibitRecordGuard _lock(*this); // don't record while settings change
    qDebug("RecordPlugin::setDevice('%s')", DBG(device));

    if (m_retry_timer.isActive()) m_retry_timer.stop();

    // select the default device if this one is not supported
    QString dev = device;
    QStringList supported = m_device->supportedDevices();
    if (!supported.isEmpty() && !supported.contains(device)) {
        // use the first entry as default
        dev = supported.first();
        qDebug("RecordPlugin::setDevice(%s) -> fallback to '%s'",
               DBG(device), DBG(dev));
    }

    // if there was no valid device name, fall back to default device
    if (dev.startsWith(_("#"))) {
        dev = _("/dev/dsp");
        qDebug("RecordPlugin::setDevice(%s) -> no valid device, using '%s'",
               DBG(device), DBG(dev));
    }

    // open and initialize the device
    QString result = m_device->open(dev);

    // set the device in the dialog
    m_device_name = dev;
    m_dialog->setDevice(dev);

    // remember the device selection, just for the GUI
    // for the next change in the method
    QString section = _("plugin ") + name();
    KConfigGroup cfg = KSharedConfig::openConfig()->group(section);
    cfg.writeEntry(_("last_device_%1").arg(
        static_cast<int>(m_method)), m_device_name);
//     qDebug(">>> %d -> '%s'", static_cast<int>(m_method), DBG(m_device_name));
    cfg.sync();

    if (!result.isNull()) {
        bool shouldRetry = false;

        qWarning("RecordPlugin::setDevice('%s'): "
                 "opening the device failed. error message='%s'",
                 DBG(device), DBG(result));

        m_controller.setInitialized(false);

        if (m_device_name.length()) {
            // build a short device name for showing to the user
            QString short_device_name = m_device_name;
            if (m_device_name.contains(_("|"))) {
                // tree syntax: extract card + device
                short_device_name = m_device_name.section(_("|"), 0, 0);
                if (m_device_name.section(_("|"), 3, 3).length())
                    short_device_name += _(", ") +
                        m_device_name.section(_("|"), 3, 3);
            }

            bool errIsNumeric = false;
            int errNumber = result.toInt(&errIsNumeric);
            if (errIsNumeric) {
                if (errNumber == ENODEV) {
                    result = i18n(
                        "Maybe your system lacks support for the "\
                        "corresponding hardware or the hardware is not "\
                        "connected."
                    );
                } else if (errNumber == EBUSY) {
                    result = i18n(
                        "The audio device seems to be occupied by another "\
                        "application. Retrying..."
                    );
                    shouldRetry = true;
                } else {
                    result = i18n(
                        "Some unexpected error happened (%1). "\
                        "You may try an other recording method or "\
                        "recording device.",
                        QString::fromLocal8Bit(strerror(errNumber))
                    );
                }
            }

            if (result.length()) {
                if (shouldRetry) {
                    notice(result);
                } else {
                    m_dialog->showDevicePage();
                    Kwave::MessageBox::sorry(parentWidget(),
                        result, i18nc("%1 = a device name",
                        "Unable to open the recording device (%1)",
                        short_device_name));
                }
            }
        }

        if (shouldRetry) {
            // retry later...
            m_retry_timer.start(OPEN_RETRY_TIME);
        } else {
            m_device_name = QString();
            changeTracks(0);
        }
    } else {
        changeTracks(m_dialog->params().tracks);
    }

    if (paramsValid()) {
        m_controller.setInitialized(true);
    } else {
        qDebug("RecordPlugin::setDevice('%s') failed, "
                "returning to 'UNINITIALIZED'", DBG(device));
        m_controller.setInitialized(false);
    }

}

//***************************************************************************
void Kwave::RecordPlugin::changeTracks(unsigned int new_tracks)
{
    Q_ASSERT(m_dialog);
    if (!m_dialog) return;

    InhibitRecordGuard _lock(*this); // don't record while settings change
//     qDebug("RecordPlugin::changeTracks(%u)", new_tracks);

    if (!m_device || m_device_name.isNull()) {
        // no device -> dummy/shortcut
        m_dialog->setSupportedTracks(0, 0);
        m_dialog->setTracks(0);
        changeSampleRate(0);
        return;
    }

    // check the supported tracks
    unsigned int min = 0;
    unsigned int max = 0;
    if ((m_device->detectTracks(min, max) < 0) || (max < 1))
        min = max = 0;
    if (min > max) min = max;

    unsigned int channels = new_tracks;
    if ((channels < min) || (channels > max)) {
        // clip to the supported number of tracks
        if (channels < min) channels = min;
        if (channels > max) channels = max;
        qDebug("RecordPlugin::changeTracks(%u) -> clipped to %u",
               new_tracks, channels);

        if ((new_tracks && channels) && (new_tracks != channels)) {
            QString s1;
            switch (new_tracks) {
                case 1: s1 = i18n("Mono");   break;
                case 2: s1 = i18n("Stereo"); break;
                case 4: s1 = i18n("Quadro"); break;
                default:
                    s1 = i18n("%1 channels", new_tracks);
            }
            QString s2;
            switch (channels) {
                case 1: s2 = i18n("Mono");   break;
                case 2: s2 = i18n("Stereo"); break;
                case 4: s2 = i18n("Quadro"); break;
                default:
                    s2 = i18n("%1 channels", channels);
            }

            notice(i18n("%1 is not supported, using %2", s1, s2));
        }
    }
    Q_ASSERT(channels >= min);
    Q_ASSERT(channels <= max);
    m_dialog->setSupportedTracks(min, max);

    // try to activate the new number of tracks
    int err = m_device->setTracks(channels);
    if (err < 0) {
        // revert to the current device setting if failed
        int t = m_device->tracks();
        if (t > 0) {
            // current device state seems to be valid
            channels = t;
            if (channels < min) channels = min;
            if (channels > max) channels = max;
        } else {
            // current device state is invalid
            channels = 0;
        }

        if (new_tracks && (channels > 0)) notice(
            i18n("Recording with %1 channel(s) failed, "\
                 "using %2 channel(s)", new_tracks, channels));
    }
    m_dialog->setTracks(channels);

    // activate the new sample rate
    changeSampleRate(m_dialog->params().sample_rate);
}

//***************************************************************************
void Kwave::RecordPlugin::changeSampleRate(double new_rate)
{
    Q_ASSERT(m_dialog);
    if (!m_dialog) return;

    InhibitRecordGuard _lock(*this); // don't record while settings change
//     qDebug("RecordPlugin::changeSampleRate(%u)", Kwave::toInt(new_rate));

    if (!m_device || m_device_name.isNull()) {
        // no device -> dummy/shortcut
        m_dialog->setSampleRate(0);
        changeCompression(Kwave::Compression::INVALID);
        return;
    }

    // check the supported sample rates
    QList<double> supported_rates = m_device->detectSampleRates();
    bool is_supported = false;
    foreach (const double &r, supported_rates)
        if (qFuzzyCompare(new_rate, r)) { is_supported = true; break; }
    double rate = new_rate;
    if (!is_supported && !supported_rates.isEmpty()) {
        // find the nearest sample rate
        double nearest = supported_rates.last();
        foreach (double r, supported_rates) {
            if (fabs(r - rate) <= fabs(nearest - rate))
                nearest = r;
        }
        rate = nearest;

        const QString sr1(m_dialog->rate2string(new_rate));
        const QString sr2(m_dialog->rate2string(rate));
        if ((Kwave::toInt(new_rate) > 0) &&
            (Kwave::toInt(rate) > 0) &&
            (Kwave::toInt(new_rate) != Kwave::toInt(rate)))
            notice(i18n("%1 Hz is not supported, "\
                        "using %2 Hz", sr1, sr2));
    }
    m_dialog->setSupportedSampleRates(supported_rates);

    // try to activate the new sample rate
    int err = m_device->setSampleRate(rate);
    if (err < 0) {
        // revert to the current device setting if failed
        rate = m_device->sampleRate();
        if (rate < 0) rate = 0;

        const QString sr1(m_dialog->rate2string(new_rate));
        const QString sr2(m_dialog->rate2string(rate));
        if ((Kwave::toInt(new_rate) > 0) &&
            (Kwave::toInt(rate) > 0) &&
            (Kwave::toInt(new_rate) != Kwave::toInt(rate)))
            notice(i18n("%1 Hz failed, using %2 Hz", sr1, sr2));
    }
    m_dialog->setSampleRate(rate);

    // set the compression again
    changeCompression(m_dialog->params().compression);
}

//***************************************************************************
void Kwave::RecordPlugin::changeCompression(
    Kwave::Compression::Type new_compression
)
{
    Q_ASSERT(m_dialog);
    if (!m_dialog) return;

    InhibitRecordGuard _lock(*this); // don't record while settings change
//     qDebug("RecordPlugin::changeCompression(%d)", new_compression);

    if (!m_device || m_device_name.isNull()) {
        // no device -> dummy/shortcut
        m_dialog->setCompression(-1);
        changeBitsPerSample(0);
        return;
    }

    // check the supported compressions
    QList<Kwave::Compression::Type> supported_comps =
        m_device->detectCompressions();
    Kwave::Compression::Type compression = new_compression;
    if (!supported_comps.contains(compression) &&
        (compression != Kwave::Compression::NONE))
    {
        // try to disable the compression (type 0)
        compression = Kwave::Compression::NONE;
        if (!supported_comps.isEmpty() &&
            !supported_comps.contains(compression))
        {
            // what now, "None" is not supported
            // -> take the first supported one
            compression = supported_comps[0];
        }

        if (compression != new_compression) {
            const QString c1(Kwave::Compression(new_compression).name());
            const QString c2(Kwave::Compression(compression).name());
            notice(i18n("Compression '%1' not supported, using '%2'", c1, c2));
        }
    }
    m_dialog->setSupportedCompressions(supported_comps);

    // try to activate the new compression
    int err = m_device->setCompression(compression);
    if (err < 0) {
        // revert to the current device setting if failed
        if (compression != m_device->compression()) {
            const QString c1(Kwave::Compression(compression).name());
            const QString c2(Kwave::Compression(m_device->compression()
                             ).name());
            notice(i18n("Compression '%1' failed, using '%2'.", c1 ,c2));
        }
        compression = m_device->compression();
    }
    m_dialog->setCompression(compression);

    // set the resolution in bits per sample again
    changeBitsPerSample(m_dialog->params().bits_per_sample);
}

//***************************************************************************
void Kwave::RecordPlugin::changeBitsPerSample(unsigned int new_bits)
{
    Q_ASSERT(m_dialog);
    if (!m_dialog) return;

    InhibitRecordGuard _lock(*this); // don't record while settings change
//     qDebug("RecordPlugin::changeBitsPerSample(%d)", new_bits);

    if (!m_device || m_device_name.isNull()) {
        // no device -> dummy/shortcut
        m_dialog->setBitsPerSample(0);
        changeSampleFormat(Kwave::SampleFormat::Unknown);
        return;
    }

    // check the supported resolution in bits per sample
    QList<unsigned int> supported_bits = m_device->supportedBits();
    int bits = new_bits;
    if (!supported_bits.contains(bits) && !supported_bits.isEmpty()) {
        // find the nearest resolution
        int nearest = supported_bits.last();
        foreach (unsigned int b, supported_bits) {
            if (qAbs(Kwave::toInt(b) - nearest) <= qAbs(bits - nearest))
                nearest = Kwave::toInt(b);
        }
        bits = nearest;

        if ((Kwave::toInt(new_bits) > 0) && (bits > 0)) notice(
            i18n("%1 bits per sample is not supported, "\
                 "using %2 bits per sample",
                 Kwave::toInt(new_bits), bits));
    }
    m_dialog->setSupportedBits(supported_bits);

    // try to activate the resolution
    int err = m_device->setBitsPerSample(bits);
    if (err < 0) {
        // revert to the current device setting if failed
        bits = m_device->bitsPerSample();
        if (bits < 0) bits = 0;
        if ((new_bits > 0) && (bits > 0)) notice(
            i18n("%1 bits per sample failed, "
                 "using %2 bits per sample",
                 Kwave::toInt(new_bits), bits));
    }
    m_dialog->setBitsPerSample(bits);

    // set the sample format again
    changeSampleFormat(m_dialog->params().sample_format);
}

//***************************************************************************
void Kwave::RecordPlugin::changeSampleFormat(
    Kwave::SampleFormat::Format new_format)
{
    Q_ASSERT(m_dialog);
    if (!m_dialog) return;

    InhibitRecordGuard _lock(*this); // don't record while settings change

    if (!m_device || m_device_name.isNull()) {
        // no device -> dummy/shortcut
        m_dialog->setSampleFormat(Kwave::SampleFormat::Unknown);
        return;
    }

    // check the supported sample formats
    QList<Kwave::SampleFormat::Format> supported_formats =
        m_device->detectSampleFormats();
    Kwave::SampleFormat::Format format = new_format;
    if (!supported_formats.contains(format) && !supported_formats.isEmpty()) {
        // use the device default instead
        format = m_device->sampleFormat();

        // if this was also not supported -> stupid device !?
        if (!supported_formats.contains(format)) {
            format = supported_formats.first(); // just take the first one :-o
        }

        Kwave::SampleFormat::Map sf;
        const QString s1 = sf.description(sf.findFromData(new_format), true);
        const QString s2 = sf.description(sf.findFromData(format), true);
        if (!(new_format == -1) && !(new_format == format)) {
            notice(i18n("Sample format '%1' is not supported, "\
                        "using '%2'", s1, s2));
        }
    }
    m_dialog->setSupportedSampleFormats(supported_formats);

    // try to activate the new format
    int err = m_device->setSampleFormat(format);
    if (err < 0) {
        // use the device default instead
        format = m_device->sampleFormat();

        Kwave::SampleFormat::Map sf;
        const QString s1 = sf.description(sf.findFromData(new_format), true);
        const QString s2 = sf.description(sf.findFromData(format), true);
        if (format > 0) notice(
            i18n("Sample format '%1' failed, using '%2'", s1, s2));
    }
    m_dialog->setSampleFormat(format);
}

//***************************************************************************
void Kwave::RecordPlugin::buffersChanged()
{
    InhibitRecordGuard _lock(*this); // don't record while settings change
    // this implicitly activates the new settings
}

//***************************************************************************
void Kwave::RecordPlugin::enterInhibit()
{
    m_inhibit_count++;
    if ((m_inhibit_count == 1) && m_thread) {
        // set hourglass cursor
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

//      qDebug("RecordPlugin::enterInhibit() - STOPPING");
        m_thread->stop();
        Q_ASSERT(!m_thread->isRunning());

        // de-queue all buffers that are still in the queue
        while (m_thread->queuedBuffers())
            processBuffer();
    }
}

//***************************************************************************
void Kwave::RecordPlugin::leaveInhibit()
{
    Q_ASSERT(m_inhibit_count);
    Q_ASSERT(m_dialog);

    if (m_inhibit_count) m_inhibit_count--;

    while (!m_inhibit_count && paramsValid()) {
//      qDebug("RecordPlugin::leaveInhibit() - STARTING ("
//             "%d channels, %d bits)",
//             m_dialog->params().tracks,
//             m_dialog->params().bits_per_sample);

        Q_ASSERT(!m_thread->isRunning());
        if (m_thread->isRunning()) break;

        // set new parameters for the recorder
        setupRecordThread();

        // and let the thread run (again)
        m_thread->start();
        break;
    }

    // take back the hourglass cursor
    if (!m_inhibit_count) QApplication::restoreOverrideCursor();
}

//***************************************************************************
bool Kwave::RecordPlugin::paramsValid()
{
    if (!m_thread || !m_device || !m_dialog) return false;

    // check for a valid/usable record device
    if (m_device_name.isNull()) return false;
    if ( (m_device->sampleFormat() != Kwave::SampleFormat::Unsigned) &&
         (m_device->sampleFormat() != Kwave::SampleFormat::Signed) )
        return false;
    if (m_device->bitsPerSample() < 1) return false;
    if (m_device->endianness() == Kwave::UnknownEndian) return false;

    // check for valid parameters in the dialog
    const Kwave::RecordParams &params = m_dialog->params();
    if (params.tracks < 1) return false;
    if ( (params.sample_format != Kwave::SampleFormat::Unsigned) &&
         (params.sample_format != Kwave::SampleFormat::Signed) ) return false;

    return true;
}

//***************************************************************************
void Kwave::RecordPlugin::resetRecording(bool &accepted)
{
    InhibitRecordGuard _lock(*this);

    if (m_writers) m_writers->clear();

    emitCommand(_("nomacro:close()"));
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    accepted = manager().signalManager().isEmpty();
    if (!accepted) return;

    // the parent context might have changed, maybe we have to
    // re-parent this plugin instance!
    migrateToActiveContext();

    m_buffers_recorded = 0;

    m_controller.setEmpty(true);
    emit sigRecordedSamples(0);
}

//***************************************************************************
void Kwave::RecordPlugin::setupRecordThread()
{
    Q_ASSERT(m_thread);
    Q_ASSERT(m_dialog);
    Q_ASSERT(m_device);
    if (!paramsValid()) return;

    // stop the thread if necessary (should never happen)
    Q_ASSERT(!m_thread->isRunning());
    if (m_thread->isRunning()) m_thread->stop();
    Q_ASSERT(!m_thread->isRunning());

    // delete the previous decoder
    delete m_decoder;
    m_decoder = nullptr;

    // our own reference to the record parameters
    const Kwave::RecordParams &params = m_dialog->params();
    if (!paramsValid()) return;

    // create a decoder for the current sample format
    switch (params.compression) {
        case Kwave::Compression::NONE:
            switch (params.sample_format) {
                case Kwave::SampleFormat::Unsigned: /* FALLTHROUGH */
                case Kwave::SampleFormat::Signed:
                    // decoder for all linear formats
                    m_decoder = new(std::nothrow) Kwave::SampleDecoderLinear(
                        m_device->sampleFormat(),
                        m_device->bitsPerSample(),
                        m_device->endianness()
                    );
                    break;
                default:
                    notice(
                        i18n("The current sample format is not supported!")
                    );
            }
            break;
        default:
            notice(
                i18n("The current compression type is not supported!")
            );
            return;
    }

    Q_ASSERT(m_decoder);
    if (!m_decoder) {
        Kwave::MessageBox::sorry(m_dialog, i18n("Out of memory"));
        return;
    }

    // set up the prerecording queues
    m_prerecording_queue.clear();
    if (params.pre_record_enabled) {
        // prepare a queue for each track
        const unsigned int prerecording_samples = Kwave::toUint(
            rint(params.pre_record_time * params.sample_rate));
        m_prerecording_queue.resize(params.tracks);
        for (int i=0; i < m_prerecording_queue.size(); i++)
            m_prerecording_queue[i].setSize(prerecording_samples);

        if (m_prerecording_queue.size() != Kwave::toInt(params.tracks)) {
            m_prerecording_queue.clear();
            Kwave::MessageBox::sorry(m_dialog, i18n("Out of memory"));
            return;
        }
    }

    // set up the recording trigger values
    m_trigger_value.resize(params.tracks);
    m_trigger_value.fill(0.0);

    // set up the record thread
    m_thread->setRecordDevice(m_device);
    unsigned int buf_count = params.buffer_count;
    unsigned int buf_size  = params.tracks *
                             m_decoder->rawBytesPerSample() *
                            (1 << params.buffer_size);
    m_thread->setBuffers(buf_count, buf_size);
}

//***************************************************************************
void Kwave::RecordPlugin::startRecording()
{
    Q_ASSERT(m_dialog);
    Q_ASSERT(m_thread);
    Q_ASSERT(m_device);
    if (!m_dialog || !m_thread || !m_device) return;

    InhibitRecordGuard _lock(*this); // don't record while settings change

    if ((m_state != Kwave::REC_PAUSED) || !m_decoder) {
        double rate = m_dialog->params().sample_rate;
        unsigned int tracks = m_dialog->params().tracks;
        unsigned int bits = m_dialog->params().bits_per_sample;

        if (!tracks) return;

        /*
         * if tracks or sample rate has changed
         * -> start over with a new signal and new settings
         */
        if ((!m_writers) ||
            (m_writers->tracks() != tracks) || !qFuzzyCompare(
             Kwave::FileInfo(signalManager().metaData()).rate(), rate))
        {
            // create a new and empty signal
            emitCommand(QString(_("newsignal(0,%1,%2,%3)")).arg(
                rate).arg(bits).arg(tracks));
            QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

            // the parent context might have changed, maybe we have to
            // re-parent this plugin instance!
            migrateToActiveContext();

            Kwave::SignalManager &mgr = signalManager();
            if (!qFuzzyCompare(mgr.rate(), rate) || (mgr.bits() != bits) ||
                (mgr.tracks() != tracks))
            {
                emitCommand(_("close"));
                return;
            }

            // we do not need undo while recording, this would only waste undo
            // buffers with modified/inserted data
            signalManager().disableUndo();

            // create a sink for our audio data
            delete m_writers;
            m_writers = new(std::nothrow) Kwave::MultiTrackWriter(
                signalManager(), Kwave::Append);
            if ((!m_writers) || (m_writers->tracks() != tracks)) {
                Kwave::MessageBox::sorry(m_dialog, i18n("Out of memory"));
                return;
            }
        } else {
            // re-use the current signal and append to it
        }

        // initialize the file information
        Kwave::FileInfo fileInfo(signalManager().metaData());
        fileInfo.setRate(rate);
        fileInfo.setBits(bits);
        fileInfo.setTracks(tracks);
        fileInfo.set(Kwave::INF_MIMETYPE, _("audio/vnd.wave"));
        fileInfo.set(Kwave::INF_SAMPLE_FORMAT,
            Kwave::SampleFormat(m_dialog->params().sample_format).toInt());
        fileInfo.set(Kwave::INF_COMPRESSION, m_dialog->params().compression);

        // add our Kwave Software tag
        const KAboutData about_data = KAboutData::applicationData();
        QString software = about_data.componentName() + _("-") +
                           about_data.version() + _(" ") +
                           i18n("(built with KDE Frameworks %1)",
                           _(KXMLGUI_VERSION_STRING));
        fileInfo.set(Kwave::INF_SOFTWARE, software);

        // add a date tag, ISO format
        QString date(QDate::currentDate().toString(_("yyyy-MM-dd")));
        fileInfo.set(Kwave::INF_CREATION_DATE, date);
        signalManager().setFileInfo(fileInfo, false);
    }

    // now the recording can be considered to be started
    m_controller.deviceRecordStarted();
}

//***************************************************************************
void Kwave::RecordPlugin::recordStopped(int reason)
{
    qDebug("RecordPlugin::recordStopped(%d)", reason);
    if (reason >= 0) return; // nothing to do

    // recording was aborted
    QString err_msg;
    switch (reason) {
        case -ENOBUFS:
            err_msg = i18n("Buffer overrun. Please increase the "\
                            "number and/or size of the record buffers.");
            break;
        case -EBUSY:
            err_msg = i18n("The recording device seems to be busy.");
            break;
        default:
            err_msg = i18n("Reading from the recording device failed. "\
                           "Error number = %1 (%2)", -reason,
                            QString::fromLocal8Bit(strerror(-reason)));
    }
    Kwave::MessageBox::error(m_dialog, err_msg);

    if (m_writers) m_writers->flush();
    qDebug("RecordPlugin::recordStopped(): last=%lu",
           static_cast<unsigned long int>(
           (m_writers) ? m_writers->last() : 0));

    // flush away all prerecording buffers
    m_prerecording_queue.clear();

    // update the file info if we recorded something
    // NOTE: this implicitly sets the "modified" flag of the signal
    if (m_writers && m_writers->last()) {
        Kwave::FileInfo info(signalManager().metaData());
        info.setLength(signalLength());
        info.setTracks(m_dialog->params().tracks);
        signalManager().setFileInfo(info, false);
    }

}

//***************************************************************************
void Kwave::RecordPlugin::stateChanged(Kwave::RecordState state)
{
    m_state = state;
    switch (m_state) {
        case Kwave::REC_PAUSED:
            if (m_writers) m_writers->flush();
            break;
        case Kwave::REC_UNINITIALIZED:
        case Kwave::REC_EMPTY:
        case Kwave::REC_DONE:
            // reset buffer status
            if (m_writers) {
                m_writers->flush();
                delete m_writers;
                m_writers = nullptr;
            }
            m_buffers_recorded = 0;
            m_dialog->updateBufferState(0, 0);
            break;
        default:
            ;
    }
}

//***************************************************************************
void Kwave::RecordPlugin::updateBufferProgressBar()
{
    Q_ASSERT(m_dialog);
    Q_ASSERT(m_thread);
    if (!m_dialog || !m_thread) return;

    unsigned int buffers_total = m_dialog->params().buffer_count;

    // if we are still recording: update the progress bar
    if ((m_state != Kwave::REC_EMPTY) && (m_state != Kwave::REC_PAUSED) &&
        (m_state != Kwave::REC_DONE))
    {
        // count up the number of recorded buffers
        m_buffers_recorded++;

        if (m_buffers_recorded <= buffers_total) {
            // buffers are just in progress of getting filled
            m_dialog->updateBufferState(m_buffers_recorded, buffers_total);
        } else {
            // we have remaining+1 buffers (one is currently filled)
            unsigned int remaining = m_thread->remainingBuffers() + 1;
            if (remaining > buffers_total) remaining = buffers_total;
            m_dialog->updateBufferState(remaining, buffers_total);
        }
    } else {
        // no longer recording: count the buffer downwards
        unsigned int queued = m_thread->queuedBuffers();
        if (!queued) buffers_total = 0;
        m_dialog->updateBufferState(queued, buffers_total);
    }
}

//***************************************************************************
void Kwave::RecordPlugin::split(QByteArray &raw_data, QByteArray &dest,
                                unsigned int bytes_per_sample,
                                unsigned int track,
                                unsigned int tracks)
{
    unsigned int raw_data_size = static_cast<unsigned int>(raw_data.size());
    unsigned int samples = (raw_data_size / bytes_per_sample) / tracks;

#if 0
    // simple sawtooth generator, based on raw data
    // works for up to 16 channels
    static int saw[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    raw_data.fill(0x05);
    for (unsigned int s = 0; s < samples; s++) {
        int v = saw[track];
        for (unsigned int byte = 0; byte < bytes_per_sample; byte++) {
            quint8 x = (quint8)v;
            raw_data[(((s * tracks) + track) * bytes_per_sample) + byte] = x;
            v >>= 8;
        }

        const int max = (1 << ((bytes_per_sample * 8) - 1)) - 1;
        saw[track] += max / 64;
        if (saw[track] >= max) saw[track] = 0;
    }
#endif

    if (tracks == 1) {
        // this would give a 1:1 memcpy
        dest = raw_data;
    } else {
        switch (bytes_per_sample) {
            case 1: {
                // 1...8 bits per sample, use 8 bit pointers
                const quint8 *src =
                    reinterpret_cast<const quint8 *>(raw_data.constData());
                quint8 *dst =
                    reinterpret_cast<quint8 *>(dest.data());
                src += track;
                while (samples) {
                    *dst = *src;
                    dst++;
                    src += tracks;
                    samples--;
                }
                break;
            }
            case 2: {
                // 9...16 bits per sample, use 16 bit pointers
                const quint16 *src =
                    reinterpret_cast<const quint16 *>(raw_data.constData());
                quint16 *dst =
                    reinterpret_cast<quint16 *>(dest.data());
                src += track;
                while (samples) {
                    *dst = *src;
                    dst++;
                    src += tracks;
                    samples--;
                }
                break;
            }
            case 3: {
                // 17...24 bits per sample, use 8 bit pointers, three times
                const quint8 *src =
                    reinterpret_cast<const quint8 *>(raw_data.constData());
                quint8 *dst =
                    reinterpret_cast<quint8 *>(dest.data());
                src += track * 3;
                while (samples) {
                    *(dst++) = *(src++);
                    *(dst++) = *(src++);
                    *(dst++) = *(src++);
                    src += (tracks - 1) * 3;
                    samples--;
                }
                break;
            }
            case 4: {
                // 24...32 bits per sample, use 32 bit pointers
                const quint32 *src =
                    reinterpret_cast<const quint32 *>(raw_data.constData());
                quint32 *dst =
                    reinterpret_cast<quint32 *>(dest.data());
                src += track;
                while (samples) {
                    *dst = *src;
                    dst++;
                    src += tracks;
                    samples--;
                }
                break;
            }
            case 8: {
                // 64 bits per sample, use 64 bit pointers
                const quint64 *src =
                    reinterpret_cast<const quint64 *>(raw_data.constData());
                quint64 *dst =
                    reinterpret_cast<quint64 *>(dest.data());
                src += track;
                while (samples) {
                    *dst = *src;
                    dst++;
                    src += tracks;
                    samples--;
                }
                break;
            }
            default: {
                // default: byte wise operation
                const quint8 *src =
                    reinterpret_cast<const quint8 *>(raw_data.constData());
                quint8 *dst =
                    reinterpret_cast<quint8 *>(dest.data());
                src += (track * bytes_per_sample);
                unsigned int increment = (tracks - 1) * bytes_per_sample;
                while (samples) {
                    for (unsigned int b = 0; b < bytes_per_sample; b++) {
                        *dst = *src;
                        dst++;
                        src++;
                        samples--;
                    }
                    src += increment;
                }
            }
        }
    }
}

//***************************************************************************
bool Kwave::RecordPlugin::checkTrigger(unsigned int track,
                                       const Kwave::SampleArray &buffer)
{
    Q_ASSERT(m_dialog);
    if (!m_dialog) return false;

    // check if the recording start time has been reached
    if (m_dialog->params().start_time_enabled) {
        if (QDateTime::currentDateTime() < m_dialog->params().start_time)
            return false;
    }

    // shortcut if no trigger has been set
    if (!m_dialog->params().record_trigger_enabled) return true;

    // check the input parameters
    if (!buffer.size()) return false;
    if (!m_writers) return false;
    if (m_trigger_value.size() != Kwave::toInt(m_writers->tracks()))
        return false;

    // pass the buffer through a rectifier and a lowpass with
    // center frequency about 2Hz to get the amplitude
    float trigger = static_cast<float>(
        m_dialog->params().record_trigger / 100.0);
    float rate = static_cast<float>(
        m_dialog->params().sample_rate);

    /*
     * simple lowpass calculation:
     *
     *               1 + z
     * H(z) = a0 * -----------   | z = e ^ (j*2*pi*f)
     *               z + b1
     *
     *        1            1 - n
     * a0 = -----    b1 = --------
     *      1 + n          1 + n
     *
     * Fg = fg / fa
     *
     * n = cot(Pi * Fg)
     *
     * y[t] = a0 * x[t] + a1 * x[t-1] - b1 * y[t-1]
     *
     */

    // rise coefficient: ~20Hz
    const float f_rise = 20.0f;
    float Fg = f_rise / rate;
    float n = 1.0f / tanf(float(M_PI) * Fg);
    const float a0_r = 1.0f / (1.0f + n);
    const float b1_r = (1.0f - n) / (1.0f + n);

    // fall coefficient: ~1.0Hz
    const float f_fall = 1.0f;
    Fg = f_fall / rate;
    n = 1.0f / tanf(float(M_PI) * Fg);
    const float a0_f = 1.0f / (1.0f + n);
    const float b1_f = (1.0f - n) / (1.0f + n);

    float y = m_trigger_value[track];
    float last_x = y;
    for (unsigned int t = 0; t < buffer.size(); ++t) {
        float x = fabsf(sample2float(buffer[t])); /* rectifier */

        if (x > y) { /* diode */
            // rise if amplitude is above average (serial R)
            y = (a0_r * x) + (a0_r * last_x) - (b1_r * y);
        }

        // fall (parallel R)
        y = (a0_f * x) + (a0_f * last_x) - (b1_f * y);

        // remember x[t-1]
        last_x = x;

// nice for debugging:
//      buffer[t] = (int)((double)(1 << (SAMPLE_BITS-1)) * y);
        if (y > trigger) return true;
    }
    m_trigger_value[track] = y;

    qDebug(">> level=%5.3g, trigger=%5.3g", y, trigger);

    return false;
}

//***************************************************************************
void Kwave::RecordPlugin::enqueuePrerecording(unsigned int track,
                                              const Kwave::SampleArray &decoded)
{
    Q_ASSERT(m_dialog);
    Q_ASSERT(Kwave::toInt(track) < m_prerecording_queue.size());
    if (!m_dialog) return;
    if (Kwave::toInt(track) >= m_prerecording_queue.size()) return;

    // append the array with decoded sample to the prerecording buffer
    m_prerecording_queue[track].put(decoded);
}

//***************************************************************************
void Kwave::RecordPlugin::flushPrerecordingQueue()
{
    if (!m_prerecording_queue.size()) return;
    Q_ASSERT(m_dialog);
    Q_ASSERT(m_thread);
    Q_ASSERT(m_decoder);
    if (!m_dialog || !m_thread || !m_decoder) return;

    const Kwave::RecordParams &params = m_dialog->params();
    const unsigned int tracks = params.tracks;
    Q_ASSERT(tracks);
    if (!tracks) return;
    Q_ASSERT(m_writers);
    if (!m_writers) return;
    Q_ASSERT(tracks == m_writers->tracks());
    if (tracks != m_writers->tracks()) return;

    for (unsigned int track=0; track < tracks; ++track) {
        Kwave::SampleFIFO &fifo = m_prerecording_queue[track];
        Q_ASSERT(fifo.length());
        if (!fifo.length()) continue;
        fifo.crop(); // enforce the correct size

        // push all buffers to the writer, starting at the tail
        Kwave::Writer *writer = (*m_writers)[track];
        Q_ASSERT(writer);
        if (writer) {
            Kwave::SampleArray buffer(writer->blockSize());
            unsigned int rest = fifo.length();
            while (rest) {
                unsigned int read = fifo.get(buffer);
                if (read < 1) break;
                writer->write(buffer, read);
                rest -= read;
            }
        } else {
            // fallback: discard the FIFO content
            fifo.flush();
        }
        Q_ASSERT(fifo.length() == 0);
    }

    // the queues are no longer needed
    m_prerecording_queue.clear();

    // we have transferred data to the writers, we are no longer empty
    m_controller.setEmpty(false);
}

//***************************************************************************
void Kwave::RecordPlugin::processBuffer()
{
    bool recording_done = false;

    // de-queue the buffer from the thread
    if (!m_thread) return;
    if (!m_thread->queuedBuffers()) return;
    QByteArray buffer = m_thread->dequeue();

    // abort here if we have no dialog or no decoder
    if (!m_dialog || !m_decoder) return;

    // we received a buffer -> update the progress bar
    updateBufferProgressBar();

    const Kwave::RecordParams &params = m_dialog->params();
    const unsigned int tracks = params.tracks;
    Q_ASSERT(tracks);
    if (!tracks) return;

    const unsigned int bytes_per_sample = m_decoder->rawBytesPerSample();
    Q_ASSERT(bytes_per_sample);
    if (!bytes_per_sample) return;

    unsigned int buffer_size = static_cast<unsigned int>(buffer.size());
    unsigned int samples = (buffer_size / bytes_per_sample) / tracks;
    Q_ASSERT(samples);
    if (!samples) return;

    // check for reached recording time limit if enabled
    if (params.record_time_limited && m_writers) {
        sample_index_t last = m_writers->last();
        sample_index_t already_recorded = (last) ? (last + 1) : 0;
        sample_index_t limit = static_cast<sample_index_t>(rint(
            params.record_time * params.sample_rate));
        if (already_recorded + samples >= limit) {
            // reached end of recording time, we are full
            if (m_state == Kwave::REC_RECORDING) {
                samples = Kwave::toUint(
                    (limit > already_recorded) ?
                    (limit - already_recorded) : 0);
                buffer.resize(samples * tracks * bytes_per_sample);
            }
            recording_done = true;
        }
    }

    QByteArray buf;
    buf.resize(bytes_per_sample * samples);
    Q_ASSERT(buf.size() == Kwave::toInt(bytes_per_sample * samples));
    if (buf.size() != Kwave::toInt(bytes_per_sample * samples)) return;

    Kwave::SampleArray decoded(samples);
    Q_ASSERT(decoded.size() == samples);
    if (decoded.size() != samples) return;

    // check for trigger
    // note: this might change the state, which affects the
    //       processing of all tracks !
    if ((m_state == Kwave::REC_WAITING_FOR_TRIGGER) ||
        ((m_state == Kwave::REC_PRERECORDING) &&
          params.record_trigger_enabled) ||
        ((m_state == Kwave::REC_PRERECORDING) &&
          params.start_time_enabled))
    {
        for (unsigned int track=0; track < tracks; ++track) {
            // split off and decode buffer with current track
            split(buffer, buf, bytes_per_sample, track, tracks);
            m_decoder->decode(buf, decoded);
            if (checkTrigger(track, decoded)) {
                m_controller.deviceTriggerReached();
                break;
            }
        }
    }

    if ((m_state == Kwave::REC_RECORDING) && !m_prerecording_queue.isEmpty()) {
        // flush all prerecorded buffers to the output
        flushPrerecordingQueue();
    }

    // use a copy of the state, in case it changes below ;-)
    Kwave::RecordState state = m_state;
    for (unsigned int track = 0; track < tracks; ++track) {
        // decode and care for all special effects, meters and so on
        // split off and decode buffer with current track
        split(buffer, buf, bytes_per_sample, track, tracks);
        m_decoder->decode(buf, decoded);

        // update the level meter and other effects
        m_dialog->updateEffects(track, decoded);

        // if the first buffer is full -> leave REC_BUFFERING
        // limit state transitions to a point before the first track is
        // processed (avoid asymmetry)
        if ((track == 0) && (m_state == Kwave::REC_BUFFERING) &&
            (m_buffers_recorded > 1))
        {
            m_controller.deviceBufferFull();
            state = m_state; // might have changed!
        }

        switch (state) {
            case Kwave::REC_UNINITIALIZED:
            case Kwave::REC_EMPTY:
            case Kwave::REC_PAUSED:
            case Kwave::REC_DONE:
            case Kwave::REC_BUFFERING:
            case Kwave::REC_WAITING_FOR_TRIGGER:
                // already handled before or nothing to do...
                break;
            case Kwave::REC_PRERECORDING:
                // enqueue the buffers into a FIFO
                enqueuePrerecording(track, decoded);
                break;
            case Kwave::REC_RECORDING: {
                // put the decoded track data into the buffer
                if (!m_writers) break; // (could happen due to queued signal)
                Q_ASSERT(tracks == m_writers->tracks());
                if (!tracks || (tracks != m_writers->tracks())) break;

                Kwave::Writer *writer = (*m_writers)[track];
                Q_ASSERT(writer);
                if (writer) (*writer) << decoded;
                m_controller.setEmpty(false);

                break;
            }
            DEFAULT_IMPOSSIBLE;
        }
    }

    // update the number of recorded samples
    if (m_writers) emit sigRecordedSamples(m_writers->last() + 1);

    // if this was the last received buffer, change state
    if (recording_done &&
        (m_state != Kwave::REC_DONE) &&
        (m_state != Kwave::REC_EMPTY))
    {
        m_controller.actionStop();
    }

}

//***************************************************************************
void Kwave::RecordPlugin::prerecordingChanged(bool enable)
{
    Q_UNUSED(enable)
    InhibitRecordGuard _lock(*this); // activate the change
}

//***************************************************************************
#include "RecordPlugin.moc"
//***************************************************************************
//***************************************************************************

#include "moc_RecordPlugin.cpp"
