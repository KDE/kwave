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

#include <klocale.h> // for the i18n macro

#include "libkwave/ArtsNativeMultiTrackFilter.h"
#include "libkwave/ArtsMultiTrackSink.h"
#include "libkwave/ArtsMultiTrackSink.h"
#include "libkwave/MultiTrackWriter.h"

#include "kwave/PluginManager.h"
#include "kwave/UndoTransactionGuard.h"

#include "NoisePlugin.h"

KWAVE_PLUGIN(NoisePlugin,"noise","Thomas Eschenbacher");

//***************************************************************************
NoisePlugin::NoisePlugin(PluginContext &context)
    :KwavePlugin(context), m_stop(false)
{
}

//***************************************************************************
void NoisePlugin::run(QStringList)
{
    unsigned int first, last;
    MultiTrackWriter sink;

    Arts::Dispatcher *dispatcher = manager().artsDispatcher();
    dispatcher->lock();
    Q_ASSERT(dispatcher);
    if (!dispatcher) close();

    UndoTransactionGuard undo_guard(*this, i18n("noise"));
    m_stop = false;

    selection(&first, &last, true);
    manager().openMultiTrackWriter(sink, selectedTracks(), Overwrite,
	first, last);

    // lock the dispatcher exclusively
    dispatcher->lock();

    // create all objects
    unsigned int tracks = selectedTracks().count();
    ArtsNativeMultiTrackFilter noise(tracks, "Arts::Synth_NOISE");
    ArtsMultiTrackSink arts_sink(sink);

    // connect them
    noise.connectOutput(arts_sink, "sink", "outvalue");

    // start all
    noise.start();
    arts_sink.start();

    // transport the samples
    qDebug("NoisePlugin: filter started...");
    while (!m_stop && !(arts_sink.done())) {
	arts_sink.goOn();
    }
    qDebug("NoisePlugin: filter done.");

    // shutdown
    noise.stop();
    arts_sink.stop();

    dispatcher->unlock();

    close();
}

//***************************************************************************
int NoisePlugin::stop()
{
    m_stop = true;
    return KwavePlugin::stop();
}

//***************************************************************************
//***************************************************************************
