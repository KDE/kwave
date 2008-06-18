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
#include <klocale.h> // for the i18n macro

#include <QList>
#include <QStringList>

#include "libkwave/MultiTrackWriter.h"
#include "libkwave/SampleWriter.h"
#include "libgui/SelectTimeWidget.h" // for selection mode
#include "kwave/PluginManager.h"
#include "kwave/UndoTransactionGuard.h"

#include "ZeroPlugin.h"

KWAVE_PLUGIN(ZeroPlugin,"zero","Thomas Eschenbacher");

#define ZERO_COUNT 64*1024

//***************************************************************************
ZeroPlugin::ZeroPlugin(const PluginContext &context)
    :KwavePlugin(context), m_stop(false)
{
     i18n("zero");
}

//***************************************************************************
void ZeroPlugin::run(QStringList params)
{
    unsigned int first = 0;
    unsigned int last  = 0;

    UndoTransactionGuard undo_guard(*this, i18n("silence"));
    m_stop = false;

    MultiTrackWriter *writers = 0;

    /*
     * new mode: insert a range filled with silence:
     * -> usage: zero(<mode>, <range>)
     * currently the only allowed value for <mode> is "0" (milliseconds)
     * and <range> must be a number of milliseconds
     */
    if (params.count() == 2) {
	// get the current selection start
	selection(&first, &last, false);

	// mode for the time (like in selectrange plugin)
	bool ok = true;
	int mode = params[0].toInt(&ok);
	Q_ASSERT(ok);
	if (!ok) return;
	Q_ASSERT((mode == (int)SelectTimeWidget::byTime) ||
	         (mode == (int)SelectTimeWidget::bySamples) ||
	         (mode == (int)SelectTimeWidget::byPercents));
	switch (mode) {
	    case (int)SelectTimeWidget::byTime:
		break;
	    case (int)SelectTimeWidget::bySamples:
	    case (int)SelectTimeWidget::byPercents:
	    default:
		// for the moment we only support (and need) the
		// "by time" mode
		return;
	}

	// length of the range of zeroes to insert
	double ms = params[1].toDouble(&ok);
	Q_ASSERT(ok);
	if (!ok) return;

	// convert from ms to samples
	double rate = signalRate();
	unsigned int length = (unsigned int)rint(ms / 1E3 * rate);

	// get the list of affected tracks
	QList<unsigned int> tracks = manager().selectedTracks();

	// some sanity check
	Q_ASSERT(length);
	Q_ASSERT(tracks.count());
	if (!length || !tracks.count()) return; // nothing to do

	last  = first + length - 1;
	writers = new MultiTrackWriter(signalManager(),
	    tracks, Insert, first, last);
    } else {
	writers = new MultiTrackWriter(signalManager(), Overwrite);
    }

    Q_ASSERT(writers);
    if (!writers) return; // out-of-memory

    // break if aborted
    if (!writers->tracks()) return;

    first = (*writers)[0]->first();
    last  = (*writers)[0]->last();
    unsigned int count = writers->tracks();

    // get the buffer with zeroes for faster filling
    if (m_zeroes.size() != ZERO_COUNT) {
	m_zeroes.resize(ZERO_COUNT);
	m_zeroes.fill(0);
    }
    Q_ASSERT(m_zeroes.size() == ZERO_COUNT);

    // loop over the sample range
    while ((first <= last) && (!m_stop)) {
	unsigned int rest = last - first + 1;
	if (rest < m_zeroes.size()) m_zeroes.resize(rest);

	// loop over all writers
	unsigned int w;
	for (w=0; w < count; w++) {
	    *((*writers)[w]) << m_zeroes;
	}

	first += m_zeroes.size();
    }

    delete writers;
    close();
}

//***************************************************************************
int ZeroPlugin::stop()
{
    m_stop = true;
    return KwavePlugin::stop();
}

//***************************************************************************
#include "ZeroPlugin.moc"
//***************************************************************************
//***************************************************************************
