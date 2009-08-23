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

#include <QtGlobal>
#include <QCursor>
#include <QDateTime>
#include <QStringList>
#include <QList>
#include <QVariant>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kconfig.h>
#include <kglobal.h>

#include "libkwave/CompressionType.h"
#include "libkwave/FileInfo.h"
#include "libkwave/InsertMode.h"
#include "libkwave/MessageBox.h"
#include "libkwave/PluginManager.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleFIFO.h"
#include "libkwave/SampleFormat.h"
#include "libkwave/SignalManager.h"
#include "libkwave/Writer.h"

#include "RecordDevice.h"
#include "RecordDialog.h"
#include "RecordPlugin.h"
#include "RecordThread.h"
#include "SampleDecoder.h"
#include "SampleDecoderLinear.h"
#include "Record-ALSA.h"
#include "Record-OSS.h"

KWAVE_PLUGIN(RecordPlugin,"record","2.2","Thomas Eschenbacher");

//***************************************************************************
RecordPlugin::RecordPlugin(const PluginContext &context)
    :Kwave::Plugin(context), m_method(), m_device_name(), m_controller(),
     m_state(REC_EMPTY), m_device(0),
     m_dialog(0), m_thread(0), m_decoder(0), m_prerecording_queue(),
     m_writers(0), m_buffers_recorded(0), m_inhibit_count(0),
     m_trigger_value()
{
    i18n("record");
}

//***************************************************************************
RecordPlugin::~RecordPlugin()
{
    Q_ASSERT(!m_dialog);
    if (m_dialog) delete m_dialog;
    m_dialog = 0;

    Q_ASSERT(!m_thread);
    if (m_thread) delete m_thread;
    m_thread = 0;

    Q_ASSERT(!m_decoder);
    if (m_decoder) delete m_decoder;
    m_decoder = 0;

    if (m_device) delete m_device;
    m_device = 0;
}

//***************************************************************************
QStringList *RecordPlugin::setup(QStringList &previous_params)
{
    qDebug("RecordPlugin::setup");

    // create the setup dialog
    m_dialog = new RecordDialog(parentWidget(), previous_params,
                                &m_controller);
    Q_ASSERT(m_dialog);
    if (!m_dialog) return 0;

    // create the lowlevel recording thread
    m_thread = new RecordThread();
    Q_ASSERT(m_thread);
    if (!m_thread) {
	delete m_dialog;
	m_dialog = 0;
	return 0;
    }

    // connect some signals of the setup dialog
    connect(m_dialog, SIGNAL(sigMethodChanged(record_method_t)),
            this,     SLOT(setMethod(record_method_t)));
    connect(m_dialog, SIGNAL(sigDeviceChanged(const QString &)),
            this,     SLOT(setDevice(const QString &)));

    connect(m_dialog, SIGNAL(sigTracksChanged(unsigned int)),
            this,     SLOT(changeTracks(unsigned int)));
    connect(m_dialog, SIGNAL(sampleRateChanged(double)),
            this,     SLOT(changeSampleRate(double)));
    connect(m_dialog, SIGNAL(sigCompressionChanged(int)),
            this,     SLOT(changeCompression(int)));
    connect(m_dialog, SIGNAL(sigBitsPerSampleChanged(unsigned int)),
            this,     SLOT(changeBitsPerSample(unsigned int)));
    connect(m_dialog,
	    SIGNAL(sigSampleFormatChanged(SampleFormat)),
            this,
	    SLOT(changeSampleFormat(SampleFormat)));
    connect(m_dialog, SIGNAL(sigBuffersChanged()),
            this,     SLOT(buffersChanged()));
    connect(this,     SIGNAL(sigRecordedSamples(unsigned int)),
            m_dialog, SLOT(setRecordedSamples(unsigned int)));
    connect(m_dialog,      SIGNAL(sigTriggerChanged(bool)),
            &m_controller, SLOT(enableTrigger(bool)));
    m_controller.enableTrigger(m_dialog->params().record_trigger_enabled);
    connect(m_dialog, SIGNAL(sigPreRecordingChanged(bool)),
            &m_controller, SLOT(enablePrerecording(bool)));
    connect(m_dialog, SIGNAL(sigPreRecordingChanged(bool)),
            this, SLOT(prerecordingChanged(bool)));
    m_controller.enablePrerecording(m_dialog->params().pre_record_enabled);

    // connect the record controller and this
    connect(&m_controller, SIGNAL(sigReset(bool &)),
            this,          SLOT(resetRecording(bool &)));
    connect(&m_controller, SIGNAL(sigStartRecord()),
            this,          SLOT(startRecording()));
    connect(&m_controller, SIGNAL(sigStopRecord(int)),
            &m_controller, SLOT(deviceRecordStopped(int)));
    connect(&m_controller, SIGNAL(stateChanged(RecordState)),
            this,          SLOT(stateChanged(RecordState)));

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

    // activate the playback method
    setMethod(m_dialog->params().method);

//     // select the record device
//     setDevice(m_dialog->params().device_name);

    QStringList *list = new QStringList();
    Q_ASSERT(list);
    if (list && m_dialog->exec()) {
	// user has pressed "OK"
	*list = m_dialog->params().toList();
    } else {
	// user pressed "Cancel"
	if (list) delete list;
	list = 0;
    }

    /* de-queue all buffers that are pending and remove the record thread */
    if (m_thread) {
	m_thread->stop();
	while (m_thread->queuedBuffers())
	    processBuffer();
	delete m_thread;
	m_thread = 0;
    }

    if (m_decoder) delete m_decoder;
    m_decoder = 0;

    if (m_dialog) {
	fileInfo().setLength(signalLength());
	fileInfo().setTracks(m_dialog->params().tracks);
	delete m_dialog;
	m_dialog = 0;
    }

    // flush away all prerecording buffers
    m_prerecording_queue.clear();

    return list;
}

//***************************************************************************
void RecordPlugin::notice(QString message)
{
    Q_ASSERT(m_dialog);
    if (m_dialog) m_dialog->message(message);
}

//***************************************************************************
void RecordPlugin::closeDevice()
{
    if (m_device) {
	m_device->close();
	delete m_device;
	m_device = 0;
    }
}

//***************************************************************************
void RecordPlugin::setMethod(record_method_t method)
{
    Q_ASSERT(m_dialog);
    if (!m_dialog) return;

    InhibitRecordGuard _lock(*this); // don't record while settings change
    qDebug("RecordPlugin::setMethod(%d)", static_cast<int>(method));

    // change the recording method (class RecordDevice)
    if ((method != m_method) || !m_device) {
	if (m_device) delete m_device;
	m_device = 0;
	bool searching = false;

	// use the previous device
	QString device = "";
	QString section = "plugin "+name();
	KConfigGroup cfg = KGlobal::config()->group(section);

	// restore the previous device
	device = cfg.readEntry(
	    QString("last_device_%1").arg(static_cast<int>(method)));
// 	    qDebug("<<< %d -> '%s'", static_cast<int>(method), device.data());
	m_device_name = device;

	do {
	    switch (method) {
#ifdef HAVE_OSS_SUPPORT
		case RECORD_OSS:
		    m_device = new RecordOSS();
		    Q_ASSERT(m_device);
		    break;
#endif /* HAVE_OSS_SUPPORT */

#ifdef HAVE_ALSA_SUPPORT
		case RECORD_ALSA:
		    m_device = new RecordALSA();
		    Q_ASSERT(m_device);
		    break;
#endif /* HAVE_ALSA_SUPPORT */
		default:
		    qDebug("unsupported recording method (%d)",
			static_cast<int>(method));
		    if (!searching) {
			// start trying out all other methods
			searching = true;
			method = RECORD_NONE;
			++method;
			continue;
		    } else {
			// try next method
			++method;
		    }
		    qDebug("unsupported recording method - trying next (%d)",
		           static_cast<int>(method));
		    if (method != RECORD_INVALID) continue;
	    }
	    break;
	} while (true);
    }
    Q_ASSERT(m_device);

    // if we found no recording method
    if (method == RECORD_INVALID) {
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
void RecordPlugin::setDevice(const QString &device)
{
    Q_ASSERT(m_dialog);
    Q_ASSERT(m_device);
    if (!m_dialog || !m_device) return;

    InhibitRecordGuard _lock(*this); // don't record while settings change
    qDebug("RecordPlugin::setDevice('%s')", device.toLocal8Bit().data());

    // select the default device if this one is not supported
    QString dev = device;
    QStringList supported = m_device->supportedDevices();
    if (!supported.isEmpty() && !supported.contains(device)) {
	// use the first entry as default
	dev = supported.first();
	qDebug("RecordPlugin::setDevice(%s) -> fallback to '%s'",
	    device.toLocal8Bit().data(),
	    dev.toLocal8Bit().data());
    }

    // open and initialize the device
    int result = m_device->open(dev);

    // set the device in the dialog
    m_device_name = dev;
    m_dialog->setDevice(dev);

    // remember the device selection, just for the GUI
    // for the next change in the method
    QString section = "plugin "+name();
    KConfigGroup cfg = KGlobal::config()->group(section);
    cfg.writeEntry(QString("last_device_%1").arg(
	static_cast<int>(m_method)), m_device_name);
// 	qDebug(">>> %d -> '%s'", static_cast<int>(m_method),
// 	    m_device_name.data());
    cfg.sync();

    if (result < 0) {
	qWarning("RecordPlugin::openDevice('%s'): "\
	         "opening the device failed. error=%d",
	         dev.toLocal8Bit().data(), result);

	m_controller.setInitialized(false);
	m_dialog->showDevicePage();

	if (m_device_name.length()) {
	    // build a short device name for showing to the user
	    QString short_device_name = m_device_name;
	    if (m_device_name.contains("|")) {
		// tree syntax: extract card + device
		short_device_name = m_device_name.section("|", 0, 0) +
		    ", " + m_device_name.section("|", 3, 3);
	    }

	    // show an error message box
	    QString reason;
	    switch (result) {
		case -ENOENT:
		case -ENODEV:
		case -ENXIO:
		case -EIO:
		    reason = i18n(
			"Kwave was unable to open the device '%1'.\n"\
			"Maybe your system lacks support for the corresponding "\
			" hardware or the hardware is not connected.",
			short_device_name);
		    break;
		case -EBUSY:
		    reason = i18n(
			"The device '%1' seems to be occupied by another "\
			"application.\n"\
			"Please try again later.",
			short_device_name);
		    break;
	    }

	    if (reason.length()) Kwave::MessageBox::sorry(parentWidget(),
		reason, i18n("unable to open the recording device"));
	}

	m_device_name = QString::null;
	changeTracks(0);
    } else {
	m_controller.setInitialized(true);
	changeTracks(m_dialog->params().tracks);
    }

}

//***************************************************************************
void RecordPlugin::changeTracks(unsigned int new_tracks)
{
    Q_ASSERT(m_dialog);
    if (!m_dialog) return;

    InhibitRecordGuard _lock(*this); // don't record while settings change
//     qDebug("RecordPlugin::changeTracks(%u)", new_tracks);

    if (!m_device || m_device_name.isNull()) {
	// no device -> dummy/shortcut
	m_dialog->setSupportedTracks(0,0);
	m_dialog->setTracks(0);
	changeSampleRate(0);
	return;
    }

    // check the supported tracks
    unsigned int min = 0;
    unsigned int max = 0;
    m_device->detectTracks(min, max);
    unsigned int tracks = new_tracks;
    if ((tracks < min) || (tracks > max)) {
	// clip to the supported number of tracks
	if (tracks < min) tracks = min;
	if (tracks > max) tracks = max;

	if ((new_tracks && tracks) && (new_tracks != tracks)) {
	    QString s1;
	    switch (new_tracks) {
		case 1: s1 = i18n("Mono");   break;
		case 2: s1 = i18n("Stereo"); break;
		case 4: s1 = i18n("Quadro"); break;
		default:
		    s1 = i18n("%1 tracks", new_tracks);
	    }
	    QString s2;
	    switch (tracks) {
		case 1: s2 = i18n("Mono");   break;
		case 2: s2 = i18n("Stereo"); break;
		case 4: s2 = i18n("Quadro"); break;
		default:
		    s2 = i18n("%1 tracks", tracks);
	    }

	    notice(i18n("%1 is not supported, using %2", s1, s2));
	}
    }
    m_dialog->setSupportedTracks(min, max);

    // try to activate the new number of tracks
    int err = m_device->setTracks(tracks);
    if (err < 0) {
	// revert to the current device setting if failed
	tracks = m_device->tracks();
	if (new_tracks && (tracks > 0)) notice(
	    i18n("Recording with %1 track(s) failed, "\
		 "using %2 track(s)", new_tracks, tracks));
    }
    m_dialog->setTracks(tracks);

    // activate the new sample rate
    changeSampleRate(m_dialog->params().sample_rate);
}

//***************************************************************************
void RecordPlugin::changeSampleRate(double new_rate)
{
    Q_ASSERT(m_dialog);
    if (!m_dialog) return;

    InhibitRecordGuard _lock(*this); // don't record while settings change
//     qDebug("RecordPlugin::changeSampleRate(%u)", (unsigned int)new_rate);

    if (!m_device || m_device_name.isNull()) {
	// no device -> dummy/shortcut
	m_dialog->setSampleRate(0);
	changeCompression(-1);
	return;
    }

    // check the supported sample rates
    QList<double> supported_rates = m_device->detectSampleRates();
    double rate = new_rate;
    if (!supported_rates.contains(rate) && !supported_rates.isEmpty()) {
	// find the nearest sample rate
	double nearest = supported_rates.last();
	foreach (double r, supported_rates) {
	    if (fabs(r - rate) <= fabs(nearest - rate))
	        nearest = r;
	}
	rate = nearest;

	const QString sr1(m_dialog->rate2string(new_rate));
	const QString sr2(m_dialog->rate2string(rate));
	if ((static_cast<int>(new_rate) > 0) &&
	    (static_cast<int>(rate) > 0) &&
	    (static_cast<int>(new_rate) != static_cast<int>(rate)))
	    notice(i18n("%1Hz is not supported, "\
		        "using %2Hz", sr1, sr2));
    }
    m_dialog->setSupportedSampleRates(supported_rates);

    // try to activate the new sample rate
    int err = m_device->setSampleRate(rate);
    if (err < 0) {
	// revert to the current device setting if failed
	rate = m_device->sampleRate();

	const QString sr1(m_dialog->rate2string(new_rate));
	const QString sr2(m_dialog->rate2string(rate));
	if ((static_cast<int>(new_rate) > 0) &&
	    (static_cast<int>(rate) > 0) &&
	    (static_cast<int>(new_rate) != static_cast<int>(rate)))
	    notice(i18n("%1Hz failed, using %2Hz", sr1, sr2));
    }
    m_dialog->setSampleRate(rate);

    // set the compression again
    changeCompression(m_dialog->params().compression);
}

//***************************************************************************
void RecordPlugin::changeCompression(int new_compression)
{
    Q_ASSERT(m_dialog);
    if (!m_dialog) return;

    InhibitRecordGuard _lock(*this); // don't record while settings change
//     qDebug("RecordPlugin::changeCompression(%d)", (int)new_compression);

    if (!m_device || m_device_name.isNull()) {
	// no device -> dummy/shortcut
	m_dialog->setCompression(-1);
	changeBitsPerSample(0);
	return;
    }

    // check the supported compressions
    CompressionType types;
    QList<int> supported_comps = m_device->detectCompressions();
    int compression = new_compression;
    if (!supported_comps.contains(compression) && (compression != 0)) {
	// try to disable the compression (type 0)
	compression = 0;
	if (!supported_comps.isEmpty() &&
	    !supported_comps.contains(compression))
	{
	    // what now, "None" is not supported
	    // -> take the first supported one
	    compression = supported_comps[0];
	}

	if (compression != new_compression) {
	    const QString c1(types.name(types.findFromData(new_compression)));
	    const QString c2(types.name(types.findFromData(compression)));
	    notice(i18n("compression '%1' not supported, using '%2'",
                        c1, c2));
	}
    }
    m_dialog->setSupportedCompressions(supported_comps);

    // try to activate the new compression
    int err = m_device->setCompression(compression);
    if (err < 0) {
	// revert to the current device setting if failed
	compression = m_device->compression();

	if (compression != m_device->compression()) {
	    CompressionType types;
	    const QString c1(types.name(types.findFromData(compression)));
	    const QString c2(types.name(types.findFromData(
	                 m_device->compression())));
	    notice(i18n("compression %1 failed, using %2.", c1 ,c2));
	}
    }
    m_dialog->setCompression(compression);

    // set the resolution in bits per sample again
    changeBitsPerSample(m_dialog->params().bits_per_sample);
}

//***************************************************************************
void RecordPlugin::changeBitsPerSample(unsigned int new_bits)
{
    Q_ASSERT(m_dialog);
    if (!m_dialog) return;

    InhibitRecordGuard _lock(*this); // don't record while settings change
//     qDebug("RecordPlugin::changeBitsPerSample(%d)", (int)new_bits);

    if (!m_device || m_device_name.isNull()) {
	// no device -> dummy/shortcut
	m_dialog->setBitsPerSample(0);
	changeSampleFormat(SampleFormat::Unknown);
	return;
    }

    // check the supported resolution in bits per sample
    QList<unsigned int> supported_bits = m_device->supportedBits();
    int bits = new_bits;
    if (!supported_bits.contains(bits) && !supported_bits.isEmpty()) {
	// find the nearest resolution
	int nearest = supported_bits.last();
	foreach (unsigned int b, supported_bits) {
	    if (qAbs(static_cast<int>(b) - nearest) <= qAbs(bits - nearest))
	        nearest = static_cast<int>(b);
	}
	bits = nearest;

	if ((static_cast<int>(new_bits) > 0) && (bits > 0)) notice(
	    i18n("%1 bits per sample is not supported, "\
	         "using %2 bits per sample",
		 static_cast<int>(new_bits), bits));
    }
    m_dialog->setSupportedBits(supported_bits);

    // try to activate the resolution
    int err = m_device->setBitsPerSample(bits);
    if (err < 0) {
	// revert to the current device setting if failed
	bits = m_device->bitsPerSample();
	if ((new_bits> 0) && (bits > 0)) notice(
	    i18n("%1 bits per sample failed, "\
		 "using %2 bits per sample",
		 static_cast<int>(new_bits), bits));
    }
    m_dialog->setBitsPerSample(bits);

    // set the sample format again
    changeSampleFormat(m_dialog->params().sample_format);
}

//***************************************************************************
void RecordPlugin::changeSampleFormat(SampleFormat new_format)
{
    Q_ASSERT(m_dialog);
    if (!m_dialog) return;

    InhibitRecordGuard _lock(*this); // don't record while settings change
//     qDebug("RecordPlugin::changeSampleFormat(%d)", (int)new_format);

    if (!m_device || m_device_name.isNull()) {
	// no device -> dummy/shortcut
	m_dialog->setSampleFormat(SampleFormat::Unknown);
	return;
    }

    // check the supported sample formats
    QList<SampleFormat> supported_formats = m_device->detectSampleFormats();
    SampleFormat format = new_format;
    if (!supported_formats.contains(format) && !supported_formats.isEmpty()) {
	// use the device default instead
	format = m_device->sampleFormat();

	// if this was also not supported -> stupid device !?
	if (!supported_formats.contains(format)) {
	    format = supported_formats.first(); // just take the first one :-o
	}

	SampleFormat::Map sf;
	const QString s1 = sf.name(sf.findFromData(new_format));
	const QString s2 = sf.name(sf.findFromData(format));
	if (!(new_format == -1) && !(new_format == format)) {
	    notice(i18n("sample format '%1' is not supported, "\
		        "using '%2'", s1, s2));
	}
    }
    m_dialog->setSupportedSampleFormats(supported_formats);

    // try to activate the new format
    int err = m_device->setSampleFormat(format);
    if (err < 0) {
	// use the device default instead
	format = m_device->sampleFormat();

	SampleFormat::Map sf;
	const QString s1 = sf.name(sf.findFromData(new_format));
	const QString s2 = sf.name(sf.findFromData(format));
	if (format > 0) notice(
	    i18n("sample format '%1' failed, using '%2'", s1, s2));
    }
    m_dialog->setSampleFormat(format);
}

//***************************************************************************
void RecordPlugin::buffersChanged()
{
    InhibitRecordGuard _lock(*this); // don't record while settings change
    // this implicitely activates the new settings
}

//***************************************************************************
void RecordPlugin::enterInhibit()
{
    m_inhibit_count++;
    if ((m_inhibit_count == 1) && m_thread) {
	// set hourglass cursor
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

// 	qDebug("RecordPlugin::enterInhibit() - STOPPING");
	m_thread->stop();
	Q_ASSERT(!m_thread->isRunning());

	// de-queue all buffers that are still in the queue
	while (m_thread->queuedBuffers())
	    processBuffer();
    }
}

//***************************************************************************
void RecordPlugin::leaveInhibit()
{
    Q_ASSERT(m_inhibit_count);
    Q_ASSERT(m_dialog);

    if (m_inhibit_count) m_inhibit_count--;

    while (!m_inhibit_count && paramsValid()) {
// 	qDebug("RecordPlugin::leaveInhibit() - STARTING ("
// 	       "%d channels, %d bits)",
// 	       m_dialog->params().tracks,
// 	       m_dialog->params().bits_per_sample);

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
bool RecordPlugin::paramsValid()
{
    if (!m_thread || !m_device || !m_dialog) return false;
    if (m_device_name.isNull()) return false;

    const RecordParams &params = m_dialog->params();
    if (params.tracks < 1) return false;

    return true;
}

//***************************************************************************
void RecordPlugin::resetRecording(bool &accepted)
{
    InhibitRecordGuard _lock(*this);

    emitCommand("nomacro:close()");
    accepted = manager().signalManager().isEmpty();
    if (!accepted) return;

    if (m_writers) m_writers->clear();
    m_buffers_recorded = 0;

    m_controller.setEmpty(true);
    emit sigRecordedSamples(0);
}

//***************************************************************************
void RecordPlugin::setupRecordThread()
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
    if (m_decoder) delete m_decoder;
    m_decoder = 0;

    // our own reference to the record parameters
    const RecordParams &params = m_dialog->params();
    if (!paramsValid()) return;

    // create a decoder for the current sample format
    switch (params.compression) {
	case AF_COMPRESSION_NONE:
	    switch (params.sample_format) {
		case SampleFormat::Unsigned:
		case SampleFormat::Signed:
		    // decoder for all linear formats
		    m_decoder = new SampleDecoderLinear(
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
	case AF_COMPRESSION_G711_ALAW:
	case AF_COMPRESSION_G711_ULAW:
	case CompressionType::MPEG_LAYER_II:
	case AF_COMPRESSION_MS_ADPCM:
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
	const unsigned int prerecording_samples = static_cast<unsigned int>(
	    rint(params.pre_record_time * params.sample_rate));
	m_prerecording_queue.resize(params.tracks);
	for (int i=0; i < m_prerecording_queue.size(); i++)
	    m_prerecording_queue[i].setSize(prerecording_samples);

	if (m_prerecording_queue.size() != static_cast<int>(params.tracks)) {
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
void RecordPlugin::startRecording()
{
    Q_ASSERT(m_dialog);
    Q_ASSERT(m_thread);
    Q_ASSERT(m_device);
    if (!m_dialog || !m_thread || !m_device) return;

    InhibitRecordGuard _lock(*this); // don't record while settings change
    qDebug("RecordPlugin::startRecording()");

    if ((m_state != REC_PAUSED) || !m_decoder) {
	double rate = m_dialog->params().sample_rate;
	unsigned int tracks = m_dialog->params().tracks;
	unsigned int samples = 0;
	unsigned int bits = m_dialog->params().bits_per_sample;

	if (!tracks) return;

	/*
	 * if tracks or sample rate has changed
	 * -> start over with a new signal and new settings
	 */
	if ((!m_writers) ||
	    (m_writers->tracks() != tracks) ||
	    (fileInfo().rate() != rate))
	{
	    // create a new and empty signal

	    emitCommand(QString("newsignal(%1,%2,%3,%4)").arg(
		samples).arg(rate).arg(bits).arg(tracks));
	    SignalManager &mgr = signalManager();
	    if ((mgr.rate() != rate) || (mgr.bits() != bits) ||
	        (mgr.tracks() != tracks))
	    {
		emitCommand("close");
		return;
	    }

	    // we do not need UNDO here
	    signalManager().disableUndo();

	    // create a sink for our audio data
	    if (m_writers) delete m_writers;
	    m_writers = new Kwave::MultiTrackWriter(signalManager(), Append);
	    Q_ASSERT(m_writers);
	    Q_ASSERT((m_writers) && (m_writers->tracks() == tracks));
	    if ((!m_writers) || (m_writers->tracks() != tracks)) {
		Kwave::MessageBox::sorry(m_dialog, i18n("Out of memory"));
		return;
	    }
	} else {
	    // re-use the current signal and append to it
	}

	// initialize the file information
	fileInfo().setRate(rate);
	fileInfo().setBits(bits);
	fileInfo().setTracks(tracks);
	fileInfo().set(INF_MIMETYPE, "audio/vnd.wave");
	fileInfo().set(INF_SAMPLE_FORMAT,
	    m_dialog->params().sample_format.toInt());
	fileInfo().set(INF_COMPRESSION, m_dialog->params().compression);

	// add our Kwave Software tag
	const KAboutData *about_data =
	    KGlobal::mainComponent().aboutData();
	QString software = about_data->programName() + "-" +
			    about_data->version() +
			    i18n(" for KDE ") +
			    i18n(KDE_VERSION_STRING);
	qDebug("adding software tag: '%s'",
		software.toLocal8Bit().data());
	fileInfo().set(INF_SOFTWARE, software);

	// add a date tag, ISO format
	QDate now(QDate::currentDate());
	QString date;
	date = date.sprintf("%04d-%02d-%02d",
		now.year(), now.month(), now.day());
	QVariant value = date.toUtf8();
	fileInfo().set(INF_CREATION_DATE, value);
    }

    // now the recording can be considered to be started
    m_controller.deviceRecordStarted();
}

//***************************************************************************
void RecordPlugin::recordStopped(int reason)
{
    qDebug("RecordPlugin::recordStopped(%d)", reason);
    if (reason >= 0) return; // nothing to do

    // recording was aborted
    QString description;
    switch (reason) {
	case -ENOBUFS:
	    description = i18n("Buffer overrun. Please increase the "\
	                       "number and/or size of the record buffers.");
	    break;
	case -EBUSY:
	    description = i18n("The recording device seems to be busy!");
	    break;
	default:
	    description = i18n("Reading from the recording device failed, "\
	                       "error number = %1 (%2)", -reason,
			       QString::fromLocal8Bit(strerror(-reason)));
    }
    Kwave::MessageBox::error(m_dialog, description);

    if (m_writers) m_writers->flush();
    qDebug("RecordPlugin::recordStopped(): last=%u",
           (m_writers) ? m_writers->last() : 0);

    // flush away all prerecording buffers
    m_prerecording_queue.clear();
}

//***************************************************************************
void RecordPlugin::stateChanged(RecordState state)
{
    qDebug("RecordPlugin::stateChanged(%s)", m_controller.stateName(state));

    m_state = state;
    switch (m_state) {
	case REC_UNINITIALIZED:
	case REC_EMPTY:
	case REC_PAUSED:
	case REC_DONE:
	    // reset buffer status
	    if (m_writers) {
		m_writers->flush();
		delete m_writers;
		m_writers = 0;
	    }
	    m_buffers_recorded = 0;
	    m_dialog->updateBufferState(0,0);
	    break;
	default:
	    ;
    }
}

//***************************************************************************
void RecordPlugin::updateBufferProgressBar()
{
    Q_ASSERT(m_dialog);
    Q_ASSERT(m_thread);
    if (!m_dialog || !m_thread) return;

    unsigned int buffers_total = m_dialog->params().buffer_count;

    // if we are still recording: update the progress bar
    if ((m_state != REC_EMPTY) && (m_state != REC_PAUSED) &&
        (m_state != REC_DONE))
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
void RecordPlugin::split(QByteArray &raw_data, QByteArray &dest,
                         unsigned int bytes_per_sample,
                         unsigned int track,
                         unsigned int tracks)
{
    unsigned int samples = raw_data.size() / bytes_per_sample / tracks;
    const unsigned int increment = (tracks-1) * bytes_per_sample;
    char *src = raw_data.data();
    char *dst = dest.data();
    src += (track * bytes_per_sample);

#if 0
    static int saw[2] = {0, 0};

    // simple sawtooth generator, nice for debugging
    while (samples--) {
	for (unsigned int byte=0; byte < bytes_per_sample; byte++) {
	    *dst = (saw[track] / 128);
	    dst++;
	}
	saw[track]++;
	if (saw[track] > (SAMPLE_MAX/1024)) saw[track] = (SAMPLE_MIN/1024);
    }
#else
    while (samples--) {
	for (unsigned int byte=0; byte < bytes_per_sample; byte++) {
	    *dst = *src;
	    dst++;
	    src++;
	}
	src += increment;
    }
#endif
}

//***************************************************************************
bool RecordPlugin::checkTrigger(unsigned int track,
                                Kwave::SampleArray &buffer)
{
    Q_ASSERT(m_dialog);
    if (!m_dialog) return false;
    if (!buffer.size()) return false;
    if (!m_writers) return false;
    if (m_trigger_value.size() != static_cast<int>(m_writers->tracks()))
	return false;

    // shortcut if no trigger has been set
    if (!m_dialog->params().record_trigger_enabled) return true;

    // pass the buffer through a rectifier and a lowpass with
    // center frequency about 2Hz to get the amplitude
    float trigger = static_cast<float>(
	m_dialog->params().record_trigger / 100.0);
    const float rate = static_cast<const float>(
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
    const float f_rise = 20.0;
    float Fg = f_rise / rate;
    float n = 1.0 / tan(M_PI * Fg);
    const float a0_r = 1.0 / (1.0 + n);
    const float b1_r = (1.0 - n) / (1.0 + n);

    // fall coefficient: ~1.0Hz
    const float f_fall = 1.0;
    Fg = f_fall / rate;
    n = 1.0 / tan(M_PI * Fg);
    const float a0_f = 1.0 / (1.0 + n);
    const float b1_f = (1.0 - n) / (1.0 + n);

    float y = m_trigger_value[track];
    float last_x = y;
    for (unsigned int t=0; t < buffer.size(); ++t) {
	float x = fabs(sample2float(buffer[t])); /* rectifier */

	if (x > y) { /* diode */
	    // rise if amplitude is above average (serial R)
	    y = (a0_r * x) + (a0_r * last_x) - (b1_r * y);
	}

	// fall (parallel R)
	y = (a0_f * x) + (a0_f * last_x) - (b1_f * y);

	// remember x[t-1]
	last_x = x;

// nice for debugging:
//	buffer[t] = (int)((double)(1 << (SAMPLE_BITS-1)) * y);
	if (y > trigger) return true;
    }
    m_trigger_value[track] = y;

    qDebug(">> level=%5.3g, trigger=%5.3g", y, trigger);

    return false;
}

//***************************************************************************
void RecordPlugin::enqueuePrerecording(unsigned int track,
                                       const Kwave::SampleArray &decoded)
{
    Q_ASSERT(m_dialog);
    Q_ASSERT(static_cast<int>(track) < m_prerecording_queue.size());
    if (!m_dialog) return;
    if (static_cast<int>(track) >= m_prerecording_queue.size()) return;

    // append the array with decoded sample to the prerecording buffer
    m_prerecording_queue[track].put(decoded);
}

//***************************************************************************
void RecordPlugin::flushPrerecordingQueue()
{
    if (!m_prerecording_queue.size()) return;
    Q_ASSERT(m_dialog);
    Q_ASSERT(m_thread);
    Q_ASSERT(m_decoder);
    if (!m_dialog || !m_thread || !m_decoder) return;

    const RecordParams &params = m_dialog->params();
    const unsigned int tracks = params.tracks;
    Q_ASSERT(tracks);
    if (!tracks) return;
    Q_ASSERT(m_writers);
    if (!m_writers) return;
    Q_ASSERT(tracks == m_writers->tracks());
    if (!tracks || (tracks != m_writers->tracks())) return;

    for (unsigned int track=0; track < tracks; ++track) {
	SampleFIFO &fifo = m_prerecording_queue[track];
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
void RecordPlugin::processBuffer()
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

    const RecordParams &params = m_dialog->params();
    const unsigned int tracks = params.tracks;
    Q_ASSERT(tracks);
    if (!tracks) return;

    const unsigned int bytes_per_sample = m_decoder->rawBytesPerSample();
    Q_ASSERT(bytes_per_sample);
    if (!bytes_per_sample) return;

    unsigned int samples = (buffer.size() / bytes_per_sample) / tracks;
    Q_ASSERT(samples);
    if (!samples) return;

    // check for reached recording time limit if enabled
    if (params.record_time_limited && m_writers) {
	const unsigned int last = m_writers->last();
	const unsigned int already_recorded = (last) ? (last + 1) : 0;
	const unsigned int limit = static_cast<const unsigned int>(rint(
	    params.record_time * params.sample_rate));
	if (already_recorded + samples >= limit) {
	    // reached end of recording time, we are full
	    if (m_state == REC_RECORDING) {
		samples = (limit > already_recorded) ?
		          (limit - already_recorded) : 0;
		buffer.resize(samples * tracks * bytes_per_sample);
	    }
	    recording_done = true;
	}
    }

    QByteArray buf;
    buf.resize(bytes_per_sample * samples);
    Q_ASSERT(buf.size() == static_cast<int>(bytes_per_sample * samples));
    if (buf.size() != static_cast<int>(bytes_per_sample * samples)) return;

    Kwave::SampleArray decoded(samples);
    Q_ASSERT(decoded.size() == samples);
    if (decoded.size() != samples) return;

    // check for trigger
    // note: this might change the state, which affects the
    //       processing of all tracks !
    if ((m_state == REC_WAITING_FOR_TRIGGER) ||
        ((m_state == REC_PRERECORDING) && (params.record_trigger_enabled)))
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

    if ((m_state == REC_RECORDING) && (!m_prerecording_queue.isEmpty())) {
	// flush all prerecorded buffers to the output
	flushPrerecordingQueue();
    }

    // use a copy of the state, in case it changes below ;-)
    RecordState state = m_state;
    for (unsigned int track=0; track < tracks; ++track) {
	// decode and care for all special effects, meters and so on
	// split off and decode buffer with current track
	split(buffer, buf, bytes_per_sample, track, tracks);
	m_decoder->decode(buf, decoded);

	// update the level meter and other effects
	m_dialog->updateEffects(track, decoded);

	// if the first buffer is full -> leave REC_BUFFERING
	// limit state transitions to a point before the first track is
	// processed (avoid asymmetry)
	if ((track == 0) && (m_state == REC_BUFFERING) &&
	    (m_buffers_recorded > 1))
	{
	    m_controller.deviceBufferFull();
	    state = m_state; // might have changed!
	}

	switch (state) {
	    case REC_UNINITIALIZED:
	    case REC_EMPTY:
	    case REC_PAUSED:
	    case REC_DONE:
	    case REC_BUFFERING:
	    case REC_WAITING_FOR_TRIGGER:
		// already handled before or nothing to do...
		break;
	    case REC_PRERECORDING:
		// enqueue the buffers into a FIFO
		enqueuePrerecording(track, decoded);
		break;
	    case REC_RECORDING: {
		// put the decoded track data into the buffer
		Q_ASSERT(m_writers);
		if (!m_writers) break;
		Q_ASSERT(tracks == m_writers->tracks());
		if (!tracks || (tracks != m_writers->tracks())) break;

		Kwave::Writer *writer = (*m_writers)[track];
		Q_ASSERT(writer);
		if (writer) (*writer) << decoded;
		m_controller.setEmpty(false);

		break;
	    }
	}
    }

    // update the number of recorded samples
    if (m_writers) emit sigRecordedSamples(m_writers->last()+1);

    // if this was the last received buffer, change state
    if (recording_done && (m_state != REC_DONE) && (m_state != REC_EMPTY)) {
	m_controller.actionStop();
    }

}

//***************************************************************************
void RecordPlugin::prerecordingChanged(bool enable)
{
    (void)enable;
    InhibitRecordGuard _lock(*this); // activate the change
}

//***************************************************************************
#include "RecordPlugin.moc"
//***************************************************************************
//***************************************************************************
