/***************************************************************************
      BandPassPlugin.cpp  -  Plugin for band pass filtering
                             -------------------
    begin                : Tue Jun 24 2003
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
#include <math.h>
#include <errno.h>
#include <qstringlist.h>
#include <klocale.h>
#include <arts/artsflow.h>
#include <arts/connect.h>
#include <arts/objectmanager.h>
#include <arts/dynamicrequest.h>

//#include "libkwave/ArtsNativeMultiTrackFilter.h"
#include "libkwave/ArtsKwaveMultiTrackFilter.h"

#include "BandPassPlugin.h"
#include "BandPassDialog.h"
#include "synth_band_pass_impl.h"

KWAVE_PLUGIN(BandPassPlugin,"band_pass","Dave Flogeras");

//***************************************************************************
BandPassPlugin::BandPassPlugin(const PluginContext &context)
    :KwaveFilterPlugin(context),
     m_frequency(3500.0), m_last_freq(100),m_bw(100),m_last_bw(200)
{
}

//***************************************************************************
BandPassPlugin::~BandPassPlugin()
{
}

//***************************************************************************
int BandPassPlugin::interpreteParameters(QStringList &params)
{
    bool ok;
    QString param;

    // evaluate the parameter list
    Q_ASSERT(params.count() == 2);
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
KwavePluginSetupDialog *BandPassPlugin::createDialog(QWidget *parent)
{
    BandPassDialog *dialog = new BandPassDialog(parent, signalRate());
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
ArtsMultiTrackFilter *BandPassPlugin::createFilter(unsigned int tracks)
{

//  we use our own copy instead:
    return new ArtsKwaveMultiTrackFilter <Synth_BAND_PASS,
                              Synth_BAND_PASS_impl>(tracks);

}

//***************************************************************************
bool BandPassPlugin::paramsChanged()
{
    return (m_frequency != m_last_freq) || (m_bw != m_last_bw);
}

//***************************************************************************
void BandPassPlugin::updateFilter(ArtsMultiTrackFilter *filter,
                                 bool force)
{
    float sr = signalRate();

    if (!filter) return;

    if ((m_frequency != m_last_freq) || force)
	filter->setAttribute("frequency", (m_frequency*2*M_PI)/sr);

    if ((m_bw != m_last_bw) || force)
    filter->setAttribute("bw", (m_bw*2*M_PI)/sr);

    m_last_freq  = m_frequency;
    m_last_bw = m_bw;
}

//***************************************************************************
QString BandPassPlugin::actionName()
{
    return i18n("band_pass");
}

//***************************************************************************
void BandPassPlugin::setFreqValue(double frequency)
{
    m_frequency = frequency;
}

//***************************************************************************
void BandPassPlugin::setBwValue(double bw)
{
    m_bw = bw;
}

//***************************************************************************
#include "BandPassPlugin.moc"
//***************************************************************************
//***************************************************************************
