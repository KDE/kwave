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

#include "RecordDevice.h"
#include "RecordDialog.h"
#include "RecordPlugin.h"

KWAVE_PLUGIN(RecordPlugin,"record","Thomas Eschenbacher");

//***************************************************************************
RecordPlugin::RecordPlugin(PluginContext &context)
    :KwavePlugin(context), m_params(), m_device(0), m_dialog(0)
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

    m_params.fromList(previous_params);

    // create the setup dialog
    m_dialog = new RecordDialog(parentWidget(), m_params);
    Q_ASSERT(m_dialog);
    if (!m_dialog) return 0;

    // connect some signals of the setup dialog
    connect(m_dialog, SIGNAL(deviceChanged(const QString &)),
            this, SLOT(changeDevice(const QString &)));
    connect(m_dialog, SIGNAL(sampleRateChanged(double)),
            this, SLOT(changeSampleRate(double)));

    // select the playback device
    changeDevice(m_params.device_name);

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
    return list;
}

//***************************************************************************
void RecordPlugin::changeDevice(const QString &dev)
{
    Q_ASSERT(m_dialog);
    if (!m_dialog) return;
    qDebug("RecordPlugin::changeDevice("+dev+")");

    int res;

    // abort current pre-record operation

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
    res = m_device->open(dev);
    if (res < 0) {
	// snap back to the previous device
	// ASSUMPTION: the previous device already worked !?
	res = m_device->open(m_params.device_name);
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
		dev).arg(m_params.device_name));
	    m_dialog->setDevice(m_params.device_name);
	}
	return;
    }

    // opening the device succeeded, take it :-)
    m_params.device_name = dev;
    m_dialog->setDevice(m_params.device_name);

    // re-select the sample rate
    QValueList<double> supported_rates = m_device->detectSampleRates();
    double current_rate = m_params.sample_rate;
    if (!supported_rates.contains(current_rate)) {
	// find the nearest sample rate
	double nearest = -1.0;
	QValueList<double>::Iterator it;
	for (it=supported_rates.begin(); it != supported_rates.end(); ++it) {
	    if (fabs(*it - nearest) <= fabs(current_rate - nearest))
	        nearest = *it;
	}
	KMessageBox::sorry(m_dialog,
	    i18n("The sample rate %1Hz is not supported, using %2Hz instead."
	    ).arg(current_rate).arg(nearest));
	current_rate = nearest;
    }
    m_dialog->setSupportedSampleRates(supported_rates);

    // activate the new sample rate
    changeSampleRate(current_rate);

    // resume pre-record operation if necessary
}

//***************************************************************************
void RecordPlugin::changeSampleRate(double new_rate)
{
    Q_ASSERT(m_device);
    Q_ASSERT(m_dialog);
    if (m_device) {
	double rate = new_rate;
	int err = m_device->setSampleRate(rate);
	if (err < 0) {
	    // revert to the last known rate, assume that it worked
	    KMessageBox::sorry(m_dialog,
		i18n("Setting the sample rate %1Hz failed, using the previous"\
		     "value or the devive default instead.").arg((int)new_rate));
	    new_rate = m_params.sample_rate;
	}
    }
    m_params.sample_rate = new_rate;
    if (m_dialog) m_dialog->setSampleRate(m_params.sample_rate);
}

//***************************************************************************
//***************************************************************************
