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

#include <errno.h>
#include <math.h>
#include <new>

#include <KLocalizedString>
#include <QStringList>

#include "libkwave/PluginManager.h"

#include "BandPass.h"
#include "BandPassDialog.h"
#include "BandPassPlugin.h"
#include "libkwave/MultiTrackSource.h"

KWAVE_PLUGIN(band_pass, BandPassPlugin)

//***************************************************************************
Kwave::BandPassPlugin::BandPassPlugin(QObject *parent,
                                      const QVariantList &args)
    :Kwave::FilterPlugin(parent, args),
     m_frequency(3500.0), m_last_freq(100),m_bw(100),m_last_bw(200)
{
}

//***************************************************************************
Kwave::BandPassPlugin::~BandPassPlugin()
{
}

//***************************************************************************
int Kwave::BandPassPlugin::interpreteParameters(QStringList &params)
{
    bool ok;
    QString param;

    if (params.isEmpty()) return -EINVAL;

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
Kwave::PluginSetupDialog *Kwave::BandPassPlugin::createDialog(QWidget *parent)
{
    Kwave::BandPassDialog *dialog =
        new(std::nothrow) Kwave::BandPassDialog(parent, signalRate());
    Q_ASSERT(dialog);
    if (!dialog) return nullptr;

    // connect the signals for detecting value changes in pre-listen mode
    connect(dialog, SIGNAL(freqChanged(double)),
            this, SLOT(setFreqValue(double)));
    connect(dialog, SIGNAL(bwChanged(double)),
            this, SLOT(setBwValue(double)));
    return dialog;
}

//***************************************************************************
Kwave::SampleSource *Kwave::BandPassPlugin::createFilter(unsigned int tracks)
{
    return new(std::nothrow)
        Kwave::MultiTrackSource<Kwave::BandPass, true>(tracks);
}

//***************************************************************************
bool Kwave::BandPassPlugin::paramsChanged()
{
    return (!qFuzzyCompare(m_frequency, m_last_freq) ||
            !qFuzzyCompare(m_bw, m_last_bw));
}

//***************************************************************************
void Kwave::BandPassPlugin::updateFilter(Kwave::SampleSource *filter,
                                         bool force)
{
    double sr = signalRate();

    if (!filter) return;

    if (!qFuzzyCompare(m_frequency, m_last_freq) || force)
        filter->setAttribute(SLOT(setFrequency(QVariant)),
            QVariant((m_frequency * 2.0 * M_PI) / sr));

    if (!qFuzzyCompare(m_bw, m_last_bw) || force)
        filter->setAttribute(SLOT(setBandwidth(QVariant)),
            QVariant((m_bw * 2.0 * M_PI) / sr));

    m_last_freq  = m_frequency;
    m_last_bw    = m_bw;
}

//***************************************************************************
QString Kwave::BandPassPlugin::actionName()
{
    return i18n("Band Pass");
}

//***************************************************************************
void Kwave::BandPassPlugin::setFreqValue(double frequency)
{
    m_frequency = frequency;
}

//***************************************************************************
void Kwave::BandPassPlugin::setBwValue(double bw)
{
    m_bw = bw;
}

//***************************************************************************
#include "BandPassPlugin.moc"
//***************************************************************************
//***************************************************************************

#include "moc_BandPassPlugin.cpp"
