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

#include <klocale.h> // for the i18n macro

#include "libkwave/MultiTrackSource.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/Connect.h"
#include "libkwave/SampleSource.h"
#include "libkwave/SampleSink.h"
#include "libkwave/PluginManager.h"
#include "libkwave/undo/UndoTransactionGuard.h"

#include "NoisePlugin.h"
#include "NoiseGenerator.h"

KWAVE_PLUGIN(Kwave::NoisePlugin, "noise", "2.1",
             I18N_NOOP("Noise Generator"), "Thomas Eschenbacher");

//***************************************************************************
Kwave::NoisePlugin::NoisePlugin(const Kwave::PluginContext &context)
    :Kwave::Plugin(context)
{
}

//***************************************************************************
void Kwave::NoisePlugin::run(QStringList)
{
    sample_index_t first, last;
    QList<unsigned int> tracks;

    Kwave::UndoTransactionGuard undo_guard(*this, i18n("Noise"));

    selection(&tracks, &first, &last, true);

    // create all objects
    Kwave::MultiTrackSource<Kwave::NoiseGenerator, true> source(tracks.count());
    Kwave::MultiTrackWriter sink(signalManager(), tracks, Kwave::Overwrite,
        first, last);

    // break if aborted
    if (!sink.tracks()) return;

    // connect the progress dialog
    connect(&sink, SIGNAL(progress(qreal)),
	    this,  SLOT(updateProgress(qreal)),
	     Qt::BlockingQueuedConnection);

    // connect them
    if (!Kwave::connect(source, SIGNAL(output(Kwave::SampleArray)),
                        sink,   SLOT(input(Kwave::SampleArray))))
    {
	return;
    }

    // transport the samples
    qDebug("NoisePlugin: filter started [%lu ... %lu] ...",
	   static_cast<unsigned long int>(first),
	   static_cast<unsigned long int>(last));
    while (!shouldStop() && !sink.done()) {
	source.goOn();
    }
    qDebug("NoisePlugin: filter done.");
}

//***************************************************************************
#include "NoisePlugin.moc"
//***************************************************************************
//***************************************************************************
