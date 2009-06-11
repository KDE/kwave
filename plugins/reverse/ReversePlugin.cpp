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
#include <klocale.h> // for the i18n macro

#include <QList>
#include <QStringList>

#include "libkwave/MultiTrackReader.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/PluginManager.h"
#include "libkwave/SampleWriter.h"
#include "libkwave/undo/UndoTransactionGuard.h"

#include "libgui/SelectTimeWidget.h" // for selection mode

#include "ReversePlugin.h"

KWAVE_PLUGIN(ReversePlugin,"reverse","2.1","Thomas Eschenbacher");

//***************************************************************************
ReversePlugin::ReversePlugin(const PluginContext &context)
    :Kwave::Plugin(context)
{
     i18n("reverse");
}

//***************************************************************************
ReversePlugin::~ReversePlugin()
{
}

//***************************************************************************
void ReversePlugin::reverse(Kwave::SampleArray &buffer)
{
    unsigned int count = buffer.size() >> 1;
    if (count <= 1) return;

    sample_t *a = buffer.data();
    sample_t *b = buffer.data() + (buffer.size() - 1);
    while (count--) {
	register sample_t h = *a;
	*a++ = *b;
	*b-- = h;
    }
}

//***************************************************************************
void ReversePlugin::run(QStringList params)
{
    Q_UNUSED(params);

    UndoTransactionGuard undo_guard(*this, i18n("reverse"));

    // get the current selection
    unsigned int first = 0;
    unsigned int last  = 0;
    unsigned int length = selection(&first, &last, true);
    if (!length) return;

    // get the list of affected tracks
    QList<unsigned int> track_list = manager().selectedTracks();
    unsigned int tracks = track_list.count();
    if (!tracks) return;

    MultiTrackReader source_a(Kwave::SinglePassForward,
	signalManager(), track_list, first, last);
    MultiTrackReader source_b(Kwave::SinglePassReverse,
	signalManager(), track_list, first, last);

    // break if aborted
    if (!source_a.tracks() || !source_b.tracks()) return;

    // connect the progress dialog
    connect(&source_a, SIGNAL(progress(unsigned int)),
	    this,      SLOT(updateProgress(unsigned int)),
	    Qt::BlockingQueuedConnection);

    // get the buffers for exchanging the data
    const unsigned int block_size = source_a.blockSize();
    Kwave::SampleArray buffer_a(block_size);
    Kwave::SampleArray buffer_b(block_size);
    Q_ASSERT(buffer_a.size() == block_size);
    Q_ASSERT(buffer_b.size() == block_size);

    // loop over the sample range
    while ((first < last) && !shouldStop()) {
	unsigned int start_a = first;
	unsigned int start_b = last - block_size;

	// loop over all tracks
	for (unsigned int track = 0; track < tracks; track++) {
	    SampleReader *src_a = source_a[track];
	    SampleReader *src_b = source_b[track];

	    if (start_a + block_size < start_b) {
		*src_a >> buffer_a; // read from start
		src_b->seek(start_b);
		*src_b >> buffer_b; // read from end

		// swap the contents
		reverse(buffer_a);
		reverse(buffer_b);

		// write back buffer from the end at the start
		SampleWriter *dst_a = manager().openSampleWriter(
		    track, Overwrite,
		    start_a, start_a + block_size - 1);
		Q_ASSERT(dst_a);
		*dst_a << buffer_b;
		dst_a->flush();
		delete dst_a;

		// write back buffer from the start at the end
		SampleWriter *dst_b = manager().openSampleWriter(
		    track, Overwrite,
		    start_b, start_b + block_size - 1);
		Q_ASSERT(dst_b);
		*dst_b << buffer_a << flush;
		delete dst_b;
	    } else {
		// single buffer with last block
		buffer_a.resize(last - first + 1);

		// read from start
		*src_a >> buffer_a;

		// swap content
		reverse(buffer_a);

		// write back
		SampleWriter *dst = manager().openSampleWriter(
		    track, Overwrite, first, last);
		(*dst) << buffer_a << flush;
		delete dst;
	    }
	}

	// next positions
	first += block_size;
	last  = (last > block_size) ? (last - block_size) : 0;
    }

    close();
}

//***************************************************************************
void ReversePlugin::updateProgress(unsigned int progress)
{
    Kwave::Plugin::updateProgress(progress + progress);
}

//***************************************************************************
#include "ReversePlugin.moc"
//***************************************************************************
//***************************************************************************
