/***************************************************************************
      NotchFilterPlugin.cpp  -  Plugin for simple notch filtering
                             -------------------
    begin                : Thu Jun 19 2003
    copyright            : (C) 2003 by Dave Flogeras
    email                : d.flogeras@unb.ca
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

#include "libkwave/ArtsKwaveMultiTrackFilter.h"

#include "NotchFilterPlugin.h"
#include "NotchFilterDialog.h"
#include "synth_notch_filter_impl.h"

KWAVE_PLUGIN(NotchFilterPlugin,"notch_filter","Dave Flogeras");

//***************************************************************************
NotchFilterPlugin::NotchFilterPlugin(PluginContext &context)
    :KwaveFilterPlugin(context),
     m_frequency(3500.0), m_last_freq(100),m_bw(100),m_last_bw(200) 
{
}

//***************************************************************************
NotchFilterPlugin::~NotchFilterPlugin()
{
}

//***************************************************************************
int NotchFilterPlugin::interpreteParameters(QStringList &params)
{
    bool ok;
    QString param;

    // evaluate the parameter list
    if (params.count() != 2) return -EINVAL;

    param = params[0];
    m_frequency = param.toDouble(&ok);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    param = params[1];
    m_bw = param.toDouble(&ok);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    return 0;
}

//***************************************************************************
KwavePluginSetupDialog *NotchFilterPlugin::createDialog(QWidget *parent)
{
    NotchFilterDialog *dialog = new NotchFilterDialog(parent, signalRate());
    Q_ASSERT(dialog);
    if (!dialog) return 0;

    // connect the signals for detecting value changes in pre-listen mode
    connect(dialog, SIGNAL(freqChanged(double)),
            this, SLOT(setFreqValue(double)));
    connect(dialog, SIGNAL(bwChanged(double)),
    	    this, SLOT(setBwValue(double)));
    return dialog;
}

//***************************************************************************
ArtsMultiTrackFilter *NotchFilterPlugin::createFilter(unsigned int tracks)
{
    return new ArtsKwaveMultiTrackFilter<Synth_NOTCH_FILTER,
                              Synth_NOTCH_FILTER_impl>(tracks);
}

//***************************************************************************
bool NotchFilterPlugin::paramsChanged()
{
    return (m_frequency != m_last_freq) || (m_bw != m_last_bw);
}

//***************************************************************************
void NotchFilterPlugin::updateFilter(ArtsMultiTrackFilter *filter,
                                 bool force)
{
    float sr = signalRate();

    if (!filter) return;

    if ((m_frequency != m_last_freq) || force)
	filter->setAttribute("frequency", (m_frequency*M_PI)/sr);

    if ((m_bw != m_last_bw) || force)
        filter->setAttribute("bw", (m_bw*M_PI)/sr);

    m_last_freq  = m_frequency;
    m_last_bw = m_bw;
}

//***************************************************************************
QString NotchFilterPlugin::actionName()
{
    return i18n("notch_filter");
}

//***************************************************************************
void NotchFilterPlugin::setFreqValue(double frequency)
{
    m_frequency = frequency;
}

//***************************************************************************
void NotchFilterPlugin::setBwValue(double bw)
{
    m_bw = bw;
}

//***************************************************************************
//***************************************************************************
