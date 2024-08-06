/***************************************************************************
     PlayBackPlugin.cpp  -  plugin for playback and playback configuration
                             -------------------
    begin                : Sun May 13 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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
#include <unistd.h>

#include <new>

#include <QApplication>
#include <QCursor>
#include <QFile>
#include <QLatin1Char>
#include <QMutex>
#include <QMutexLocker>
#include <QPointer>
#include <QProgressDialog>
#include <QString>
#include <QTimer>

#include <KConfig>

#include "libkwave/Connect.h"
#include "libkwave/Curve.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MultiPlaybackSink.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/MultiTrackSource.h"
#include "libkwave/PlayBackDevice.h"
#include "libkwave/PlayBackTypesMap.h"
#include "libkwave/Plugin.h"
#include "libkwave/PluginManager.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SignalManager.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"

#include "libkwave/modules/CurveStreamAdapter.h"
#include "libkwave/modules/Delay.h"
#include "libkwave/modules/Mul.h"
#include "libkwave/modules/Osc.h"

#include "PlayBack-ALSA.h"
#include "PlayBack-OSS.h"
#include "PlayBack-PulseAudio.h"
#include "PlayBack-Qt.h"

#include "PlayBackDialog.h"
#include "PlayBackPlugin.h"

KWAVE_PLUGIN(playback, PlayBackPlugin)

/** test frequency [Hz] */
#define PLAYBACK_TEST_FREQUENCY 440.0

//***************************************************************************
Kwave::PlayBackPlugin::PlayBackPlugin(QObject *parent,
                                      const QVariantList &args)
    :Kwave::Plugin(parent, args),
     m_dialog(Q_NULLPTR),
     m_playback_controller(manager().playbackController()),
     m_playback_sink(Q_NULLPTR)
{
}

//***************************************************************************
Kwave::PlayBackPlugin::~PlayBackPlugin()
{
    // make sure the dialog is gone
    if (m_dialog) delete m_dialog;
    m_dialog = Q_NULLPTR;

    Q_ASSERT(!m_playback_sink);
}

//***************************************************************************
Kwave::PlayBackParam Kwave::PlayBackPlugin::interpreteParameters(
    QStringList &params)
{
    Kwave::PlayBackParam playback_params;
    Kwave::PlayBackParam default_params;
    bool ok;
    QString param;

    // evaluate the parameter list
    if (params.count() != 5) return default_params;

    // parameter #0: playback method
    param = params[0];
    unsigned int method = param.toUInt(&ok);
    Q_ASSERT(ok);
    if (!ok) return default_params;
    if (method >= Kwave::PLAYBACK_INVALID) method = Kwave::PLAYBACK_NONE;
    playback_params.method = static_cast<Kwave::playback_method_t>(method);

    // parameter #1: playback device [/dev/dsp , ... ]
    param = params[1];
    playback_params.device = param;

    // parameter #2: number of channels [1 | 2]
    param = params[2];
    playback_params.channels = param.toUInt(&ok);
    Q_ASSERT(ok);
    if (!ok) return default_params;

    // parameter #3: bits per sample [8 | 16 ]
    param = params[3];
    playback_params.bits_per_sample = param.toUInt(&ok);
    Q_ASSERT(ok);
    if (!ok) return default_params;

    // parameter #4: base of buffer size [4...16]
    param = params[4];
    playback_params.bufbase = param.toUInt(&ok);
    Q_ASSERT(ok);
    if (!ok) return default_params;

    return playback_params;
}

//***************************************************************************
void Kwave::PlayBackPlugin::load(QStringList &params)
{
    use(); // stay loaded

    // register as a factory for playback devices
    m_playback_controller.registerPlaybackDeviceFactory(this);
    m_playback_controller.setDefaultParams(interpreteParameters(params));
}

/***************************************************************************/
void Kwave::PlayBackPlugin::unload()
{
    // unregister from the list playback factories
    m_playback_controller.unregisterPlaybackDeviceFactory(this);
    release();
}

//***************************************************************************
QStringList *Kwave::PlayBackPlugin::setup(QStringList &previous_params)
{
    QStringList *result = Q_NULLPTR;

    // try to interpret the list of previous parameters, ignore errors
    Kwave::PlayBackParam playback_params =
        interpreteParameters(previous_params);

    Q_ASSERT(!m_dialog);
    if (m_dialog) delete m_dialog;

    m_dialog = new(std::nothrow) Kwave::PlayBackDialog(
        *this,
        manager().playbackController(),
        playback_params
    );
    Q_ASSERT(m_dialog);
    if (!m_dialog) return Q_NULLPTR;

    connect(m_dialog, SIGNAL(sigTestPlayback()),
            this, SLOT(testPlayBack()));

    // activate the playback method
    m_dialog->setMethod(playback_params.method);

    if ((m_dialog->exec() == QDialog::Accepted) && m_dialog) {
        // get the new parameters and let them take effect
        result = new(std::nothrow) QStringList();
        Q_ASSERT(result);
        if (result) {
            QString param;

            playback_params = m_dialog->params();

            // parameter #0: playback method
            param = param.setNum(
                static_cast<unsigned int>(playback_params.method));
            result->append(param);

            // parameter #1: playback device [/dev/dsp , ... ]
            param = playback_params.device;
            result->append(param);

            // parameter #2: number of channels [1, 2, ... n]
            param = param.setNum(playback_params.channels);
            result->append(param);

            // parameter #3: bits per sample [8, 16, 24, ...]
            param = param.setNum(playback_params.bits_per_sample);
            result->append(param);

            // parameter #4: base of buffer size [8 ... 16]
            param = param.setNum(playback_params.bufbase);
            result->append(param);

            qDebug("new playback params: '%s",
                   DBG(result->join(_("','")) + _("'")));

            // take over the new playback parameters
            signalManager().playbackController().setDefaultParams(
                playback_params
            );
        }
    }

    delete m_dialog;
    m_dialog = Q_NULLPTR;

    return result;
}

//***************************************************************************
QList<Kwave::playback_method_t> Kwave::PlayBackPlugin::supportedMethods()
{
    QList<Kwave::playback_method_t> methods;

#ifdef HAVE_QT_AUDIO_SUPPORT
    methods.append(Kwave::PLAYBACK_QT_AUDIO);
#endif /* HAVE_QT_AUDIO_SUPPORT */

#ifdef HAVE_PULSEAUDIO_SUPPORT
    methods.append(Kwave::PLAYBACK_PULSEAUDIO);
#endif /* HAVE_PULSEAUDIO_SUPPORT */

#ifdef HAVE_ALSA_SUPPORT
    methods.append(Kwave::PLAYBACK_ALSA);
#endif /* HAVE_ALSA_SUPPORT */

#ifdef HAVE_OSS_SUPPORT
    methods.append(Kwave::PLAYBACK_OSS);
#endif /* HAVE_OSS_SUPPORT */

    return methods;
}

//***************************************************************************
Kwave::PlayBackDevice *Kwave::PlayBackPlugin::createDevice(
    Kwave::playback_method_t method)
{
    Kwave::PlayBackTypesMap methods;
    qDebug("PlayBackPlugin::createDevice('%s' [%d])",
           DBG(methods.name(methods.findFromData(method))),
           static_cast<int>(method) );

    switch (method) {
#ifdef HAVE_QT_AUDIO_SUPPORT
        case Kwave::PLAYBACK_QT_AUDIO:
            return new(std::nothrow) Kwave::PlayBackQt();
#endif /* HAVE_QT_AUDIO_SUPPORT */

#ifdef HAVE_PULSEAUDIO_SUPPORT
        case Kwave::PLAYBACK_PULSEAUDIO:
            return new(std::nothrow) Kwave::PlayBackPulseAudio(
                Kwave::FileInfo(signalManager().metaData()));
#endif /* HAVE_PULSEAUDIO_SUPPORT */

#ifdef HAVE_ALSA_SUPPORT
        case Kwave::PLAYBACK_ALSA:
            return new(std::nothrow) Kwave::PlayBackALSA();
#endif /* HAVE_ALSA_SUPPORT */

#ifdef HAVE_OSS_SUPPORT
        case Kwave::PLAYBACK_OSS:
            return new(std::nothrow) Kwave::PlayBackOSS();
#endif /* HAVE_OSS_SUPPORT */

        default:
            break;
    }

    return Q_NULLPTR; // nothing found :-(
}

//***************************************************************************
void Kwave::PlayBackPlugin::run(QStringList params)
{
    const double       t_sweep =   1.0; /* seconds per speaker */
    const unsigned int periods =     3; /* number of periods to play */

    Q_UNUSED(params)

    Q_ASSERT(m_dialog);
    Q_ASSERT(m_playback_sink);
    if (!m_dialog || !m_playback_sink) return;
    Kwave::PlayBackParam playback_params = m_dialog->params();

    unsigned int channels = playback_params.channels;
    double rate           = playback_params.rate;
    Q_ASSERT(channels);
    Q_ASSERT(rate > 1.0);
    if (!channels || (rate <= 1.0)) return;

    // settings are valid -> take them

    double t_period = t_sweep * channels;
    unsigned int curve_length = Kwave::toUint(t_period * rate);

    // create all objects
    Kwave::Curve curve;
    curve.insert(0.0, 0.0);
    if (channels < 2) {
        // mono
        curve.insert(0.5, 1.0);
    } else {
        // all above
        curve.insert(0.5 / static_cast<double>(channels), 1.0);
        curve.insert(1.0 / static_cast<double>(channels), 0.0);
    }
    curve.insert(1.0, 0.0);

    Kwave::CurveStreamAdapter curve_adapter(curve, curve_length);
    connect(this, SIGNAL(sigCancel()), &curve_adapter, SLOT(cancel()),
            Qt::DirectConnection);

    Kwave::MultiTrackSource<Kwave::Delay, true> delay(channels);
    for (unsigned int i = 0; i < channels; i++) {
        Q_ASSERT(delay[i]);
        if (!delay[i]) break;
        delay[i]->setAttribute(SLOT(setDelay(QVariant)),
            QVariant(i * t_sweep * rate));
    }

    Kwave::Osc osc;
    osc.setAttribute(SLOT(setFrequency(QVariant)),
                     QVariant(rate / PLAYBACK_TEST_FREQUENCY));
    connect(this, SIGNAL(sigCancel()), &osc, SLOT(cancel()),
            Qt::DirectConnection);

    Kwave::MultiTrackSource<Kwave::Mul, true> mul(channels);

    // connect everything together...
    //
    // curve -> delay --.
    //                  |
    //                  v
    //                 mul -> sink
    //                  ^
    //                  |
    //            osc --'

    Kwave::connect(
        curve_adapter,    SIGNAL(output(Kwave::SampleArray)),
        delay,            SLOT(input(Kwave::SampleArray)));
    Kwave::connect(
        delay,            SIGNAL(output(Kwave::SampleArray)),
        mul,              SLOT(input_a(Kwave::SampleArray)));
    Kwave::connect(
        osc,              SIGNAL(output(Kwave::SampleArray)),
        mul,              SLOT(input_b(Kwave::SampleArray)));
    Kwave::connect(
        mul,              SIGNAL(output(Kwave::SampleArray)),
        *m_playback_sink, SLOT(input(Kwave::SampleArray)));

    // show a progress dialog

    // transport the samples
    sample_index_t samples_max     = static_cast<sample_index_t>(
                                     periods * t_period * rate);
    sample_index_t samples_written = 0;
    while (!shouldStop() && (samples_written <= samples_max)) {
        osc.goOn();
        curve_adapter.goOn();
        delay.goOn();
        mul.goOn();

        samples_written += osc.blockSize();

        double percent = (static_cast<double>(samples_written) * 100.0) /
                          static_cast<double>(samples_max);
        emit sigTestProgress(Kwave::toInt(percent));
    }
}

//***************************************************************************
void Kwave::PlayBackPlugin::testPlayBack()
{
    qDebug("PlayBackPlugin::testPlayBack()");

    Q_ASSERT(m_dialog);
    if (!m_dialog) return;
    Kwave::PlayBackParam playback_params = m_dialog->params();

    // check if we really have selected a playback device
    if (!playback_params.device.length()) {
        Kwave::MessageBox::sorry(m_dialog, i18n(
            "Please select a playback device first"));
        return;
    }

    unsigned int channels = playback_params.channels;
    double rate           = playback_params.rate;
    if (!channels || (rate <= 1.0)) return;

    // settings are valid -> take them

    // create the multi track playback sink
    // NOTE: this must be done in main thread context!
    Q_ASSERT(!m_playback_sink);
    if (m_playback_sink) return;
    m_playback_sink = manager().openMultiTrackPlayback(
        channels,
        &playback_params
    );
    if (!m_playback_sink) return;
    m_playback_sink->setInteractive(true);

    // show a progress dialog
    QPointer<QProgressDialog> progress =
        new(std::nothrow) QProgressDialog(m_dialog);
    Q_ASSERT(progress);
    if (progress) {
        progress->setWindowTitle(i18n("Playback Test"));
        progress->setModal(true);
        progress->setMinimumDuration(0);
        progress->setMinimum(0);
        progress->setMaximum(100);
        progress->setAutoClose(false);
        progress->setValue(0);
        progress->setLabelText(
            _("<html><p><br>") +
            i18n("You should now hear a %1 Hz test tone.<br/><br/>"
                    "(If you hear clicks or dropouts, please increase<br/>"
                    "the buffer size and try again)",
                    Kwave::toInt(PLAYBACK_TEST_FREQUENCY)) +
            _("</p></html>")
        );
        connect(progress, SIGNAL(canceled()), this, SLOT(cancel()),
                Qt::QueuedConnection);
        connect(this, SIGNAL(sigDone(Kwave::Plugin*)), progress, SLOT(close()),
                Qt::QueuedConnection);
        connect(this, SIGNAL(sigTestProgress(int)),
                progress, SLOT(setValue(int)),
                Qt::QueuedConnection);

        QStringList params;
        execute(params);
        progress->exec();
        cancel();
    }

    // set hourglass cursor, waiting for shutdown could take some time...
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    // wait through manual polling here, no timeout
    qDebug("waiting...");
    while (isRunning()) {
        cancel();
        sleep(1);
        qDebug(".");
    }
    qDebug("done.");

    // close the playback sink again (here in main thread context)
    m_playback_sink->setInteractive(false);
    delete m_playback_sink;
    m_playback_sink = Q_NULLPTR;

    // free the progress dialog
    delete progress;

    // stop the worker thread through the Runnable API
    stop();

    // remove hourglass
    QApplication::restoreOverrideCursor();
}

//***************************************************************************
#include "PlayBackPlugin.moc"
//***************************************************************************
//***************************************************************************

#include "moc_PlayBackPlugin.cpp"
