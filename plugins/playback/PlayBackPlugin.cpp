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
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <QtGui/QCursor>
#include <QtGui/QApplication>
#include <QtCore/QFile>
#include <QtCore/QLatin1Char>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtGui/QProgressDialog>
#include <QtCore/QString>
#include <QtCore/QTimer>

#include <kapplication.h>
#include <kconfig.h>

#include "libkwave/Curve.h"
#include "libkwave/Connect.h"
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

#include "PlayBack-OSS.h"
#include "PlayBack-ALSA.h"
#include "PlayBack-Phonon.h"
#include "PlayBack-PulseAudio.h"

#include "PlayBackDialog.h"
#include "PlayBackPlugin.h"

KWAVE_PLUGIN(Kwave::PlayBackPlugin, "playback", "2.3",
             I18N_NOOP("Playback"), "Thomas Eschenbacher");

/** test frequency [Hz] */
#define PLAYBACK_TEST_FREQUENCY 440.0

//***************************************************************************
Kwave::PlayBackPlugin::PlayBackPlugin(Kwave::PluginManager &plugin_manager)
    :Kwave::Plugin(plugin_manager),
     m_dialog(0),
     m_playback_controller(plugin_manager.playbackController()),
     m_playback_sink(0)
{
}

//***************************************************************************
Kwave::PlayBackPlugin::~PlayBackPlugin()
{
    // make sure the dialog is gone
    if (m_dialog) delete m_dialog;
    m_dialog = 0;

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
    QStringList *result = 0;

    // try to interprete the list of previous parameters, ignore errors
    Kwave::PlayBackParam playback_params =
	interpreteParameters(previous_params);

    Q_ASSERT(!m_dialog);
    if (m_dialog) delete m_dialog;

    m_dialog = new Kwave::PlayBackDialog(
	*this,
	manager().playbackController(),
	playback_params
    );
    Q_ASSERT(m_dialog);
    if (!m_dialog) return 0;

    connect(m_dialog, SIGNAL(sigTestPlayback()),
            this, SLOT(testPlayBack()));

    // activate the playback method
    m_dialog->setMethod(playback_params.method);

    if (m_dialog->exec() == QDialog::Accepted) {
	// get the new parameters and let them take effect
	result = new QStringList();
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
    m_dialog = 0;

    return result;
}

//***************************************************************************
QList<Kwave::playback_method_t> Kwave::PlayBackPlugin::supportedMethods()
{
    QList<Kwave::playback_method_t> methods;

#ifdef HAVE_PULSEAUDIO_SUPPORT
    methods.append(Kwave::PLAYBACK_PULSEAUDIO);
#endif /* HAVE_PULSEAUDIO_SUPPORT */

    #ifdef HAVE_PHONON_SUPPORT
    methods.append(Kwave::PLAYBACK_PHONON);
#endif /* HAVE_PHONON_SUPPORT */

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
#ifdef HAVE_PULSEAUDIO_SUPPORT
	case Kwave::PLAYBACK_PULSEAUDIO:
	    return new Kwave::PlayBackPulseAudio(
		Kwave::FileInfo(signalManager().metaData()));
#endif /* HAVE_PULSEAUDIO_SUPPORT */

#ifdef HAVE_PHONON_SUPPORT
	case Kwave::PLAYBACK_PHONON:
	    return new Kwave::PlayBackPhonon();
#endif /* HAVE_PHONON_SUPPORT */

#ifdef HAVE_ALSA_SUPPORT
	case Kwave::PLAYBACK_ALSA:
	    return new Kwave::PlayBackALSA();
#endif /* HAVE_ALSA_SUPPORT */

#ifdef HAVE_OSS_SUPPORT
	case Kwave::PLAYBACK_OSS:
	    return new Kwave::PlayBackOSS();
#endif /* HAVE_OSS_SUPPORT */

	default:
	    break;
    }

    return 0; // nothing found :-(
}

//***************************************************************************
void Kwave::PlayBackPlugin::run(QStringList)
{
    const float t_sweep        =   1.0; /* seconds per speaker */
    const unsigned int periods =     3; /* number of periods to play */

    qDebug("PlayBackPlugin::run()");

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

    float t_period = t_sweep * channels;
    unsigned int curve_length = static_cast<unsigned int>(t_period * rate);

    // create all objects
    Kwave::Curve curve;
    curve.insert(0.0, 0.0);
    if (channels < 2) {
	// mono
	curve.insert(0.5, 1.0);
    } else {
	// all above
	curve.insert(0.5 / static_cast<float>(channels), 1.0);
	curve.insert(1.0 / static_cast<float>(channels), 0.0);
    }
    curve.insert(1.0, 0.0);

    Kwave::CurveStreamAdapter curve_adapter(curve, curve_length);

    Kwave::MultiTrackSource<Kwave::Delay, true> delay(channels);
    for (unsigned int i = 0; i < channels; i++) {
	Q_ASSERT(delay[i]);
	if (!delay[i]) break;
	delay[i]->setAttribute(SLOT(setDelay(const QVariant)),
	    QVariant(i * t_sweep * rate));
    }

    Kwave::Osc osc;
    osc.setAttribute(SLOT(setFrequency(const QVariant)),
                     QVariant(rate / PLAYBACK_TEST_FREQUENCY));

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
    sample_index_t samples_max     = periods * t_period * rate;
    sample_index_t samples_written = 0;
    while (!shouldStop() && (samples_written <= samples_max)) {
	osc.goOn();
	curve_adapter.goOn();
	delay.goOn();
	mul.goOn();

	samples_written += osc.blockSize();

	int percent = (samples_written * 100) / samples_max;
	emit sigTestProgress(percent);
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
    Q_ASSERT(channels);
    Q_ASSERT(rate > 1.0);
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
    QProgressDialog *progress = 0;
    progress = new QProgressDialog(m_dialog);
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
	    i18n("You should now hear a %1 Hz test tone.<br><br>"
		    "(If you hear clicks or dropouts, please increase<br>"
		    "the buffer size and try again)",
		    static_cast<int>(PLAYBACK_TEST_FREQUENCY)) +
	    _("</p></html>")
	);
	connect(progress, SIGNAL(canceled()), this, SLOT(cancel()),
		Qt::QueuedConnection);
	connect(this, SIGNAL(sigDone(Kwave::Plugin *)), progress, SLOT(close()),
		Qt::QueuedConnection);
	connect(this, SIGNAL(sigTestProgress(int)), progress, SLOT(setValue(int)),
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
    m_playback_sink = 0;

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
