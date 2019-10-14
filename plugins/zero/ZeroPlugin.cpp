/***************************************************************************
         ZeroPlugin.cpp  -  wipes out the selected range of samples to zero
                             -------------------
    begin                : Fri Jun 01 2001
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

#include <math.h>
#include <new>

#include <KLocalizedString> // for the i18n macro

#include <QList>
#include <QStringList>

#include "libkwave/MultiTrackWriter.h"
#include "libkwave/PluginManager.h"
#include "libkwave/Utils.h"
#include "libkwave/Writer.h"
#include "libkwave/undo/UndoTransactionGuard.h"

#include "libgui/SelectTimeWidget.h" // for selection mode

#include "ZeroPlugin.h"

KWAVE_PLUGIN(zero, ZeroPlugin)

#define ZERO_COUNT (64 * 1024)

//***************************************************************************
Kwave::ZeroPlugin::ZeroPlugin(QObject *parent, const QVariantList &args)
    :Kwave::Plugin(parent, args), m_zeroes()
{
}

//***************************************************************************
Kwave::ZeroPlugin::~ZeroPlugin()
{
}

//***************************************************************************
void Kwave::ZeroPlugin::run(QStringList params)
{
    QVector<unsigned int> tracks;
    sample_index_t first = 0;
    sample_index_t last  = 0;
    bool succeeded = true;

    Kwave::UndoTransactionGuard undo_guard(*this, i18n("Silence"));

    Kwave::MultiTrackWriter *writers = Q_NULLPTR;

    /*
     * new mode: insert a range filled with silence:
     * -> usage: zero(<mode>, <range>)
     */
    if (params.count() == 2) {
	// get the current selection
	selection(&tracks, &first, &last, false);

	// mode for the time (like in selectrange plugin)
	bool ok = true;
	int mode = params[0].toInt(&ok);
	Q_ASSERT(ok);
	if (!ok) return;
	Q_ASSERT(
	    (mode == static_cast<int>(Kwave::SelectTimeWidget::byTime)) ||
	    (mode == static_cast<int>(Kwave::SelectTimeWidget::bySamples)) ||
	    (mode == static_cast<int>(Kwave::SelectTimeWidget::byPercents))
	);
	if ((mode != static_cast<int>(Kwave::SelectTimeWidget::byTime)) &&
	    (mode != static_cast<int>(Kwave::SelectTimeWidget::bySamples)) &&
	    (mode != static_cast<int>(Kwave::SelectTimeWidget::byPercents)))
	{
	    return;
	}

	// length of the range of zeroes to insert
	unsigned int time = params[1].toUInt(&ok);
	Q_ASSERT(ok);
	if (!ok) return;

	// convert from time to samples
	sample_index_t length = Kwave::SelectTimeWidget::timeToSamples(
	    static_cast<Kwave::SelectTimeWidget::Mode>(mode),
	    time, signalRate(), signalLength());

	// some sanity check
	Q_ASSERT(length);
	Q_ASSERT(!tracks.isEmpty());
	if (!length || tracks.isEmpty()) return; // nothing to do

	last  = first + length - 1;
	writers = new(std::nothrow) Kwave::MultiTrackWriter(signalManager(),
	    tracks, Kwave::Insert, first, last);
    } else {
	writers = new(std::nothrow) Kwave::MultiTrackWriter(signalManager(),
	    Kwave::Overwrite);
    }

    if (!writers) return; // out-of-memory

    // break if aborted
    if (!writers->tracks()) return;

    // connect the progress dialog
    connect(writers, SIGNAL(progress(qreal)),
	    this,    SLOT(updateProgress(qreal)),
	    Qt::BlockingQueuedConnection);

    first = (*writers)[0]->first();
    last  = (*writers)[0]->last();
    unsigned int count = writers->tracks();

    // get the buffer with zeroes for faster filling
    if (m_zeroes.size() != ZERO_COUNT) {
	succeeded &= m_zeroes.resize(ZERO_COUNT);
	Q_ASSERT(succeeded);
	m_zeroes.fill(0);
    }
    Q_ASSERT(m_zeroes.size() == ZERO_COUNT);

    // loop over the sample range
    while ((first <= last) && !shouldStop() && succeeded) {
	sample_index_t rest = last - first + 1;
	if (rest < m_zeroes.size()) {
	    succeeded &= m_zeroes.resize(Kwave::toUint(rest));
	    Q_ASSERT(succeeded);
	    if (!succeeded) break;
	}

	// loop over all writers
	unsigned int w;
	for (w = 0; w < count; w++) {
	    *((*writers)[w]) << m_zeroes;
	}

	first += m_zeroes.size();
    }

    delete writers;
}

//***************************************************************************
#include "ZeroPlugin.moc"
//***************************************************************************
//***************************************************************************
