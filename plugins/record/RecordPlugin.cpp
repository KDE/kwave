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

#include <qstringlist.h>
#include <kmessagebox.h>

#include "RecordDialog.h"
#include "RecordPlugin.h"

KWAVE_PLUGIN(RecordPlugin,"record","Thomas Eschenbacher");

//***************************************************************************
RecordPlugin::RecordPlugin(PluginContext &context)
    :KwavePlugin(context), m_params(), m_device(), m_dialog(0)
{
    i18n("record");
}

//***************************************************************************
RecordPlugin::~RecordPlugin()
{
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

    // check the settings of the device and set new parameters if necessary

    // get bitmask of supported audio formats

    // - sample rate
    // - bits per sample
    // - number of tracks
    m_device.close();
    res = m_device.open(dev);
    if (res < 0) {
	// snap back to the previous device
	// ASSUMPTION: the previous device worked !?
	res = m_device.open(m_params.device_name);
	if (res < 0) {
	    // oops: previous device did also not work
	    KMessageBox::sorry(m_dialog,
		i18n("Opening the device %1 failed, "
		     "please select a different one.").arg(
		     dev));
	    m_device.close();
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
    QValueList<double> supported_rates = m_device.detectSampleRates();

    // resume pre-record operation if necessary
}

//***************************************************************************
//***************************************************************************
