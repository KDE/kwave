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

#include <qdatetime.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <qvariant.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <kglobal.h>
#include <kmessagebox.h>

#include "libkwave/CompressionType.h"
#include "libkwave/FileInfo.h"
#include "libkwave/InsertMode.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleFormat.h"
#include "libkwave/SampleWriter.h"

#include "kwave/PluginManager.h"
#include "kwave/SignalManager.h"
#include "kwave/TopWidget.h"

#include "RecordDevice.h"
#include "RecordDialog.h"
#include "RecordPlugin.h"
#include "RecordThread.h"
#include "SampleDecoder.h"
#include "SampleDecoderLinear.h"

KWAVE_PLUGIN(RecordPlugin,"record","Thomas Eschenbacher");

//***************************************************************************
RecordPlugin::RecordPlugin(PluginContext &context)
    :KwavePlugin(context), m_state(REC_EMPTY), m_device(0),
     m_dialog(0), m_thread(0), m_decoder(0), m_writers(),
     m_buffers_recorded(0), m_inhibit_count(0), m_trigger_value()
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

    // use a new record controller
    RecordController controller;

    // create the setup dialog
    m_dialog = new RecordDialog(parentWidget(), previous_params, &controller);
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
    connect(m_dialog, SIGNAL(deviceChanged(const QString &)),
            this, SLOT(changeDevice(const QString &)));
    connect(m_dialog, SIGNAL(sigTracksChanged(unsigned int)),
            this, SLOT(changeTracks(unsigned int)));
    connect(m_dialog, SIGNAL(sampleRateChanged(double)),
            this, SLOT(changeSampleRate(double)));
    connect(m_dialog, SIGNAL(sigCompressionChanged(int)),
            this, SLOT(changeCompression(int)));
    connect(m_dialog, SIGNAL(sigBitsPerSampleChanged(unsigned int)),
            this, SLOT(changeBitsPerSample(unsigned int)));
    connect(m_dialog, SIGNAL(sigSampleFormatChanged(int)),
            this, SLOT(changeSampleFormat(int)));
    connect(m_dialog, SIGNAL(sigBuffersChanged()),
            this, SLOT(buffersChanged()));

    connect(m_dialog, SIGNAL(sigTriggerChanged(bool)),
            &controller, SLOT(enableTrigger(bool)));
    controller.enableTrigger(m_dialog->params().record_trigger_enabled);

    // connect the record controller and this
    connect(&controller, SIGNAL(sigReset(bool &)),
            this, SLOT(resetRecording(bool &)));
    connect(&controller, SIGNAL(sigStartRecord()),
            this, SLOT(startRecording()));
    connect(&controller, SIGNAL(sigStopRecord(int)),
            &controller, SLOT(deviceRecordStopped(int)));
    connect(&controller, SIGNAL(stateChanged(RecordState)),
            this, SLOT(stateChanged(RecordState)));

    // connect record controller and record thread
    connect(this,        SIGNAL(sigStarted()),
            &controller, SLOT(deviceRecordStarted()));
    connect(this,        SIGNAL(sigBufferFull()),
            &controller, SLOT(deviceBufferFull()));
    connect(this, SIGNAL(sigTriggerReached()),
            &controller, SLOT(deviceTriggerReached()));
    connect(m_thread,    SIGNAL(stopped(int)),
            &controller, SLOT(deviceRecordStopped(int)));

    // connect us to the record thread
    connect(m_thread, SIGNAL(stopped(int)),
            this,     SLOT(recordStopped(int)));
    connect(m_thread, SIGNAL(bufferFull(QByteArray)),
            this,     SLOT(processBuffer(QByteArray)));

    // select the record device
    changeDevice(m_dialog->params().device_name);

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

    if (m_dialog) delete m_dialog;
    m_dialog = 0;

    if (m_thread) delete m_thread;
    m_thread = 0;

    if (m_decoder) delete m_decoder;
    m_decoder = 0;

    return list;
}

//***************************************************************************
void RecordPlugin::changeDevice(const QString &dev)
{
    Q_ASSERT(m_dialog);
    if (!m_dialog) return;
    qDebug("RecordPlugin::changeDevice("+dev+")");

    InhibitRecordGuard _lock(*this); // don't record while settings change

    // open the new device
    if (m_device) m_device->close();
    // TODO: currently we only have OSS support, so forget about
    // the following lines for now
//     if (m_device && !m_device->supportsDevice(dev)) {
// 	delete m_device;
// 	// find out which device subclass supports our "dev" and
// 	// then create a new one
	m_device = new RecordDevice();
//     }
    Q_ASSERT(m_device);
    if (!m_device) {
	KMessageBox::sorry(m_dialog, i18n("Out of memory"));
	return;
    }

    // check the settings of the device and set new parameters if necessary
    int res = m_device->open(dev);
    if (res < 0) {
	// snap back to the previous device
	// ASSUMPTION: the previous device already worked !?
	res = m_device->open(m_dialog->params().device_name);
	if (res < 0) {
	    // oops: previous device did also not work
	    KMessageBox::sorry(m_dialog,
		i18n("Opening the device %1 failed, "
		     "please select a different one.").arg(
		     dev));
	    m_device->close();
	    m_dialog->setDevice("");
	} else {
	    KMessageBox::sorry(m_dialog,
		i18n("Opening the device %1 failed, reverting to %2.").arg(
		dev).arg(m_dialog->params().device_name));
	    m_dialog->setDevice(m_dialog->params().device_name);
	}
	return;
    }

    // detect minimum and maximm number of tracks
    unsigned int min_tracks, max_tracks;
    res = m_device->detectTracks(min_tracks, max_tracks);
    if (res < 0) {
	// determining the number of tracks failed
	// -> snap back to the previous device
	// ASSUMPTION: the previous device already worked !?
	KMessageBox::sorry(m_dialog,
	    i18n("The record device '%1' does not work properly, "\
	    "determining the number of supported channels failed.").arg(
	    dev));
	res = m_device->open(m_dialog->params().device_name);
	if (res < 0) {
	    // oops: previous device did also not work
	    m_device->close();
	    m_dialog->setDevice("");
	}
	return;
    }

    // opening the device succeeded, take it :-)
    m_dialog->setDevice(dev);

    // start with setting the number of tracks, the rest
    // comes automatically:
    // tracks -> sample rate -> bits per sample -> sample format
    m_dialog->setSupportedTracks(min_tracks, max_tracks);
    changeTracks(m_dialog->params().tracks);
}

//***************************************************************************
void RecordPlugin::changeTracks(unsigned int new_tracks)
{
    Q_ASSERT(m_device);
    Q_ASSERT(m_dialog);
    if (!m_device || !m_dialog) return;

    InhibitRecordGuard _lock(*this); // don't record while settings change

    // try to activate the new number of tracks
    unsigned int tracks = new_tracks;
    int err = m_device->setTracks(tracks);
    if (err < 0) {
	// revert to the current device setting if failed
	tracks = m_device->tracks();
	KMessageBox::sorry(m_dialog,
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
    Q_ASSERT(m_device);
    Q_ASSERT(m_dialog);
    if (!m_device || !m_dialog) return;

    InhibitRecordGuard _lock(*this); // don't record while settings change

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
	KMessageBox::sorry(m_dialog,
	    i18n("The sample rate %1Hz is not supported, using %2Hz instead."
	    ).arg(sr1).arg(sr2));
    }
    m_dialog->setSupportedSampleRates(supported_rates);

    // try to activate the new sample rate
    int err = m_device->setSampleRate(rate);
    if (err < 0) {
	// revert to the current device setting if failed
	rate = m_device->sampleRate();

	const QString sr1(m_dialog->rate2string(new_rate));
	const QString sr2(m_dialog->rate2string(rate));
	KMessageBox::sorry(m_dialog,
	    i18n("Setting the sample rate %1Hz failed, "\
		 "using %2Hz instead.").arg(sr1).arg(sr2));
    }
    m_dialog->setSampleRate(rate);

    // set the compression again
    changeCompression(m_dialog->params().compression);
}

//***************************************************************************
void RecordPlugin::changeCompression(int new_compression)
{
    Q_ASSERT(m_device);
    Q_ASSERT(m_dialog);
    if (!m_device || !m_dialog) return;

    InhibitRecordGuard _lock(*this); // don't record while settings change

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
	const QString c1(types.name(types.findFromData(new_compression)));
	const QString c2(types.name(types.findFromData(compression)));
	KMessageBox::sorry(m_dialog,
	    i18n("The compression '%1' is not supported, "\
	         "using '%2' instead.").arg(c1).arg(c2));
    }
    m_dialog->setSupportedCompressions(supported_comps);

    // try to activate the new compression
    int err = m_device->setCompression(compression);
    if (err < 0) {
	// revert to the current device setting if failed
	compression = m_device->compression();

	CompressionType types;
	const QString c1(types.name(types.findFromData(compression)));
	const QString c2(types.name(types.findFromData(
	                 m_device->compression())));
	KMessageBox::sorry(m_dialog,
	    i18n("Setting the compression type %1 failed, "\
		 "using %2 instead.").arg(c1).arg(c2));
    }
    m_dialog->setCompression(compression);

    // set the resolution in bits per sample again
    changeBitsPerSample(m_dialog->params().bits_per_sample);
}

//***************************************************************************
void RecordPlugin::changeBitsPerSample(unsigned int new_bits)
{
    Q_ASSERT(m_device);
    Q_ASSERT(m_dialog);
    if (!m_device || !m_dialog) return;

    InhibitRecordGuard _lock(*this); // don't record while settings change

    // check the supported resolution in bits per sample
    QValueList<unsigned int> supported_bits = m_device->detectBitsPerSample();
    int bits = new_bits;
    if (!supported_bits.contains(bits)) {
	// find the nearest resolution
	int nearest = supported_bits.last();
	QValueList<unsigned int>::Iterator it;
	for (it=supported_bits.begin(); it != supported_bits.end(); ++it) {
	    if (abs((int)*it - nearest) <= abs(bits - nearest))
	        nearest = (int)*it;
	}
	bits = nearest;

	KMessageBox::sorry(m_dialog,
	    i18n("The resolution %1 bits per sample is not supported, "\
	         "using %2 bits per sample instead.").arg(
		 (int)new_bits).arg(bits));
    }
    m_dialog->setSupportedBitsPerSample(supported_bits);

    // try to activate the resolution
    int err = m_device->setBitsPerSample(bits);
    if (err < 0) {
	// revert to the current device setting if failed
	bits = m_device->bitsPerSample();
	KMessageBox::sorry(m_dialog,
	    i18n("Setting the resolution % bits per sample failed, "\
		 "using %2 bits per sample instead.").arg(
		 (int)new_bits).arg(bits));
    }
    m_dialog->setBitsPerSample(bits);

    // set the sample format again
    changeSampleFormat(m_dialog->params().sample_format);
}

//***************************************************************************
void RecordPlugin::changeSampleFormat(int new_format)
{
    Q_ASSERT(m_device);
    Q_ASSERT(m_dialog);
    if (!m_device || !m_dialog) return;

    InhibitRecordGuard _lock(*this); // don't record while settings change

    // check the supported sample formats
    QValueList<int> supported_formats = m_device->detectSampleFormats();
    int format = new_format;
    if (!supported_formats.contains(format)) {
	// use the device default instead
	format = m_device->sampleFormat();

	SampleFormat types;
	const QString s1 = types.name(types.findFromData(new_format));
	const QString s2 = types.name(types.findFromData(format));
	KMessageBox::sorry(m_dialog,
	    i18n("The sample format '%1' is not supported, "\
	         "using '%2' instead.").arg(s1).arg(s2));
    }
    m_dialog->setSupportedSampleFormats(supported_formats);

    // try to activate the new format
    int err = m_device->setSampleFormat(format);
    if (err < 0) {
	// use the device default instead
	format = m_device->sampleFormat();

	SampleFormat types;
	const QString s1 = types.name(types.findFromData(new_format));
	const QString s2 = types.name(types.findFromData(format));
	KMessageBox::sorry(m_dialog,
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
	qDebug("RecordPlugin::enterInhibit() - STOPPING");
	m_thread->stop();
    }
}

//***************************************************************************
void RecordPlugin::leaveInhibit()
{
    Q_ASSERT(m_inhibit_count);
    if (m_inhibit_count) m_inhibit_count--;
    if (!m_inhibit_count && m_thread) {
	qDebug("RecordPlugin::leaveInhibit() - STARTING");

	Q_ASSERT(!m_thread->running());

	// set new parameters for the recorder
	setupRecordThread();

	// and let the thread run (again)
	m_thread->start();
    }
}

//***************************************************************************
void RecordPlugin::resetRecording(bool &accepted)
{
    qDebug("RecordPlugin::resetRecording()");
    InhibitRecordGuard _lock(*this);

    TopWidget &topwidget = this->manager().topWidget();
    accepted = topwidget.closeFile();
    if (!accepted) return;

    m_writers.flush();
    m_writers.resize(0);
    m_buffers_recorded = 0;
}

//***************************************************************************
void RecordPlugin::setupRecordThread()
{
    Q_ASSERT(m_thread);
    if (!m_thread) return;

    // stop the thread if necessary (should never happen)
    Q_ASSERT(!m_thread->running());
    if (m_thread->running()) m_thread->stop();
    Q_ASSERT(!m_thread->running());

    // delete the previous decoder
    if (m_decoder) delete m_decoder;
    m_decoder = 0;

    // create a decoder for the current sample format
    switch (m_dialog->params().compression) {
	case AF_COMPRESSION_NONE:
	    switch (m_dialog->params().sample_format) {
		case AF_SAMPFMT_UNSIGNED:
		case AF_SAMPFMT_TWOSCOMP:
		    // decoder for all linear formats
		    m_decoder = new SampleDecoderLinear(
			m_dialog->params().sample_format,
			m_dialog->params().bits_per_sample
		    );
		    break;
		default:
		    KMessageBox::sorry(m_dialog, i18n(
			"The current sample format is not supported!"
		    ));
	    }
	    break;
	case AF_COMPRESSION_G711_ALAW:
	case AF_COMPRESSION_G711_ULAW:
	case CompressionType::MPEG_LAYER_II:
	case AF_COMPRESSION_MS_ADPCM:
	default:
	    KMessageBox::sorry(m_dialog, i18n(
		"The current compression type is not supported!"));
	    return;
    }

    Q_ASSERT(m_decoder);
    if (!m_decoder) {
	KMessageBox::sorry(m_dialog, i18n("Out of memory"));
	return;
    }

    // set up the recording trigger values
    m_trigger_value.resize(m_dialog->params().tracks);
    m_trigger_value.fill((double)0.0);

    // set up the record thread
    m_thread->setRecordDevice(m_device);
    unsigned int buf_count = m_dialog->params().buffer_count;
    unsigned int buf_size  = m_dialog->params().tracks *
                             m_decoder->rawBytesPerSample() *
                            (1 << m_dialog->params().buffer_size);
    m_thread->setBuffers(buf_count, buf_size);
}

//***************************************************************************
void RecordPlugin::startRecording()
{
    Q_ASSERT(m_dialog);
    Q_ASSERT(m_thread);
    Q_ASSERT(m_device);
    if (!m_dialog || !m_thread || !m_device) return;

    qDebug("RecordPlugin::startRecording()");
    InhibitRecordGuard _lock(*this); // don't record while settings change

    if ((m_state != REC_PAUSED) || !m_decoder) {
	double rate = m_dialog->params().sample_rate;
	unsigned int tracks = m_dialog->params().tracks;
	unsigned int samples = 0;
	unsigned int bits = m_dialog->params().bits_per_sample;

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
	fileInfo().set(INF_SAMPLE_FORMAT, m_dialog->params().sample_format);
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
    emit sigStarted();
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
			       strerror(-reason));
    }
    KMessageBox::error(m_dialog, description);

    m_writers.flush();
    qDebug("RecordPlugin::recordStopped(): wrote %u samples",
           m_writers.last());
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
    while (samples--) {
	for (unsigned int byte=0; byte < bytes_per_sample; byte++) {
	    *dst = *src;
	    dst++;
	    src++;
	}
	src += increment;
    }
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
    double trigger = (double)m_dialog->params().record_trigger / 100.0;
    const double rate = (double)m_dialog->params().sample_rate;

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
    const double f_rise = 20.0;
    double Fg = f_rise / rate;
    double n = 1.0 / tan(M_PI * Fg);
    const double a0_r = 1.0 / (1.0 + n);
    const double b1_r = (1.0 - n) / (1.0 + n);

    // fall coefficient: ~1.0Hz
    const double f_fall = 1.0;
    Fg = f_fall / rate;
    n = 1.0 / tan(M_PI * Fg);
    const double a0_f = 1.0 / (1.0 + n);
    const double b1_f = (1.0 - n) / (1.0 + n);

    double y = m_trigger_value[track];
    double last_x = 0.0;
    for (unsigned int t=0; t < buffer.size(); ++t) {
	double x = fabs((double)buffer[t]) / (double)(1 << (SAMPLE_BITS-1));
	if (x < 0) x *= -1.0; /* rectifier */

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

    qDebug(">> level=%0.3g, trigger=%0.3g", y, trigger);

    return false;
}

//***************************************************************************
void RecordPlugin::processBuffer(QByteArray buffer)
{
//    qDebug("RecordPlugin::processBuffer()");
    Q_ASSERT(m_dialog);
    Q_ASSERT(m_thread);
    Q_ASSERT(m_decoder);
    if (!m_dialog || !m_thread || !m_decoder) return;

    // we received a buffer -> update the progress bar
    updateBufferProgressBar();

    // split into several single buffers
    const RecordParams &params = m_dialog->params();
    const unsigned int tracks = params.tracks;
    Q_ASSERT(tracks);

    const unsigned int bytes_per_sample = m_decoder->rawBytesPerSample();
    const unsigned int samples = buffer.size() / bytes_per_sample / tracks;

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
    //       processing all tracks !
    if (m_state == REC_WAITING_FOR_TRIGGER) {
	for (unsigned int track=0; track < tracks; ++track) {
	    // split off and decode buffer with current track
	    split(buffer, buf, bytes_per_sample, track, tracks);
	    m_decoder->decode(buf, decoded);
	    if (checkTrigger(track, decoded)) {
		emit sigTriggerReached();
		break;
	    }
	}
    }

    for (unsigned int track=0; track < tracks; ++track) {
	// split off and decode buffer with current track
	split(buffer, buf, bytes_per_sample, track, tracks);
	m_decoder->decode(buf, decoded);

	// update the level meter and other effects
	// ### TODO ###

	switch (m_state) {
	    case REC_EMPTY:
		break;
	    case REC_BUFFERING:
		// first buffer is full
		// -> leave REC_BUFFERING
		if ((m_buffers_recorded > 1) && buffer.size())
		    emit sigBufferFull();
		break;
	    case REC_PRERECORDING:
		// enqueue the buffers into a FIFO
		break;
	    case REC_WAITING_FOR_TRIGGER:
		// already handled before...
		break;
	    case REC_RECORDING: {
		// put the decoded track data into the buffer
		Q_ASSERT(tracks == m_writers.count());
		if (!tracks || (tracks != m_writers.count())) return;
		SampleWriter *writer = m_writers[track];
		Q_ASSERT(writer);
		if (writer) (*writer) << decoded;
		break;
	    }
	    case REC_PAUSED:
		break;
	    case REC_DONE:
		break;
	}
    }

}

//***************************************************************************
//***************************************************************************
