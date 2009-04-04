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

#include "libkwave/MultiTrackWriter.h"
#include "libkwave/KwaveConnect.h"
#include "libkwave/KwaveSampleSource.h"
#include "libkwave/KwaveSampleSink.h"
#include "libkwave/KwaveMultiTrackSource.h"
#include "libkwave/PluginManager.h"
#include "libkwave/undo/UndoTransactionGuard.h"

#include "NoisePlugin.h"
#include "NoiseGenerator.h"

KWAVE_PLUGIN(NoisePlugin,"noise","Thomas Eschenbacher");

//***************************************************************************
NoisePlugin::NoisePlugin(const PluginContext &context)
    :Kwave::Plugin(context), m_stop(false)
{
}

//***************************************************************************
void NoisePlugin::run(QStringList)
{
    unsigned int first, last;

    UndoTransactionGuard undo_guard(*this, i18n("noise"));

    m_stop = false;
    selection(&first, &last, true);

    // create all objects
    unsigned int tracks = selectedTracks().count();
    Kwave::MultiTrackSource<NoiseGenerator, true> source(tracks);
    MultiTrackWriter sink(signalManager(), selectedTracks(), Overwrite,
        first, last);

    // connect them
    if (!Kwave::connect(source, SIGNAL(output(Kwave::SampleArray)),
                        sink,   SLOT(input(Kwave::SampleArray))))
    {
	close();
	return;
    }

    // transport the samples
    qDebug("NoisePlugin: filter started [%u ... %u] ...", first, last);
    while (!m_stop && !(sink.done())) {
	source.goOn();
    }
    qDebug("NoisePlugin: filter done.");

    close();
}

//***************************************************************************
int NoisePlugin::stop()
{
    m_stop = true;
    return Kwave::Plugin::stop();
}

//***************************************************************************
#include "NoisePlugin.moc"
//***************************************************************************
//***************************************************************************
