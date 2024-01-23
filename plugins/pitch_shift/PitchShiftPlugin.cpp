/***************************************************************************
   PitchShiftPlugin.cpp  -  plugin for modifying the "pitch_shift"
                             -------------------
    begin                : Sun Mar 23 2003
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

#include "PitchShiftDialog.h"
#include "PitchShiftFilter.h"
#include "PitchShiftPlugin.h"
#include "libkwave/MultiTrackSource.h"

KWAVE_PLUGIN(pitch_shift, PitchShiftPlugin)

//***************************************************************************
Kwave::PitchShiftPlugin::PitchShiftPlugin(QObject *parent,
                                          const QVariantList &args)
    :Kwave::FilterPlugin(parent, args),
     m_speed(1.0), m_frequency(5.0), m_percentage_mode(false),
     m_last_speed(0), m_last_freq(0)
{
}

//***************************************************************************
Kwave::PitchShiftPlugin::~PitchShiftPlugin()
{
}

//***************************************************************************
int Kwave::PitchShiftPlugin::interpreteParameters(QStringList &params)
{
    bool ok;
    QString param;

    // evaluate the parameter list
    if (params.count() != 3) return -EINVAL;

    param = params[0];
    m_speed = param.toDouble(&ok);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    param = params[1];
    m_frequency = param.toDouble(&ok);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    param = params[2];
    m_percentage_mode = (param.toUInt(&ok));
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    return 0;
}

//***************************************************************************
Kwave::PluginSetupDialog *Kwave::PitchShiftPlugin::createDialog(QWidget *parent)
{
    Kwave::PitchShiftDialog *dialog = new(std::nothrow)
        Kwave::PitchShiftDialog(parent);
    Q_ASSERT(dialog);
    if (!dialog) return Q_NULLPTR;

    // connect the signals for detecting value changes in pre-listen mode
    connect(dialog, SIGNAL(changed(double,double)),
            this, SLOT(setValues(double,double)));

    return dialog;
}

//***************************************************************************
Kwave::SampleSource *Kwave::PitchShiftPlugin::createFilter(unsigned int tracks)
{
    return new(std::nothrow)
        Kwave::MultiTrackSource<Kwave::PitchShiftFilter, true>(tracks);
}

//***************************************************************************
bool Kwave::PitchShiftPlugin::paramsChanged()
{
    return (!qFuzzyCompare(m_speed, m_last_speed) ||
            !qFuzzyCompare(m_frequency, m_last_freq));
}

//***************************************************************************
void Kwave::PitchShiftPlugin::updateFilter(Kwave::SampleSource *filter,
                                           bool force)
{
    double sr = signalRate();

    if (!filter) return;

    if (!qFuzzyCompare(m_frequency, m_last_freq) || force)
        filter->setAttribute(SLOT(setFrequency(QVariant)),
            QVariant((m_frequency * 2.0 * M_PI) / sr));

    if (!qFuzzyCompare(m_speed, m_last_speed) || force)
        filter->setAttribute(SLOT(setSpeed(QVariant)),
            QVariant(m_speed));

    m_last_freq  = m_frequency;
    m_last_speed = m_speed;
}

//***************************************************************************
QString Kwave::PitchShiftPlugin::actionName()
{
    return i18n("Pitch Shift");
}

//***************************************************************************
void Kwave::PitchShiftPlugin::setValues(double speed, double frequency)
{
    m_frequency = frequency;
    m_speed     = speed;
}

//***************************************************************************
#include "PitchShiftPlugin.moc"
//***************************************************************************
//***************************************************************************
