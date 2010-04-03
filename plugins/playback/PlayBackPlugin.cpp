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

#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <QCursor>
#include <QFile>
#include <QMutex>
#include <QProgressDialog>
#include <QString>
#include <QDateTime>

#include <kapplication.h>
#include <kconfig.h>

#include "libkwave/Curve.h"
#include "libkwave/KwaveConnect.h"
#include "libkwave/KwaveMultiPlaybackSink.h"
#include "libkwave/KwaveMultiTrackSource.h"
#include "libkwave/KwavePlugin.h"
#include "libkwave/Matrix.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/PlaybackController.h"
#include "libkwave/PlayBackDevice.h"
#include "libkwave/PluginManager.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SignalManager.h"

#include "libkwave/modules/CurveStreamAdapter.h"
#include "libkwave/modules/KwaveDelay.h"
#include "libkwave/modules/KwaveMul.h"
#include "libkwave/modules/KwaveOsc.h"

#include "PlayBack-OSS.h"
#include "PlayBack-ALSA.h"
#include "PlayBack-Phonon.h"
#include "PlayBack-PulseAudio.h"

#include "PlayBackDialog.h"
#include "PlayBackPlugin.h"

KWAVE_PLUGIN(PlayBackPlugin, "playback", "2.2",
             I18N_NOOP("Playback"), "Thomas Eschenbacher");

/** Sets the number of screen refreshes per second when in playback mode */
#define SCREEN_REFRESHES_PER_SECOND 16

//***************************************************************************
PlayBackPlugin::PlayBackPlugin(const PluginContext &context)
    :Kwave::Plugin(context), m_dialog(0),
    m_device(0), m_lock_device(), m_playback_params(),
    m_playback_controller(manager().playbackController()),
    m_old_first(0),
    m_old_last(0)
{
#ifdef HAVE_ALSA_SUPPORT
    // set builtin defaults to ALSA
    m_playback_params.method = PLAYBACK_ALSA;
    m_playback_params.device = "default";
#endif /* HAVE_ALSA_SUPPORT */

    connect(this, SIGNAL(sigPlaybackDone()),
            &m_playback_controller, SLOT(playbackDone()));
    connect(this, SIGNAL(sigPlaybackPos(sample_index_t)),
            &m_playback_controller, SLOT(updatePlaybackPos(sample_index_t)));
    connect(this, SIGNAL(sigPlaybackDone()),
            this, SLOT(closeDevice()),
            Qt::QueuedConnection);

    // register as a factory for playback devices
    manager().registerPlaybackDeviceFactory(this);
}

//***************************************************************************
PlayBackPlugin::~PlayBackPlugin()
{
    // make sure the dialog is gone
    if (m_dialog) delete m_dialog;
    m_dialog = 0;

    // unregister from the list playback factories
    manager().unregisterPlaybackDeviceFactory(this);

    // close the device now if it accidentally is still open
    QMutexLocker lock_for_delete(&m_lock_device);
    if (m_device) delete m_device;
    m_device = 0;
}

//***************************************************************************
int PlayBackPlugin::interpreteParameters(QStringList &params)
{
    bool ok;
    QString param;

    // evaluate the parameter list
    if (params.count() != 5) return -EINVAL;

    // parameter #0: playback method
    param = params[0];
    unsigned int method = param.toUInt(&ok);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;
    if (method >= PLAYBACK_INVALID) method = PLAYBACK_NONE;
    m_playback_params.method = static_cast<playback_method_t>(method);

    // parameter #1: playback device [/dev/dsp , ... ]
    param = params[1];
    m_playback_params.device = param;

    // parameter #2: number of channels [1 | 2]
    param = params[2];
    m_playback_params.channels = param.toUInt(&ok);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    // parameter #3: bits per sample [8 | 16 ]
    param = params[3];
    m_playback_params.bits_per_sample = param.toUInt(&ok);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    // parameter #4: base of buffer size [4...16]
    param = params[4];
    m_playback_params.bufbase = param.toUInt(&ok);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    return 0;
}

//***************************************************************************
void PlayBackPlugin::load(QStringList &params)
{
    interpreteParameters(params);

    connect(&m_playback_controller, SIGNAL(sigDeviceStartPlayback()),
            this, SLOT(startDevicePlayBack()));
    connect(&m_playback_controller, SIGNAL(sigDeviceStopPlayback()),
            this, SLOT(stopDevicePlayBack()));
}

//***************************************************************************
QStringList *PlayBackPlugin::setup(QStringList &previous_params)
{
    // sorry, cannot do setup while the playback is running
    if (isRunning()) {
	Kwave::MessageBox::sorry(parentWidget(),
	    i18n("Playback is currently running."
	         "Please stop playback first and then try again"),
	    i18n("Sorry"));
	return 0;
    }

    QStringList *result = 0;

    // try to interprete the list of previous parameters, ignore errors
    if (previous_params.count()) interpreteParameters(previous_params);

    Q_ASSERT(!m_dialog);
    if (m_dialog) delete m_dialog;

    m_dialog = new PlayBackDialog(*this, m_playback_params);
    Q_ASSERT(m_dialog);
    if (!m_dialog) return 0;

    connect(m_dialog, SIGNAL(sigMethodChanged(playback_method_t)),
            this, SLOT(setMethod(playback_method_t)));
    connect(m_dialog, SIGNAL(sigDeviceChanged(const QString &)),
            this, SLOT(setDevice(const QString &)));
    connect(m_dialog, SIGNAL(sigTestPlayback()),
            this, SLOT(testPlayBack()));

    // activate the playback method
    setMethod(m_playback_params.method);

    if (m_dialog->exec() == QDialog::Accepted) {
	// get the new parameters and let them take effect
	result = new QStringList();
	Q_ASSERT(result);
	if (result) {
	    QString param;

	    m_playback_params = m_dialog->params();

	    // parameter #0: playback method
	    param = param.setNum(
	        static_cast<unsigned int>(m_playback_params.method));
	    result->append(param);

	    // parameter #1: playback device [/dev/dsp , ... ]
	    param = m_playback_params.device;
	    result->append(param);

	    // parameter #2: number of channels [1, 2, ... n]
	    param = param.setNum(m_playback_params.channels);
	    result->append(param);

	    // parameter #3: bits per sample [8, 16, 24, ...]
	    param = param.setNum(m_playback_params.bits_per_sample);
	    result->append(param);

	    // parameter #4: base of buffer size [8 ... 16]
	    param = param.setNum(m_playback_params.bufbase);
	    result->append(param);

	    qDebug("playback >>> '%s", QString(result->join("','") +
		"'").toLocal8Bit().data());
	}
    }

    delete m_dialog;
    m_dialog = 0;

    return result;
}

//***************************************************************************
bool PlayBackPlugin::supportsDevice(const QString &name)
{
    (void)name;
    // always return true, we are currently the one and only playback
    // device factory
    return true;
}

//***************************************************************************
PlayBackDevice *PlayBackPlugin::createDevice(playback_method_t &method)
{
    bool searching = false;
    do {
	switch (method) {

#ifdef HAVE_OSS_SUPPORT
	    case PLAYBACK_OSS:
		return new PlayBackOSS();
#endif /* HAVE_OSS_SUPPORT */

#ifdef HAVE_ALSA_SUPPORT
	    case PLAYBACK_ALSA:
		return new PlayBackALSA();
#endif /* HAVE_ALSA_SUPPORT */

#ifdef HAVE_PHONON_SUPPORT
	    case PLAYBACK_PHONON:
		return new PlayBackPhonon();
#endif /* HAVE_PHONON_SUPPORT */

#ifdef HAVE_PULSEAUDIO_SUPPORT
	    case PLAYBACK_PULSEAUDIO:
		return new PlayBackPulseAudio(signalManager().fileInfo());
#endif /* HAVE_PULSEAUDIO_SUPPORT */

	    default:
		qDebug("unsupported playback method (%d)",
		    static_cast<int>(method));
		if (!searching) {
		    // start trying out all other methods
		    searching = true;
		    method = PLAYBACK_NONE;
		    ++method;
		    continue;
		} else {
		    // try next method
		    ++method;
		}
		qDebug("unsupported playback method - trying next (%d)",
		    static_cast<int>(method));
		if (method != PLAYBACK_INVALID) continue;
	}
	break;
    } while (true);

    return 0; // nothing found :-(
}

//***************************************************************************
void PlayBackPlugin::setMethod(playback_method_t method)
{
    qDebug("PlayBackPlugin::setMethod(%d)", static_cast<int>(method));
    QString last_device_name = m_playback_params.device;

    // set hourglass cursor
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    // change the playback method (class PlayBackDevice)
    if ((method != m_playback_params.method) || !m_device) {
	if (m_device) delete m_device;
	m_device = 0;

	// remember the device selection, just for the GUI
	// for the next time this method gets selected
	// change in method -> save the current device and use
	// the previous one
	QString device = "";
	QString section = "plugin "+name();
	KConfigGroup cfg = KGlobal::config()->group(section);
	if (m_dialog) {

	    // save the current device
	    cfg.writeEntry(QString("last_device_%1").arg(
		static_cast<int>(m_playback_params.method)),
		m_dialog->params().device);
	    qDebug(">>> %d -> '%s'",
	           static_cast<int>(m_playback_params.method),
	           m_dialog->params().device.toLocal8Bit().data());
	    cfg.sync();
	}

	// NOTE: the "method" may get modified here if not supported!
	m_device = createDevice(method);

	if (m_dialog) {
	    // restore the previous one
	    device = cfg.readEntry(QString("last_device_%1").arg(
		static_cast<int>(method)));
	    qDebug("<<< %d -> '%s'", static_cast<int>(method),
		device.toLocal8Bit().data());
	    last_device_name = device;
	    m_playback_params.device = device;
	    m_dialog->setSupportedDevices(QList<QString>());
 	    m_dialog->setDevice(m_playback_params.device);
	}
    }
    Q_ASSERT(m_device);

    // if we found no playback method
    if (method == PLAYBACK_INVALID) {
	qWarning("found no valid playback method");
    }

    // take the change in the method
    m_playback_params.method = method;

    // activate the change in the dialog
    if (m_dialog) m_dialog->setMethod(method);

    // set list of supported devices
    QStringList supported_devices;
    Q_ASSERT(m_device);
    if (m_device) supported_devices = m_device->supportedDevices();
    if (m_dialog) m_dialog->setSupportedDevices(supported_devices);

    // set current device (again), no matter if supported or not,
    // the dialog will take care of this.
    setDevice(last_device_name);

    // check the filter for the "select..." dialog. If it is
    // empty, the "select" dialog will be disabled
    QString file_filter;
    if (m_device) file_filter = m_device->fileFilter();
    if (m_dialog) m_dialog->setFileFilter(file_filter);

    // remove hourglass
    QApplication::restoreOverrideCursor();
}

//***************************************************************************
void PlayBackPlugin::setDevice(const QString &device)
{
//     qDebug("PlayBackPlugin::setDevice(%s)", device.toLocal8Bit().data());

    // select the default device if new one is not supported
    QString dev = device;
    if (m_device) {
	QStringList supported = m_device->supportedDevices();
	supported.removeAll("#EDIT#");
	supported.removeAll("#SELECT#");
	supported.removeAll("#TREE#");
	if (!supported.isEmpty() && !supported.contains(device)) {
	    // use the first entry as default
	    dev = supported.first();
	    qDebug("PlayBackPlugin::setDevice(%s) -> fallback to '%s'",
		device.toLocal8Bit().data(),
		dev.toLocal8Bit().data());
	}
    }

    // set the device in the dialog
    if (m_dialog) m_dialog->setDevice(dev);
    m_playback_params.device = dev;

    QList<unsigned int> supported_bits;
    if (m_device) supported_bits = m_device->supportedBits(dev);
    if (m_dialog) m_dialog->setSupportedBits(supported_bits);

    unsigned int min = 0;
    unsigned int max = 0;
    if (m_device) m_device->detectChannels(dev, min, max);
    if (m_dialog) m_dialog->setSupportedChannels(min, max);
}

//***************************************************************************
PlayBackDevice *PlayBackPlugin::openDevice(const QString &name,
    int tracks, const PlayBackParam *playback_params)
{
    QString device_name = name;
    PlayBackParam params;
    PlayBackDevice *device = 0;

    qDebug("PlayBackPlugin::openDevice('%s',params)",name.toLocal8Bit().data());

    if (!playback_params) {
	// use default parameters if none given
	params          = m_playback_params;
	if (!signalManager().isClosed() && !signalManager().isEmpty()) {
	    params.rate     = signalManager().fileInfo().rate();
	    params.channels = selectedTracks().count();
	}
    } else {
	// use given parameters
	params = *playback_params;
    }

    // override the number of tracks if not negative
    if (tracks > 0) params.channels = tracks;

    // use default device if no name is given
    if (!device_name.length()) device_name = params.device;

    // create a new device
    device = createDevice(params.method);
    Q_ASSERT(device);
    if (!device) {
	qWarning("PlayBackPlugin::openDevice(): "\
		"creating device failed.");
	return 0;
    }

    // open and initialize the device
    QString result = device->open(
	params.device,
	params.rate,
	params.channels,
	params.bits_per_sample,
	params.bufbase
    );
    if (result.length()) {
	qWarning("PlayBackPlugin::openDevice(): "\
	        "opening the device failed.");

	// delete the device if it did not open
	delete device;
	device = 0;

	// show an error message box
	Kwave::MessageBox::error(parentWidget(), result,
	    i18n("Unable to open '%1'",
	    params.device.section('|',0,0)));
    }

    return device;
}

//***************************************************************************
void PlayBackPlugin::closeDevice()
{
    QMutexLocker lock_for_delete(&m_lock_device);

    if (!m_device) return; // already closed
    delete m_device;
    m_device = 0;
    m_old_first = 0;
    m_old_last = 0;
}

//***************************************************************************
void PlayBackPlugin::startDevicePlayBack()
{
    // set the real sample rate for playback from the signal itself
    m_playback_params.rate = signalRate();

    QMutexLocker lock_for_delete(&m_lock_device);

    // remove the old device if still one exists
    if (m_device) {
	qWarning("PlayBackPlugin::startDevicePlayBack(): "\
	         "removing stale instance");
	delete m_device;
	m_device = 0;
    }

    // open the device and abort if not possible
    qDebug("PlayBackPlugin::startDevicePlayBack(), device='%s'",
          m_playback_params.device.toLocal8Bit().data());
    m_device = openDevice(m_playback_params.device, -1, &m_playback_params);
    if (!m_device) {
	// simulate a "playback done" on errors
	emit sigPlaybackDone();
	return;
    }

    sample_index_t first;
    sample_index_t last;
    selection(0, &first, &last, false);

    if (m_playback_controller.paused()) {
	// continue after pause
	if ((m_old_first != first) || (m_old_last != last)) {
	    // selection has changed
	    if (first != last) {
		// something selected -> set new range
		m_playback_controller.setStartPos(first);
		m_playback_controller.setEndPos(last);

		unsigned int pos = m_playback_controller.currentPos();
		if ((pos < first) || (pos > last)) {
		    // completely new area selected, or the right margin
		    // has been moved before the current playback pointer
		    // -> play from start of new selection
		    m_playback_controller.updatePlaybackPos(first);
		}
	    } else {
		// nothing selected -> select all and move to position
		m_playback_controller.setStartPos(first);
		m_playback_controller.setEndPos(signalLength()-1);
	    }
	}
    } else {
	// determine first and last sample if not in paused mode"
	if (first == last) {
	    // nothing selected -> play from cursor position
	    m_playback_controller.setStartPos(first);
	    m_playback_controller.setEndPos(signalLength()-1);
	} else {
	    // play only in selection
	    m_playback_controller.setStartPos(first);
	    m_playback_controller.setEndPos(last);
	}
	m_playback_controller.updatePlaybackPos(first);
    }

    m_old_first = first;
    m_old_last = last;

    QStringList empty_list;
    execute(empty_list);
}

//***************************************************************************
void PlayBackPlugin::stopDevicePlayBack()
{
    cancel();
    if (!isRunning()) {
	qDebug("PlayBackPlugin::stopDevicePlayBack() - not running");
	emit sigPlaybackDone();
    }
    closeDevice();
}

//***************************************************************************
void PlayBackPlugin::run(QStringList)
{
    QMutexLocker lock(&m_lock_device);

    unsigned int first = m_playback_controller.startPos();
    unsigned int last  = m_playback_controller.endPos();
    unsigned int out_channels = m_playback_params.channels;

    // get the list of selected channels
    QList<unsigned int> audible_tracks = selectedTracks();
    unsigned int audible_count = audible_tracks.count();
    if (!audible_count || !m_device) {
	// not even one selected track or no (open) device
	qDebug("PlayBackPlugin::run(): no audible track(s) !");
	emit sigPlaybackDone();
	return;
    }

    // set up a set of sample reader (streams)
    MultiTrackReader input(
	Kwave::FullSnapshot,
	signalManager(), audible_tracks, first, last);

    // create a translation matrix for mixing up/down to the desired
    // number of output channels
    Matrix<double> matrix(audible_count, out_channels);
    unsigned int x, y;
    for (y = 0; y < out_channels; y++) {
	unsigned int m1, m2;
	m1 = y * audible_count;
	m2 = (y + 1) * audible_count;

	for (x = 0; x < audible_count; x++) {
	    unsigned int n1, n2;
	    n1 = x * out_channels;
	    n2 = n1 + out_channels;

	    // get the common area of [n1..n2] and [m1..m2]
	    unsigned int l = (n1 > m1) ? n1 : m1;
	    unsigned int r = (n2 < m2) ? n2 : m2;

	    matrix[x][y] = (r > l) ? static_cast<double>(r-l) /
		static_cast<double>(audible_count) : 0.0;
	}
    }

    // loop until process is stopped
    // or run once if not in loop mode
    Kwave::SampleArray in_samples(audible_count);
    Kwave::SampleArray out_samples(out_channels);
    unsigned int pos = m_playback_controller.currentPos();

    // counter for refresh of the playback position
    unsigned int pos_countdown = 0;

    emit sigPlaybackPos(pos);
    do {

	// if current position is after start -> skip the passed
	// samples (this happens when resuming after a pause)
	if (pos > first) {
	    for (x=0; x < audible_count; x++) {
		SampleReader *stream = input[x];
		if (stream) stream->skip(pos-first);
	    }
	}

	while ((pos++ <= last) && !shouldStop()) {
	    unsigned int x;
	    for (x=0; x < audible_count; x++) {
		in_samples[x] = 0;
		SampleReader *stream = input[x];
		Q_ASSERT(stream);
		if (!stream) continue;

		sample_t act;
		(*stream) >> act;
		in_samples[x] = act;
	    }

	    // multiply matrix with input to get output
	    unsigned int y;
	    for (y=0; y < out_channels; y++) {
		double sum = 0;
		for (x=0; x < audible_count; x++) {
		    sum += static_cast<double>(in_samples[x]) * matrix[x][y];
		}
		out_samples[y] = static_cast<sample_t>(sum);
	    }

	    // write samples to the playback device
	    int result = m_device->write(out_samples);
	    if (result) {
		cancel();
		pos = last;
	    }

	    // update the playback position if timer elapsed
	    if (!pos_countdown) {
		pos_countdown = static_cast<unsigned int>(ceil(
		    m_playback_params.rate / SCREEN_REFRESHES_PER_SECOND));
		emit sigPlaybackPos(pos);
	    } else {
		--pos_countdown;
	    }
	}

	// maybe we loop. in this case the playback starts
	// again from the left marker
	if (m_playback_controller.loop() && !shouldStop()) {
	    input.reset();
	    pos = m_playback_controller.startPos();
	}

    } while (m_playback_controller.loop() && !shouldStop());

    // playback is done
    emit sigPlaybackDone();
//    qDebug("PlayBackPlugin::run() done.");
}

//***************************************************************************
void PlayBackPlugin::testPlayBack()
{
    const float t_sweep        =   1.0; /* seconds per speaker */
    const double freq          = 440.0; /* test frequency [Hz] */
    const unsigned int periods =     3; /* number of periods to play */

    qDebug("PlayBackPlugin::testPlayBack()");

    Q_ASSERT(m_dialog);
    if (!m_dialog) return;
    PlayBackParam playback_params = m_dialog->params();

    // check if we really have selected a playback device
    if (!playback_params.device.length()) {
	Kwave::MessageBox::sorry(m_dialog, i18n(
	    "Please select a playback device first"));
	return;
    }

    unsigned int tracks = playback_params.channels;
    double rate         = playback_params.rate;
    Q_ASSERT(tracks);
    Q_ASSERT(rate > 1.0);
    if (!tracks || (rate <= 1.0)) return;

    // settings are valid -> take them
    m_playback_params = playback_params;

    // create the multi track playback sink
    Kwave::SampleSink *sink = manager().openMultiTrackPlayback(tracks);
    if (!sink) return;
    sink->setInteractive(true);

    float t_period = t_sweep * tracks;
    unsigned int curve_length = static_cast<unsigned int>(t_period * rate);

    // create all objects
    Curve curve;
    curve.insert(0.0, 0.0);
    if (tracks < 2) {
	// mono
	curve.insert(0.5, 1.0);
    } else {
	// all above
	curve.insert(0.5 / static_cast<float>(tracks), 1.0);
	curve.insert(1.0 / static_cast<float>(tracks), 0.0);
    }
    curve.insert(1.0, 0.0);

    Kwave::CurveStreamAdapter curve_adapter(curve, curve_length);

    Kwave::MultiTrackSource<Kwave::Delay, true> delay(tracks);
    for (unsigned int i=0; i < tracks; i++) {
	Q_ASSERT(delay[i]);
	if (!delay[i]) break;
	delay[i]->setAttribute(SLOT(setDelay(const QVariant)),
	    QVariant(i * t_sweep * rate));
    }

    Kwave::Osc osc;
    osc.setAttribute(SLOT(setFrequency(const QVariant)),
                     QVariant(rate / freq));

    Kwave::MultiTrackSource<Kwave::Mul, true> mul(tracks);

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
	curve_adapter, SIGNAL(output(Kwave::SampleArray)),
	delay,         SLOT(input(Kwave::SampleArray)));
    Kwave::connect(
	delay,         SIGNAL(output(Kwave::SampleArray)),
	mul,           SLOT(input_a(Kwave::SampleArray)));
    Kwave::connect(
	osc,           SIGNAL(output(Kwave::SampleArray)),
	mul,           SLOT(input_b(Kwave::SampleArray)));
    Kwave::connect(
	mul,           SIGNAL(output(Kwave::SampleArray)),
	*sink,         SLOT(input(Kwave::SampleArray)));

    // show a progress dialog
    QProgressDialog *progress = 0;
    if (m_dialog) {
	progress = new QProgressDialog(m_dialog);
	Q_ASSERT(progress);
	if (progress) {
	    progress->setWindowTitle(i18n("Playback Test"));
	    progress->setModal(true);
	    progress->setMinimumDuration(0);
	    progress->setMaximum(100);
	    progress->setAutoClose(true);
	    progress->setValue(1);
	    progress->setLabelText(
		"<html><p><br>" +
		i18n("You should now hear a %1 Hz test tone.<br><br>"\
		     "(If you hear clicks or dropouts, please increase<br>"\
		     "the buffer size and try again)", static_cast<int>(freq)) +
		"</p></html>"
	    );
	    progress->show();
	    QApplication::processEvents();
	}
    }

    // transport the samples
    QTime time;
    time.start();
    int t_max = periods * static_cast<int>(t_period) * 1000;
    while ((time.elapsed() < t_max) /*&& (!sink.done())*/) {

	osc.goOn();
	curve_adapter.goOn();
	delay.goOn();
	mul.goOn();

	if (progress) {
	    progress->setValue((100 * time.elapsed()) / t_max);
	    QApplication::processEvents();
	    if (progress->wasCanceled()) break;
	}
    }
    sink->setInteractive(false);

    if (progress) delete progress;
    delete sink;
}

//***************************************************************************
#include "PlayBackPlugin.moc"
//***************************************************************************
//***************************************************************************
