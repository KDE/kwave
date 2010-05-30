/***************************************************************************
    NormalizePlugin.cpp  -  plugin for level normalizing
                             -------------------
    begin                : Fri May 01 2009
    copyright            : (C) 2009 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de

    original algorithms  : (C) 1999-2005 Chris Vaill <chrisvaill at gmail>
                           taken from "normalize-0.7.7"
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
#include <sched.h>

#include <QList>
#include <QStringList>
#include <QVector>

#include <klocale.h> // for the i18n macro
#include <threadweaver/Job.h>
#include <threadweaver/ThreadWeaver.h>
#include <threadweaver/DebuggingAids.h>

#include "libkwave/FileInfo.h"
#include "libkwave/KwaveConnect.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/PluginManager.h"
#include "libkwave/SignalManager.h"
#include "libkwave/Writer.h"
#include "libkwave/undo/UndoTransactionGuard.h"

#include "NormalizePlugin.h"
#include "Normalizer.h"

/** use a 100-element (1 second) window [units of 10ms] */
#define SMOOTHLEN 100

/** target volume level [dB] */
#define TARGET_LEVEL -12

KWAVE_PLUGIN(NormalizePlugin, "normalize", "2.1",
             I18N_NOOP("Normalizer"), "Thomas Eschenbacher");

//***************************************************************************
class GetMaxPowerJob: public ThreadWeaver::Job
{
public:
    typedef struct {
	QVector<double> fifo; /**< FIFO for power values */
	unsigned int    wp;   /**< FIFO write pointer */
	unsigned int    n;    /**< number of elements in the FIFO */
	double          sum;  /**< sum of queued power values */
	double          max;  /**< maximum power value */
    } average_t;

    /**
     * Constructor
     * @param reader reference to a SampleReader to read from
     * @param average reference to smoothing information
     * @param window_size length of the sliding window for volume detection
     */
    GetMaxPowerJob(SampleReader &reader, average_t &average,
                   unsigned int window_size);

    /** Destructor */
    virtual ~GetMaxPowerJob();

    /**
	* overloaded 'run' function that runns goOn() in the context
	* of the worker thread.
	*/
    virtual void run();

private:

    /** reference to the SampleReader */
    SampleReader &m_reader;

    /** reference to the smoothing information */
    average_t &m_average;

    /** size of a processing window */
    unsigned int m_window_size;

};

//***************************************************************************
GetMaxPowerJob::GetMaxPowerJob(SampleReader &reader, average_t &average,
                               unsigned int window_size)
    :ThreadWeaver::Job(), m_reader(reader), m_average(average),
     m_window_size(window_size)
{
}

//***************************************************************************
GetMaxPowerJob::~GetMaxPowerJob()
{
    int i = 0;
    while (!isFinished()) {
	qDebug("job %p waiting... #%u", static_cast<void *>(this), i++);
	sched_yield();
    }
    Q_ASSERT(isFinished());
}

//***************************************************************************
void GetMaxPowerJob::run()
{
    Kwave::SampleArray data(m_window_size);
    unsigned int round = 0;
    unsigned int loops = 5 * m_reader.blockSize() / m_window_size;
    loops++;

    while ((round++ < loops) && !m_reader.eof()) {
	unsigned int len = m_reader.read(data, 0, m_window_size);

	// calculate power of one block
	double sum = 0;
	for (unsigned int i = 0; i < len; i++) {
	    sample_t s = data[i];
	    double d = sample2double(s);
	    sum += d * d;
	}
	double pow = sum / static_cast<double>(len);

	// collect all power values in a FIFO
	average_t &avg = m_average;
	unsigned int wp = avg.wp;
	avg.sum -= avg.fifo[wp];
	avg.sum += pow;
	avg.fifo[wp] = pow;
	if (++wp >= SMOOTHLEN) wp = 0;
	avg.wp = wp;
	if (avg.n == SMOOTHLEN) {
	    // detect power peak
	    double p = avg.sum / static_cast<double>(SMOOTHLEN);
	    if (p > avg.max) avg.max = p;
	} else {
	    avg.n++;
	}
    }
//     qDebug("%p -> pos=%u, max=%g", this, m_reader.pos(), m_average.max);
}

//***************************************************************************
//***************************************************************************

NormalizePlugin::NormalizePlugin(const PluginContext &context)
    :Kwave::Plugin(context)
{
}

//***************************************************************************
NormalizePlugin::~NormalizePlugin()
{
}

//***************************************************************************
void NormalizePlugin::run(QStringList params)
{
    Q_UNUSED(params);
    UndoTransactionGuard undo_guard(*this, i18n("Normalize"));

    // get the current selection
    QList<unsigned int> tracks;
    sample_index_t first = 0;
    sample_index_t last  = 0;
    sample_index_t length = selection(&tracks, &first, &last, true);
    if (!length || tracks.isEmpty()) return;

    // get the list of affected tracks
    MultiTrackReader source(Kwave::SinglePassForward,
	signalManager(), tracks, first, last);

    // connect the progress dialog
    connect(&source, SIGNAL(progress(qreal)),
	    this,  SLOT(updateProgress(qreal)),
	     Qt::BlockingQueuedConnection);

    // detect the peak value
    emit setProgressText(i18n("Analyzing volume level..."));
//     qDebug("NormalizePlugin: getting peak...");
    double level = getMaxPower(source);
//     qDebug("NormalizePlugin: level is %g", level);

    Kwave::MultiTrackWriter sink(signalManager(), tracks, Overwrite,
	first, last);
    Kwave::MultiTrackSource<Kwave::Normalizer, true> normalizer(
	tracks.count(), this);

    // break if aborted
    if (!sink.tracks()) return;

    // connect them
    bool ok = true;
    if (ok) ok = Kwave::connect(
	source,     SIGNAL(output(Kwave::SampleArray)),
	normalizer, SLOT(input(Kwave::SampleArray)));
    if (ok) ok = Kwave::connect(
	normalizer, SIGNAL(output(Kwave::SampleArray)),
	sink,       SLOT(input(Kwave::SampleArray)));
    if (!ok) {
	return;
    }

    double target = pow(10.0, (TARGET_LEVEL / 20.0));
    double gain = target / level;
    qDebug("NormalizePlugin: gain=%g", gain);

    source.reset();
    QString db;
    emit setProgressText(i18n("Normalizing (%1 dB) ...",
	db.sprintf("%+0.1f", 20 * log10(gain))));

    normalizer.setAttribute(SLOT(setGain(const QVariant)), QVariant(gain));
    while (!shouldStop() && !source.eof()) {
	source.goOn();
    }

    sink.flush();
}

//***************************************************************************
double NormalizePlugin::getMaxPower(MultiTrackReader &source)
{
    double maxpow = 0.0;
    const unsigned int tracks = source.tracks();
    const double rate = signalManager().metaData().fileInfo().rate();
    const unsigned int window_size = static_cast<unsigned int>(rate / 100);
    if (!window_size) return 0;

    // set up smoothing window buffer
    QVector<GetMaxPowerJob::average_t> average(tracks);
    for (unsigned int t = 0; t < tracks; t++) {
	average[t].fifo.resize(SMOOTHLEN);
	average[t].fifo.fill(0.0);
	average[t].wp  = 0;
	average[t].n   = 0;
	average[t].sum = 0.0;
	average[t].max = 0.0;
    }

    ThreadWeaver::Weaver weaver;
    QList<ThreadWeaver::Job *> joblist;

    while (!shouldStop() && !source.eof()) {

	weaver.suspend();
	for (unsigned int t = 0; t < tracks; t++) {
	    SampleReader *reader = source[t];
	    if (!reader) continue;
	    if (reader->eof()) continue;

	    GetMaxPowerJob *job = new GetMaxPowerJob(
		*reader, average[t], window_size
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
    }

    if (average[0].n < SMOOTHLEN) {
	// if file was too short, calculate power out of what we have
	for (unsigned int t = 0; t < tracks; t++) {
	    GetMaxPowerJob::average_t &avg = average[t];
	    double pow = avg.sum / static_cast<double>(avg.n);
	    if (pow > maxpow) maxpow = pow;
	}
    } else {
	// get maximum among all tracks
	for (unsigned int t = 0; t < tracks; t++) {
	    double p = average[t].max;
	    if (p > maxpow) maxpow = p;
	}
    }

    double level = sqrt(maxpow);
    return level;
}

//***************************************************************************
#include "NormalizePlugin.moc"
//***************************************************************************
//***************************************************************************
