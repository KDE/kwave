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

#include <qcursor.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <qvariant.h>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kmessagebox.h>

#include "libkwave/CompressionType.h"
#include "libkwave/FileInfo.h"
#include "libkwave/InsertMode.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleFIFO.h"
#include "libkwave/SampleFormat.h"
#include "libkwave/SampleWriter.h"

#include "libgui/Notice.h"

#include "kwave/PluginManager.h"
#include "kwave/SignalManager.h"
#include "kwave/TopWidget.h"

#include "RecordDevice.h"
#include "RecordDialog.h"
#include "RecordPlugin.h"
#include "RecordThread.h"
#include "SampleDecoder.h"
#include "SampleDecoderLinear.h"
#include "Record-ALSA.h"
#include "Record-OSS.h"

KWAVE_PLUGIN(RecordPlugin,"record","Thomas Eschenbacher");

//***************************************************************************
RecordPlugin::RecordPlugin(const PluginContext &context)
    :KwavePlugin(context), m_method(), m_device_name(), m_controller(),
     m_state(REC_EMPTY), m_device(0),
     m_dialog(0), m_thread(0), m_decoder(0), m_prerecording_queue(),
     m_writers(), m_buffers_recorded(0), m_inhibit_count(0),
     m_trigger_value()
{
    m_prerecording_queue.setAutoDelete(true);
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
    connect(m_thread, SIGNAL(bufferFull(QByteArray)),
            this,     SLOT(processBuffer(QByteArray)));

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

    if (m_dialog) {
	fileInfo().setLength(signalLength());
	fileInfo().setTracks(m_dialog->params().tracks);
	delete m_dialog;
	m_dialog = 0;
    }

    if (m_thread) delete m_thread;
    m_thread = 0;

    if (m_decoder) delete m_decoder;
    m_decoder = 0;

    // flush away all prerecording buffers
    m_prerecording_queue.clear();

    return list;
}

//***************************************************************************
void RecordPlugin::notice(QString message)
{
    Q_ASSERT(m_dialog);
    if (!m_dialog) return;

    (void) new Notice(m_dialog, message);
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
    KConfig *cfg = KGlobal::config();
    Q_ASSERT(cfg);

    InhibitRecordGuard _lock(*this); // don't record while settings change
    qDebug("RecordPlugin::setMethod(%d)", (int)method);

    // change the recording method (class RecordDevice)
    if ((method != m_method) || !m_device) {
	if (m_device) delete m_device;
	m_device = 0;
	bool searching = false;

	// use the previous device
	if (cfg) {
	    QString device = "";
	    cfg->setGroup("plugin "+name());

	    // restore the previous device
	    device = cfg->readEntry(
	        QString("last_device_%1").arg(static_cast<int>(method)));
// 	    qDebug("<<< %d -> '%s'", static_cast<int>(method), device.data());
	    m_device_name = device;
	}

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
		    qDebug("unsupported recording method (%d)", (int)method);
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
		           (int)method);
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
    qDebug("RecordPlugin::setDevice(%s)", device.local8Bit().data());

    // open and initialize the device
    int result = m_device->open(device);

    // set the device in the dialog
    m_device_name = device;
    m_dialog->setDevice(device);

    // remember the device selection, just for the GUI
    // for the next change in the method
    KConfig *cfg = KGlobal::config();
    if (cfg) {
	cfg->setGroup("plugin "+name());
	cfg->writeEntry(QString("last_device_%1").arg(
	    static_cast<int>(m_method)), m_device_name);
// 	qDebug(">>> %d -> '%s'", static_cast<int>(m_method),
// 	    m_device_name.data());
	cfg->sync();
    }

    if (result < 0) {
	qWarning("RecordPlugin::openDevice(): "\
	         "opening the device failed.");

// 	// show an error message box
// 	KMessageBox::error(parentWidget(), result,
// 	    i18n("unable to open '%1'").arg(
// 	    m_device_name));
	changeTracks(0);
	return;
    }

    changeTracks(m_dialog->params().tracks);
}

//***************************************************************************
void RecordPlugin::changeTracks(unsigned int new_tracks)
{
    Q_ASSERT(m_dialog);
    if (!m_dialog) return;

    InhibitRecordGuard _lock(*this); // don't record while settings change
    qDebug("RecordPlugin::changeTracks(%u)", new_tracks);

    if (!m_device || !new_tracks) {
	// no device -> dummy/shortcut
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
		    s1 = i18n("%1 tracks").arg(new_tracks);
	    }
	    QString s2;
	    switch (tracks) {
		case 1: s2 = i18n("Mono");   break;
		case 2: s2 = i18n("Stereo"); break;
		case 4: s2 = i18n("Quadro"); break;
		default:
		    s2 = i18n("%1 tracks").arg(tracks);
	    }

	    notice(i18n("This device does not support recording with %1, "\
		        "using %2 instead.").arg(s1).arg(s2));
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
		 "using %2 track(s) instead.").arg(new_tracks).arg(tracks));
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

    if (!m_device || (new_rate <= 0)) {
	// no device -> dummy/shortcut
	m_dialog->setSampleRate(0);
	changeCompression(0);
	return;
    }

    // check the supported sample rates
    QValueList<double> supported_rates = m_device->detectSampleRates();
    double rate = new_rate;
    if (!supported_rates.contains(rate)) {
	// find the nearest sample rate
	double nearest = supported_rates.last();
	QValueList<double>::Iterator it;
	for (it=supported_rates.begin(); it != supported_rates.end(); ++it) {
	    if (fabs(*it - nearest) <= fabs(rate - nearest))
	        nearest = *it;
	}
	rate = nearest;

	const QString sr1(m_dialog->rate2string(new_rate));
	const QString sr2(m_dialog->rate2string(rate));
	if (((int)new_rate > 0) && ((int)rate > 0) &&
	    ((int)new_rate != (int)rate))
	    notice(i18n("The sample rate %1Hz is not supported, "\
		        "using %2Hz instead.").arg(sr1).arg(sr2));
    }
    m_dialog->setSupportedSampleRates(supported_rates);

    // try to activate the new sample rate
    int err = m_device->setSampleRate(rate);
    if (err < 0) {
	// revert to the current device setting if failed
	rate = m_device->sampleRate();

	const QString sr1(m_dialog->rate2string(new_rate));
	const QString sr2(m_dialog->rate2string(rate));
	if (((int)new_rate > 0) && ((int)rate > 0) &&
	    ((int)new_rate != (int)rate))
	    notice(i18n("Setting the sample rate %1Hz failed, "\
		        "using %2Hz instead.").arg(sr1).arg(sr2));
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

    if (!m_device || (new_compression < 0)) {
	// no device -> dummy/shortcut
	m_dialog->setCompression(-1);
	changeBitsPerSample(0);
	return;
    }

    // check the supported compressions
    CompressionType types;
    QValueList<int> supported_comps = m_device->detectCompressions();
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
	    notice(i18n("The compression '%1' is not supported, "\
		        "using '%2' instead.").arg(c1).arg(c2));
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
	    notice(i18n("Setting the compression type %1 failed, "\
		        "using %2 instead.").arg(c1).arg(c2));
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
//     qDebug("RecordPlugin::changeBitsPerSample(%u)", new_bits);

    if (!m_device || !new_bits) {
	// no device -> dummy/shortcut
	m_dialog->setBitsPerSample(0);
	changeSampleFormat(SampleFormat::Unknown);
	return;
    }

    // check the supported resolution in bits per sample
    QValueList<unsigned int> supported_bits = m_device->supportedBits();
    int bits = new_bits;
    if (!supported_bits.contains(bits) && !supported_bits.isEmpty()) {
	// find the nearest resolution
	int nearest = supported_bits.last();
	QValueList<unsigned int>::Iterator it;
	for (it=supported_bits.begin(); it != supported_bits.end(); ++it) {
	    if (abs((int)*it - nearest) <= abs(bits - nearest))
	        nearest = (int)*it;
	}
	bits = nearest;

	if (((int)new_bits > 0) && (bits > 0)) notice(
	    i18n("The resolution %1 bits per sample is not supported, "\
	         "using %2 bits per sample instead.").arg(
		 (int)new_bits).arg(bits));
    }
    m_dialog->setSupportedBits(supported_bits);

    // try to activate the resolution
    int err = m_device->setBitsPerSample(bits);
    if (err < 0) {
	// revert to the current device setting if failed
	bits = m_device->bitsPerSample();
	if ((new_bits> 0) && (bits > 0)) notice(
	    i18n("Setting the resolution %1 bits per sample failed, "\
		 "using %2 bits per sample instead.").arg(
		 (int)new_bits).arg(bits));
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

    if (!m_device || (new_format < 0)) {
	// no device -> dummy/shortcut
	m_dialog->setSampleFormat(SampleFormat::Unknown);
	return;
    }

    // check the supported sample formats
    QValueList<SampleFormat> supported_formats =
	m_device->detectSampleFormats();
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
	    notice(i18n("The sample format '%1' is not supported, "\
		        "using '%2' instead.").arg(s1).arg(s2));
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
	    i18n("Setting the sample format '%1' failed, "\
	         "using '%2' instead.").arg(s1).arg(s2));
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

	qDebug("RecordPlugin::enterInhibit() - STOPPING");
	m_thread->stop();
	Q_ASSERT(!m_thread->running());
    }
}

//***************************************************************************
void RecordPlugin::leaveInhibit()
{
    Q_ASSERT(m_inhibit_count);
    Q_ASSERT(m_dialog);

    if (m_inhibit_count) m_inhibit_count--;
    if (!m_inhibit_count && m_thread && m_device && m_dialog) {
// 	qDebug("RecordPlugin::leaveInhibit() - STARTING ("+
// 	       "%d channels, %d bits)",
//                m_dialog->params().tracks,
// 	       m_dialog->params().bits_per_sample);

	Q_ASSERT(!m_thread->running());
	if (m_thread->running()) return;

	// set new parameters for the recorder
	setupRecordThread();

	// and let the thread run (again)
	m_thread->start();

	// take back the hourglass cursor
	QApplication::restoreOverrideCursor();
    }
}

//***************************************************************************
void RecordPlugin::resetRecording(bool &accepted)
{
    InhibitRecordGuard _lock(*this);
    qDebug("RecordPlugin::resetRecording()");

    TopWidget &topwidget = this->manager().topWidget();
    accepted = topwidget.closeFile();
    if (!accepted) return;

    m_writers.flush();
    m_writers.resize(0);
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
    if (!m_thread || !m_dialog || !m_device) return;

    // stop the thread if necessary (should never happen)
    Q_ASSERT(!m_thread->running());
    if (m_thread->running()) m_thread->stop();
    Q_ASSERT(!m_thread->running());

    // delete the previous decoder
    if (m_decoder) delete m_decoder;
    m_decoder = 0;

    // our own reference to the record parameters
    const RecordParams &params = m_dialog->params();

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
	KMessageBox::sorry(m_dialog, i18n("Out of memory"));
	return;
    }

    // set up the prerecording queues
    m_prerecording_queue.clear();
    if (params.pre_record_enabled) {
	// prepare a queue for each track
	m_prerecording_queue.resize(params.tracks);
	unsigned int fifo_depth = (unsigned int)ceil((
	    params.pre_record_time * params.sample_rate));

	for (unsigned int track=0; track < params.tracks; ++track) {
	    SampleFIFO *fifo = new SampleFIFO(fifo_depth);
	    m_prerecording_queue.insert(track, fifo);
	    Q_ASSERT(fifo);
	    if (!fifo) {
		m_prerecording_queue.clear();
		KMessageBox::sorry(m_dialog, i18n("Out of memory"));
		return;
	    }
	}
    }

    // set up the recording trigger values
    m_trigger_value.resize(params.tracks);
    m_trigger_value.fill((double)0.0);

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

//     InhibitRecordGuard _lock(*this); // don't record while settings change
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
	if ((m_writers.count() != tracks) ||
	    (fileInfo().rate() != rate))
	{
	    // create a new and empty signal
	    TopWidget &topwidget = this->manager().topWidget();
	    int res = topwidget.newSignal(samples, rate, bits, tracks);
	    if (res < 0) return;

	    // we do not need UNDO here
	    signalManager().disableUndo();

	    // create a sink for our audio data
	    manager().openMultiTrackWriter(m_writers, Append);
	    Q_ASSERT(m_writers.count() == tracks);
	    if (m_writers.count() != tracks) {
		KMessageBox::sorry(m_dialog, i18n("Out of memory"));
		return;
	    }
	} else {
	    // re-use the current signal and append to it
	}

	// initialize the file information
	fileInfo().setRate(rate);
	fileInfo().setBits(bits);
	fileInfo().setTracks(tracks);
//	fileInfo().setLength(m_writers.last());
	fileInfo().set(INF_MIMETYPE, "audio/vnd.wave");
	fileInfo().set(INF_SAMPLE_FORMAT,
	    m_dialog->params().sample_format.toInt());
	fileInfo().set(INF_COMPRESSION, m_dialog->params().compression);

	// add our Kwave Software tag
	const KAboutData *about_data = KGlobal::instance()->aboutData();
	QString software = about_data->programName() + "-" +
	    about_data->version() + i18n(" for KDE ") +
	    i18n(QString::fromLatin1(KDE_VERSION_STRING));
	fileInfo().set(INF_SOFTWARE, software);

	// add a date tag
	QDate now(QDate::currentDate());
	QString date;
	date = date.sprintf("%04d-%02d-%02d",
		now.year(), now.month(), now.day());
	QVariant value = date.utf8();
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
	                       "error number = %1 (%2)").arg(-reason).arg(
			       QString::fromLocal8Bit(strerror(-reason)));
    }
    KMessageBox::error(m_dialog, description);

    m_writers.flush();
    qDebug("RecordPlugin::recordStopped(): wrote %u samples",
           m_writers.last());

    // flush away all prerecording buffers
    m_prerecording_queue.clear();
}

//***************************************************************************
void RecordPlugin::stateChanged(RecordState state)
{
    qDebug("RecordPlugin::stateChanged(%d)", (int)state);

    m_state = state;
    switch (m_state) {
	case REC_EMPTY:
	case REC_PAUSED:
	case REC_DONE:
	    // reset buffer status
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
                                QMemArray<sample_t> &buffer)
{
    Q_ASSERT(m_dialog);
    if (!m_dialog) return false;
    if (!buffer.size()) return false;
    if (m_trigger_value.size() != m_writers.count()) return false;

    // shortcut if no trigger has been set
    if (!m_dialog->params().record_trigger_enabled) return true;

    // pass the buffer through a rectifier and a lowpass with
    // center frequency about 2Hz to get the amplitude
    float trigger = (float)m_dialog->params().record_trigger / 100.0;
    const float rate = (float)m_dialog->params().sample_rate;

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
                                       const QMemArray<sample_t> &decoded)
{
    Q_ASSERT(m_dialog);
    Q_ASSERT(track < m_prerecording_queue.size());
    if (!m_dialog) return;
    if (track >= m_prerecording_queue.size()) return;

    // append the array with decoded sample to the prerecording buffer
    SampleFIFO *fifo = m_prerecording_queue.at(track);
    Q_ASSERT(fifo);
    if (fifo) fifo->put(decoded);
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
    Q_ASSERT(tracks == m_writers.count());
    if (!tracks || (tracks != m_writers.count())) return;

    for (unsigned int track=0; track < tracks; ++track) {
	SampleFIFO *fifo = m_prerecording_queue.at(track);
	Q_ASSERT(fifo);
	if (!fifo) continue;
	Q_ASSERT(fifo->length());
	if (!fifo->length()) continue;

	// push all buffers to the writer, starting at the tail
	SampleWriter *writer = m_writers[track];
	Q_ASSERT(writer);
	if (writer) {
	    fifo->align();
	    *writer << fifo->data();
	}

	// discard the FIFO and it's content
	fifo->resize(0);
    }

    // the queues are no longer needed
    m_prerecording_queue.clear();

    // we have transferred data to the writers, we are no longer empty
    m_controller.setEmpty(false);
}

//***************************************************************************
void RecordPlugin::processBuffer(QByteArray buffer)
{
    bool recording_done = false;

//     qDebug("RecordPlugin::processBuffer()");
    Q_ASSERT(m_dialog);
    Q_ASSERT(m_thread);
    if (!m_dialog || !m_thread || !m_decoder) return;

    // we received a buffer -> update the progress bar
    updateBufferProgressBar();

    const RecordParams &params = m_dialog->params();

    const unsigned int tracks = params.tracks;
    Q_ASSERT(tracks);
    if (!tracks) return;

    const unsigned int bytes_per_sample = m_decoder->rawBytesPerSample();
    Q_ASSERT(bytes_per_sample);
    if (!bytes_per_sample) return;

    unsigned int samples = buffer.size() / bytes_per_sample / tracks;
    Q_ASSERT(samples);
    if (!samples) return;

    // check for reached recording time limit if enabled
    if (params.record_time_limited) {
	const unsigned int already_recorded = m_writers.last()+1;
	const unsigned int limit = (unsigned int)rint(
	    params.record_time * params.sample_rate);
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
    Q_ASSERT(buf.size() == bytes_per_sample * samples);
    if (buf.size() != bytes_per_sample * samples) return;

    QMemArray<sample_t> decoded;
    decoded.resize(samples);
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

    // decode and care for all special effects, meters and so on
    for (unsigned int track=0; track < tracks; ++track) {
	// split off and decode buffer with current track
	split(buffer, buf, bytes_per_sample, track, tracks);
	m_decoder->decode(buf, decoded);

	// update the level meter and other effects
	m_dialog->updateEffects(track, decoded);
    }

    // if the first buffer is full -> leave REC_BUFFERING
    if ((m_state==REC_BUFFERING) && (m_buffers_recorded > 1) && buffer.size())
	    m_controller.deviceBufferFull();

    // use a copy of the state, in case it changes below ;-)
    RecordState state = m_state;
    for (unsigned int track=0; track < tracks; ++track) {
	switch (state) {
	    case REC_EMPTY:
		break;
	    case REC_BUFFERING:
	    case REC_WAITING_FOR_TRIGGER:
		// already handled before...
		break;
	    case REC_PRERECORDING:
		// enqueue the buffers into a FIFO
		enqueuePrerecording(track, decoded);
		break;
	    case REC_RECORDING: {
		// put the decoded track data into the buffer
		Q_ASSERT(tracks == m_writers.count());
		if (!tracks || (tracks != m_writers.count())) break;

		SampleWriter *writer = m_writers[track];
		Q_ASSERT(writer);
		if (writer) (*writer) << decoded;
		m_controller.setEmpty(false);

		break;
	    }
	    case REC_PAUSED:
		break;
	    case REC_DONE:
		break;
	}
    }

    // update the number of recorded samples
    emit sigRecordedSamples(m_writers.last()+1);

    // if this was the last received buffer, change state
    if (recording_done && (m_state != REC_DONE) && (m_state != REC_EMPTY)) {
	m_controller.actionStop();
	return;
    }

}

//***************************************************************************
//***************************************************************************
