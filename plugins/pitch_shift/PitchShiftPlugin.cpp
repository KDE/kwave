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

#include <QStringList>

#include <klocale.h>

#include "libkwave/MultiTrackSource.h"
#include "PitchShiftFilter.h"
#include "PitchShiftPlugin.h"
#include "PitchShiftDialog.h"

KWAVE_PLUGIN(PitchShiftPlugin, "pitch_shift", "2.1",
             I18N_NOOP("Pitch Shift"), "Thomas Eschenbacher");

//***************************************************************************
PitchShiftPlugin::PitchShiftPlugin(const PluginContext &context)
    :Kwave::FilterPlugin(context),
     m_speed(1.0), m_frequency(5.0), m_percentage_mode(false),
     m_last_speed(0), m_last_freq(0)
{
}

//***************************************************************************
PitchShiftPlugin::~PitchShiftPlugin()
{
}

//***************************************************************************
int PitchShiftPlugin::interpreteParameters(QStringList &params)
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
KwavePluginSetupDialog *PitchShiftPlugin::createDialog(QWidget *parent)
{
    PitchShiftDialog *dialog = new PitchShiftDialog(parent);
    Q_ASSERT(dialog);
    if (!dialog) return 0;

    // connect the signals for detecting value changes in pre-listen mode
    connect(dialog, SIGNAL(changed(double,double)),
            this, SLOT(setValues(double,double)));

    return dialog;
}

//***************************************************************************
Kwave::SampleSource *PitchShiftPlugin::createFilter(unsigned int tracks)
{
    return new Kwave::MultiTrackSource<PitchShiftFilter, true>(tracks);
}

//***************************************************************************
bool PitchShiftPlugin::paramsChanged()
{
    return ((m_speed != m_last_speed) || (m_frequency != m_last_freq));
}

//***************************************************************************
void PitchShiftPlugin::updateFilter(Kwave::SampleSource *filter,
                                    bool force)
{
    double sr = signalRate();

    if (!filter) return;

    if ((m_frequency != m_last_freq) || force)
	filter->setAttribute(SLOT(setFrequency(const QVariant)),
	    QVariant((m_frequency * 2.0 * M_PI) / sr));

    if ((m_speed != m_last_speed) || force)
	filter->setAttribute(SLOT(setSpeed(const QVariant)),
	    QVariant(m_speed));

    m_last_freq  = m_frequency;
    m_last_speed = m_speed;
}

//***************************************************************************
QString PitchShiftPlugin::actionName()
{
    return i18n("Pitch Shift");
}

//***************************************************************************
void PitchShiftPlugin::setValues(double speed, double frequency)
{
    m_frequency = frequency;
    m_speed     = speed;
}

//***************************************************************************
#include "PitchShiftPlugin.moc"
//***************************************************************************
//***************************************************************************
