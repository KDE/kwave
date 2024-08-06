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
#include <math.h>
#include <new>

#include <QStringList>

#include <KLocalizedString>

#include "LowPassDialog.h"
#include "LowPassFilter.h"
#include "LowPassPlugin.h"
#include "libkwave/MultiTrackSource.h"

KWAVE_PLUGIN(lowpass, LowPassPlugin)

//***************************************************************************
Kwave::LowPassPlugin::LowPassPlugin(QObject *parent,
                                    const QVariantList &args)
    :Kwave::FilterPlugin(parent, args),
     m_frequency(3500.0), m_last_freq(100)
{
}

//***************************************************************************
Kwave::LowPassPlugin::~LowPassPlugin()
{
}

//***************************************************************************
int Kwave::LowPassPlugin::interpreteParameters(QStringList &params)
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
Kwave::PluginSetupDialog *Kwave::LowPassPlugin::createDialog(QWidget *parent)
{
    Kwave::LowPassDialog *dialog =
        new(std::nothrow) Kwave::LowPassDialog(parent, signalRate());
    Q_ASSERT(dialog);
    if (!dialog) return Q_NULLPTR;

    // connect the signals for detecting value changes in pre-listen mode
    connect(dialog, SIGNAL(changed(double)),
            this, SLOT(setValue(double)));

    return dialog;
}

//***************************************************************************
Kwave::SampleSource *Kwave::LowPassPlugin::createFilter(unsigned int tracks)
{
    return new(std::nothrow)
        Kwave::MultiTrackSource<Kwave::LowPassFilter, true>(tracks);
}

//***************************************************************************
bool Kwave::LowPassPlugin::paramsChanged()
{
    return (!qFuzzyCompare(m_frequency, m_last_freq));
}

//***************************************************************************
void Kwave::LowPassPlugin::updateFilter(Kwave::SampleSource *filter,
                                 bool force)
{
    double sr = signalRate();

    if (!filter) return;

    if (!qFuzzyCompare(m_frequency, m_last_freq) || force)
        filter->setAttribute(SLOT(setFrequency(QVariant)),
            QVariant((m_frequency * 2.0 * M_PI) / sr));

    m_last_freq  = m_frequency;
}

//***************************************************************************
QString Kwave::LowPassPlugin::actionName()
{
    return i18n("Low Pass");
}

//***************************************************************************
void Kwave::LowPassPlugin::setValue(double frequency)
{
    m_frequency = frequency;
}

//***************************************************************************
#include "LowPassPlugin.moc"
//***************************************************************************
//***************************************************************************

#include "moc_LowPassPlugin.cpp"
