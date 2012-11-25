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
#include <threadweaver/Job.h>
#include <threadweaver/ThreadWeaver.h>
#include <threadweaver/DebuggingAids.h>

#include <QList>
#include <QSharedPointer>
#include <QStringList>
#include <QThread>

#include "libkwave/MultiTrackReader.h"
#include "libkwave/PluginManager.h"
#include "libkwave/SignalManager.h"
#include "libkwave/Utils.h"
#include "libkwave/Writer.h"
#include "libkwave/undo/UndoTransactionGuard.h"

#include "libgui/SelectTimeWidget.h" // for selection mode

#include "ReversePlugin.h"
#include "UndoReverseAction.h"

KWAVE_PLUGIN(Kwave::ReversePlugin, "reverse", "2.1",
             I18N_NOOP("Reverse"), "Thomas Eschenbacher");

//***************************************************************************
namespace Kwave
{
    class ReverseJob: public ThreadWeaver::Job
    {
    public:

	/**
	 * Constructor
	 */
	ReverseJob(
	    Kwave::SignalManager &manager, unsigned int track,
	    sample_index_t first, sample_index_t last, unsigned int block_size,
	    Kwave::SampleReader *src_a, Kwave::SampleReader *src_b
	);

	/** Destructor */
	virtual ~ReverseJob();

	/**
	 * overloaded 'run' function that runns goOn() in the context
	 * of the worker thread.
	 */
	virtual void run();

    private:

	/** reverses the content of an array of samples */
	void reverse(Kwave::SampleArray &buffer);

    private:

	/** signal manager, for opening a sample writer */
	Kwave::SignalManager &m_manager;

	/** index of the track */
	unsigned int m_track;

	/** first sample (from start) */
	sample_index_t m_first;

	/** last sample (from end) */
	sample_index_t m_last;

	/** block size in samples */
	unsigned int m_block_size;

	/** reader for start of signal */
	Kwave::SampleReader *m_src_a;

	/** reader for end of signal */
	Kwave::SampleReader *m_src_b;

    };
}

//***************************************************************************
Kwave::ReverseJob::ReverseJob(
    Kwave::SignalManager &manager, unsigned int track,
    sample_index_t first, sample_index_t last, unsigned int block_size,
    Kwave::SampleReader *src_a, Kwave::SampleReader *src_b)
    :ThreadWeaver::Job(),
     m_manager(manager), m_track(track),
     m_first(first), m_last(last), m_block_size(block_size),
     m_src_a(src_a), m_src_b(src_b)
{
}

//***************************************************************************
Kwave::ReverseJob::~ReverseJob()
{
    int i = 0;
    while (!isFinished()) {
	qDebug("job %p waiting... #%u", static_cast<void *>(this), i++);
	Kwave::yield();
    }
    Q_ASSERT(isFinished());
}

//***************************************************************************
void Kwave::ReverseJob::reverse(Kwave::SampleArray &buffer)
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
void Kwave::ReverseJob::run()
{
    sample_index_t start_a = m_first;
    sample_index_t start_b = (m_last >= m_block_size) ?
	(m_last - m_block_size) : 0;

    if (start_a + m_block_size < start_b) {
	// read from start
	Kwave::SampleArray buffer_a;
	buffer_a.resize(m_block_size);
	*m_src_a >> buffer_a;

	// read from end
	Kwave::SampleArray buffer_b;
	buffer_b.resize(m_block_size);
	m_src_b->seek(start_b);
	*m_src_b >> buffer_b;

	// swap the contents
	reverse(buffer_a);
	reverse(buffer_b);

	// write back buffer from the end at the start
	Kwave::Writer *dst_a = m_manager.openWriter(
	    m_track, Kwave::Overwrite,
	    start_a, start_a + m_block_size - 1);
	Q_ASSERT(dst_a);
	if (!dst_a) return;
	*dst_a << buffer_b;
	dst_a->flush();
	delete dst_a;

	// write back buffer from the start at the end
	Kwave::Writer *dst_b = m_manager.openWriter(
	    m_track, Kwave::Overwrite,
	    start_b, start_b + m_block_size - 1);
	Q_ASSERT(dst_b);
	if (!dst_b) return;
	*dst_b << buffer_a << flush;
	delete dst_b;
    } else {
	// single buffer with last block
	Kwave::SampleArray buffer;
	buffer.resize(m_last - m_first + 1);

	// read from start
	*m_src_a >> buffer;

	// swap content
	reverse(buffer);

	// write back
	Kwave::Writer *dst = m_manager.openWriter(
	    m_track, Kwave::Overwrite, m_first, m_last);
	if (!dst) return;
	(*dst) << buffer << flush;
	delete dst;
    }

}

//***************************************************************************
//***************************************************************************
Kwave::ReversePlugin::ReversePlugin(const Kwave::PluginContext &context)
    :Kwave::Plugin(context)
{
}

//***************************************************************************
Kwave::ReversePlugin::~ReversePlugin()
{
}

//***************************************************************************
void Kwave::ReversePlugin::run(QStringList params)
{
    QSharedPointer<Kwave::UndoTransactionGuard> undo_guard;

    // get the current selection and the list of affected tracks
    QList<unsigned int> tracks;
    sample_index_t first = 0;
    sample_index_t last  = 0;
    sample_index_t length = selection(&tracks, &first, &last, true);
    if (!length || tracks.isEmpty())
	return;

    if ((params.count() != 1) || (params.first() != "noundo")) {
	// undo is enabled, create a undo guard
	undo_guard = QSharedPointer<Kwave::UndoTransactionGuard>(
	    new Kwave::UndoTransactionGuard(*this, i18n("Reverse")));
	if (!undo_guard) return;

	// try to save undo information
	Kwave::UndoAction *undo = new Kwave::UndoReverseAction(manager());
	if (!undo_guard->registerUndoAction(undo))
	    return;
	undo->store(signalManager());
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

    // get the buffers for exchanging the data
    const unsigned int block_size = 5 * source_a.blockSize();
    Kwave::SampleArray buffer_a(block_size);
    Kwave::SampleArray buffer_b(block_size);
    Q_ASSERT(buffer_a.size() == block_size);
    Q_ASSERT(buffer_b.size() == block_size);

    ThreadWeaver::Weaver weaver;
    QList<ThreadWeaver::Job *> joblist;

    // loop over the sample range
    while ((first < last) && !shouldStop()) {

	weaver.suspend();

	// loop over all tracks
	for (int track = 0; track < tracks.count(); track++) {

	    Kwave::ReverseJob *job = new Kwave::ReverseJob(
		signalManager(), tracks[track], first, last, block_size,
		source_a[track], source_b[track]
	    );

	    if (!job) continue;
	    joblist.append(job);

	    // put the job into the thread weaver
	    weaver.enqueue(job);
	}
	weaver.resume();

	if (!joblist.isEmpty()) {
	    weaver.finish();
	    Q_ASSERT(weaver.isEmpty());
	    qDeleteAll(joblist);
	    joblist.clear();
	}

	// next positions
	first += block_size;
	last  = (last > block_size) ? (last - block_size) : 0;
    }
}

//***************************************************************************
void Kwave::ReversePlugin::updateProgress(qreal progress)
{
    Kwave::Plugin::updateProgress(progress + progress);
}

//***************************************************************************
#include "ReversePlugin.moc"
//***************************************************************************
//***************************************************************************
