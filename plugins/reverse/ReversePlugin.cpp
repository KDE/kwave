/*************************************************************************
      ReversePlugin.cpp  -  reverses the current selection
                             -------------------
    begin                : Tue Jun 09 2009
    copyright            : (C) 2009 by Thomas Eschenbacher
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

#include <QFutureSynchronizer>
#include <QList>
#include <QSharedPointer>
#include <QStringList>
#include <QThread>
#include <QtConcurrentRun>

#include "libkwave/MultiTrackReader.h"
#include "libkwave/PluginManager.h"
#include "libkwave/SignalManager.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"
#include "libkwave/Writer.h"
#include "libkwave/undo/UndoTransactionGuard.h"

#include "libgui/SelectTimeWidget.h" // for selection mode

#include "ReversePlugin.h"
#include "UndoReverseAction.h"

KWAVE_PLUGIN(Kwave::ReversePlugin, "reverse", "2.3",
             I18N_NOOP("Reverse"), "Thomas Eschenbacher");

//***************************************************************************
//***************************************************************************
Kwave::ReversePlugin::ReversePlugin(Kwave::PluginManager &plugin_manager)
    :Kwave::Plugin(plugin_manager)
{
}

//***************************************************************************
Kwave::ReversePlugin::~ReversePlugin()
{
}

//***************************************************************************
void Kwave::ReversePlugin::run(QStringList params)
{
    Kwave::SignalManager &signal_manager = signalManager();

    QSharedPointer<Kwave::UndoTransactionGuard> undo_guard;

    // get the current selection and the list of affected tracks
    QList<unsigned int> tracks;
    sample_index_t first = 0;
    sample_index_t last  = 0;
    sample_index_t length = selection(&tracks, &first, &last, true);
    if (!length || tracks.isEmpty())
	return;

    if ((params.count() != 1) || (params.first() != _("noundo"))) {
	// undo is enabled, create a undo guard
	undo_guard = QSharedPointer<Kwave::UndoTransactionGuard>(
	    new(std::nothrow) Kwave::UndoTransactionGuard(
	        *this, i18n("Reverse")));
	if (!undo_guard) return;

	// try to save undo information
	Kwave::UndoAction *undo =
	    new(std::nothrow) Kwave::UndoReverseAction(manager());
	if (!undo_guard->registerUndoAction(undo))
	    return;
	undo->store(signal_manager);
    }

    Kwave::MultiTrackReader source_a(Kwave::SinglePassForward,
	signalManager(), tracks, first, last);
    Kwave::MultiTrackReader source_b(Kwave::SinglePassReverse,
	signalManager(), tracks, first, last);

    // break if aborted
    if (!source_a.tracks() || !source_b.tracks())
	return;

    // connect the progress dialog
    connect(&source_a, SIGNAL(progress(qreal)),
	    this,      SLOT(updateProgress(qreal)),
	    Qt::BlockingQueuedConnection);

    // use a reasonably big buffer size
    const unsigned int block_size = 5 * source_a.blockSize();

    // loop over the sample range
    while ((first < last) && !shouldStop()) {
	QFutureSynchronizer<void> synchronizer;

	Kwave::ReversePlugin::SliceParams slice_params;
	slice_params.m_first      = first;
	slice_params.m_last       = last;
	slice_params.m_block_size = block_size;

	// loop over all tracks
	for (int i = 0; i < tracks.count(); i++) {
	    synchronizer.addFuture(QtConcurrent::run(
		this,
		&Kwave::ReversePlugin::reverseSlice,
		tracks[i], source_a[i], source_b[i],
		slice_params)
	    );
	}

	// next positions
	first += block_size;
	last  = (last > block_size) ? (last - block_size) : 0;

	synchronizer.waitForFinished();
    }

}

//***************************************************************************
void Kwave::ReversePlugin::reverseSlice(unsigned int track,
    Kwave::SampleReader *src_a, Kwave::SampleReader *src_b,
    const Kwave::ReversePlugin::SliceParams &params)
{
    Kwave::SignalManager &signal_manager = signalManager();
    const sample_index_t first      = params.m_first;
    const sample_index_t last       = params.m_last;
    const unsigned int   block_size = params.m_block_size;
    const sample_index_t start_a    = first;
    const sample_index_t start_b    = (last >= block_size) ?
                                      (last - block_size) : 0;
    bool ok = true;

    if (start_a + block_size < start_b) {
	// read from start
	Kwave::SampleArray buffer_a;
	ok &= buffer_a.resize(block_size);
	Q_ASSERT(ok);
	*src_a >> buffer_a;

	// read from end
	Kwave::SampleArray buffer_b;
	ok &= buffer_b.resize(block_size);
	Q_ASSERT(ok);
	src_b->seek(start_b);
	*src_b >> buffer_b;

	// swap the contents
	reverse(buffer_a);
	reverse(buffer_b);

	// write back buffer from the end at the start
	Kwave::Writer *dst_a = signal_manager.openWriter(
	    Kwave::Overwrite, track,
	    start_a, start_a + block_size - 1);
	Q_ASSERT(dst_a);
	if (!dst_a) return;
	*dst_a << buffer_b;
	dst_a->flush();
	delete dst_a;

	// write back buffer from the start at the end
	Kwave::Writer *dst_b = signal_manager.openWriter(
	    Kwave::Overwrite, track,
	    start_b, start_b + block_size - 1);
	Q_ASSERT(dst_b);
	if (!dst_b) return;
	*dst_b << buffer_a << flush;
	delete dst_b;
    } else {
	// single buffer with last block
	Kwave::SampleArray buffer;
	ok &= buffer.resize(Kwave::toUint(last - first + 1));
	Q_ASSERT(ok);

	// read from start
	*src_a >> buffer;

	// swap content
	reverse(buffer);

	// write back
	Kwave::Writer *dst = signal_manager.openWriter(
	    Kwave::Overwrite, track, first, last);
	if (!dst) return;
	(*dst) << buffer << flush;
	delete dst;
    }
}

//***************************************************************************
void Kwave::ReversePlugin::reverse(Kwave::SampleArray &buffer)
{
    unsigned int count = buffer.size() >> 1;
    if (count <= 1) return;

    sample_t *a = buffer.data();
    sample_t *b = buffer.data() + (buffer.size() - 1);
    for (; count; count--) {
	register sample_t h = *a;
	*a++ = *b;
	*b-- = h;
    }
}

//***************************************************************************
void Kwave::ReversePlugin::updateProgress(qreal progress)
{
    Kwave::Plugin::updateProgress(progress + progress);
}

//***************************************************************************
//***************************************************************************
//***************************************************************************
