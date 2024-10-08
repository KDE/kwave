/*************************************************************************
    NoisePlugin.cpp  -  overwrites the selected range of samples with noise
                             -------------------
    begin                : Wed Dec 12 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#include <new>

#include <errno.h>

#include <KLocalizedString> // for the i18n macro

#include "libkwave/Connect.h"
#include "libkwave/MultiTrackSource.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/PluginManager.h"
#include "libkwave/SampleSink.h"
#include "libkwave/SampleSource.h"
#include "libkwave/undo/UndoTransactionGuard.h"

#include "libgui/OverViewCache.h"

#include "NoiseDialog.h"
#include "NoiseGenerator.h"
#include "NoisePlugin.h"

KWAVE_PLUGIN(noise, NoisePlugin)

//***************************************************************************
Kwave::NoisePlugin::NoisePlugin(QObject *parent, const QVariantList &args)
    :Kwave::FilterPlugin(parent, args), m_level(1.0), m_last_level(0.0)
{
}

//***************************************************************************
Kwave::NoisePlugin::~NoisePlugin()
{
}

//***************************************************************************
int Kwave::NoisePlugin::interpreteParameters(QStringList &params)
{
    bool ok;
    QString param;

    // evaluate the parameter list
    if (params.count() != 2) return -EINVAL;

    param = params[0];
    m_level = param.toDouble(&ok);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    param = params[1];
    unsigned int mode = param.toUInt(&ok);
    Q_ASSERT(ok);
    if (!ok || (mode > 2)) return -EINVAL;

    // all parameters accepted
    return 0;
}

//***************************************************************************
Kwave::PluginSetupDialog *Kwave::NoisePlugin::createDialog(QWidget *parent)
{
    Q_UNUSED(parent)

    // initialize the overview cache
    Kwave::SignalManager &mgr = manager().signalManager();
    QVector<unsigned int> tracks;
    sample_index_t first, last;
    sample_index_t length = selection(&tracks, &first, &last, true);
    Kwave::OverViewCache *overview_cache =
        new(std::nothrow) Kwave::OverViewCache(mgr, first, length,
            tracks.isEmpty() ? nullptr : &tracks);
    Q_ASSERT(overview_cache);

    // create the setup dialog
    Kwave::NoiseDialog *dialog =
        new(std::nothrow) Kwave::NoiseDialog(parentWidget(), overview_cache);
    if (!dialog) {
        delete overview_cache;
        return nullptr;
    }

    // connect the signals for detecting value changes in pre-listen mode
    connect(dialog, SIGNAL(levelChanged(double)),
            this,   SLOT(setNoiseLevel(double)));

    return dialog;
}

//***************************************************************************
Kwave::SampleSource *Kwave::NoisePlugin::createFilter(unsigned int tracks)
{
    return new(std::nothrow)
        Kwave::MultiTrackSource<Kwave::NoiseGenerator, true>(tracks);
}

//***************************************************************************
bool Kwave::NoisePlugin::paramsChanged()
{
    return (!qFuzzyCompare(m_level, m_last_level));
}

//***************************************************************************
void Kwave::NoisePlugin::updateFilter(Kwave::SampleSource *filter,
                                      bool force)
{
    if (!filter) return;

    if (!qFuzzyCompare(m_level, m_last_level) || force)
        filter->setAttribute(SLOT(setNoiseLevel(QVariant)),
                             QVariant(m_level));

    m_last_level = m_level;
}

//***************************************************************************
QString Kwave::NoisePlugin::actionName()
{
    return i18n("Add Noise");
}

//***************************************************************************
void Kwave::NoisePlugin::setNoiseLevel(double level)
{
    m_level = level;
}

//***************************************************************************
#include "NoisePlugin.moc"
//***************************************************************************
//***************************************************************************

#include "moc_NoisePlugin.cpp"
