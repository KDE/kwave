/***************************************************************************
      LowPassPlugin.cpp  -  Plugin for simple lowpass filtering
                             -------------------
    begin                : Fri Mar 07 2003
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
#include <klocale.h>
#include <arts/artsflow.h>
#include <arts/connect.h>
#include <arts/objectmanager.h>
#include <arts/dynamicrequest.h>

#include "libkwave/ArtsNativeMultiTrackFilter.h"

#include "LowPassPlugin.h"
#include "LowPassDialog.h"

KWAVE_PLUGIN(LowPassPlugin,"lowpass","Thomas Eschenbacher");

//***************************************************************************
LowPassPlugin::LowPassPlugin(const PluginContext &context)
    :KwaveFilterPlugin(context),
     m_frequency(3500.0), m_last_freq(100)
{
     i18n("lowpass");
}

//***************************************************************************
LowPassPlugin::~LowPassPlugin()
{
}

//***************************************************************************
int LowPassPlugin::interpreteParameters(QStringList &params)
{
    bool ok;
    QString param;

    // evaluate the parameter list
    if (params.count() != 1) return -EINVAL;

    param = params[0];
    m_frequency = param.toDouble(&ok);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    return 0;
}

//***************************************************************************
KwavePluginSetupDialog *LowPassPlugin::createDialog(QWidget *parent)
{
    LowPassDialog *dialog = new LowPassDialog(parent, signalRate());
    Q_ASSERT(dialog);
    if (!dialog) return 0;

    // connect the signals for detecting value changes in pre-listen mode
    connect(dialog, SIGNAL(changed(double)),
            this, SLOT(setValue(double)));

    return dialog;
}

//***************************************************************************
ArtsMultiTrackFilter *LowPassPlugin::createFilter(unsigned int tracks)
{
    return new ArtsNativeMultiTrackFilter(
        tracks,"Arts::Synth_SHELVE_CUTOFF"
    );
}

//***************************************************************************
bool LowPassPlugin::paramsChanged()
{
    return (m_frequency != m_last_freq);
}

//***************************************************************************
void LowPassPlugin::updateFilter(ArtsMultiTrackFilter *filter,
                                 bool force)
{
    if (!filter) return;

    if ((m_frequency != m_last_freq) || force)
	filter->setValue("frequency", m_frequency);

    m_last_freq  = m_frequency;
}

//***************************************************************************
QString LowPassPlugin::actionName()
{
    return i18n("low pass");
}

//***************************************************************************
void LowPassPlugin::setValue(double frequency)
{
    m_frequency = frequency;
}

//***************************************************************************
//***************************************************************************
