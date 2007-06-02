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
#include <qstringlist.h>
#include <klocale.h>
#include <arts/artsflow.h>
#include <arts/connect.h>
#include <arts/objectmanager.h>
#include <arts/dynamicrequest.h>

#include "libkwave/ArtsKwaveMultiTrackFilter.h"

#include "PitchShiftPlugin.h"
#include "PitchShiftDialog.h"
#include "synth_pitch_shift_bugfixed_impl.h" // bugfix for the aRts plugin

KWAVE_PLUGIN(PitchShiftPlugin,"pitch_shift","Thomas Eschenbacher");

//***************************************************************************
PitchShiftPlugin::PitchShiftPlugin(const PluginContext &context)
    :KwaveFilterPlugin(context),
     m_speed(1.0), m_frequency(5.0), m_percentage_mode(false),
     m_last_speed(0), m_last_freq(0)
{
     i18n("pitch_shift");
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
ArtsMultiTrackFilter *PitchShiftPlugin::createFilter(unsigned int tracks)
{
//  as long as the following original aRts module is buggy:
//  ArtsNativeMultiTrackFilter pitch(tracks, "Arts::Synth_PITCH_SHIFT");

//  we use our own copy instead:
    return new ArtsKwaveMultiTrackFilter<Synth_PITCH_SHIFT_bugfixed,
                              Synth_PITCH_SHIFT_bugfixed_impl>(tracks);
}

//***************************************************************************
bool PitchShiftPlugin::paramsChanged()
{
    return ((m_speed != m_last_speed) || (m_frequency != m_last_freq));
}

//***************************************************************************
void PitchShiftPlugin::updateFilter(ArtsMultiTrackFilter *filter,
                                    bool force)
{
    if (!filter) return;

    if ((m_frequency != m_last_freq) || force)
	filter->setAttribute("frequency", m_frequency);

    if ((m_speed != m_last_speed) || force)
	filter->setAttribute("speed", m_speed);

    m_last_freq  = m_frequency;
    m_last_speed = m_speed;
}

//***************************************************************************
QString PitchShiftPlugin::actionName()
{
    return i18n("pitch shift");
}

//***************************************************************************
void PitchShiftPlugin::setValues(double speed, double frequency)
{
    m_speed     = speed;
    m_frequency = frequency;
}

//***************************************************************************
#include "PitchShiftPlugin.moc"
//***************************************************************************
//***************************************************************************
