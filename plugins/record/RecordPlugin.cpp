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

#include <qstringlist.h>
#include <qvaluelist.h>
#include <kmessagebox.h>

#include "libkwave/CompressionType.h"
#include "libkwave/SampleFormat.h"

#include "RecordDevice.h"
#include "RecordDialog.h"
#include "RecordPlugin.h"
#include "RecordThread.h"

KWAVE_PLUGIN(RecordPlugin,"record","Thomas Eschenbacher");

class InhibitRecordGuard
{
public:
    InhibitRecordGuard(RecordPlugin &recorder)
    {
	// increment recursion count
	// is recording running?
	// yes -> stop it
    };

    virtual ~InhibitRecordGuard()
    {
	// decrement recursion count
	// count == zero?
	// yes -> was recording running before?
	// yes -> start it again
    };
};

//***************************************************************************
RecordPlugin::RecordPlugin(PluginContext &context)
    :KwavePlugin(context), m_state(REC_EMPTY), m_device(0),
     m_dialog(0), m_thread(0)
{
    i18n("record");
}

//***************************************************************************
RecordPlugin::~RecordPlugin()
{
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

    // connect the record controller
    connect(&controller, SIGNAL(sigReset()),
            this, SLOT(resetRecording()));
    connect(&controller, SIGNAL(sigStartRecord()),
            this, SLOT(startRecording()));
    connect(&controller, SIGNAL(sigStopRecord()),
            this, SLOT(stopRecording()));
    connect(&controller, SIGNAL(stateChanged(RecordState)),
            this, SLOT(stateChanged(RecordState)));

    connect(this, SIGNAL(sigStarted()),
            &controller, SLOT(deviceRecordStarted()));
    connect(this, SIGNAL(sigBufferFull()),
            &controller, SLOT(deviceBufferFull()));
    connect(this, SIGNAL(sigTriggerReached()),
            &controller, SLOT(deviceTriggerReached()));
    connect(this, SIGNAL(sigStopped()),
            &controller, SLOT(deviceRecordStopped()));

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

    return list;
}

//***************************************************************************
void RecordPlugin::changeDevice(const QString &dev)
{
    Q_ASSERT(m_dialog);
    if (!m_dialog) return;
    qDebug("RecordPlugin::changeDevice("+dev+")");

    InhibitRecordGuard(*this); // suspend recording while changing settings

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

    InhibitRecordGuard(*this); // suspend recording while changing settings

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

    InhibitRecordGuard(*this); // suspend recording while changing settings

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

    InhibitRecordGuard(*this); // suspend recording while changing settings

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

    InhibitRecordGuard(*this); // suspend recording while changing settings

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

    InhibitRecordGuard(*this); // suspend recording while changing settings

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
void RecordPlugin::resetRecording()
{
    qDebug("RecordPlugin::resetRecording()");

    // delete all recorded threads and start again...

}

//***************************************************************************
void RecordPlugin::startRecording()
{
    Q_ASSERT(m_dialog);
    Q_ASSERT(m_thread);
    Q_ASSERT(m_device);
    if (!m_dialog || !m_thread || !m_device) return;

    qDebug("RecordPlugin::startRecording()");

    // create new and empty tracks
    // ### TODO ###

    // stop the device if necessary (should never happen)
    Q_ASSERT(!m_thread->running());
    if (m_thread->running()) m_thread->stop();
    Q_ASSERT(!m_thread->running());

    // start the recording thread
    m_thread->setRecordDevice(m_device);
    m_thread->setBuffers(32, 65536);
    m_thread->start();

    // do the recording stuff...
    // ### TODO ###
    // while (!stopped) {
    //    record
    // }

    // update the file info
    // ### TODO ###
}

//***************************************************************************
void RecordPlugin::stopRecording()
{
    Q_ASSERT(m_dialog);
    Q_ASSERT(m_thread);
    if (!m_dialog || !m_thread) return;

    qDebug("RecordPlugin::stopRecording()");
    m_thread->stop();
}

//***************************************************************************
void RecordPlugin::stateChanged(RecordState state)
{
    qDebug("RecordPlugin::stateChanged(%d)", (int)state);
}

//***************************************************************************
//***************************************************************************
