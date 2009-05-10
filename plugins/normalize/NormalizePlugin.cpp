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
#include <klocale.h> // for the i18n macro

#include <QList>
#include <QStringList>

#include "libkwave/FileInfo.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/PluginManager.h"
#include "libkwave/SampleWriter.h"
#include "libkwave/undo/UndoTransactionGuard.h"

#include "libgui/SelectTimeWidget.h" // for selection mode

#include "NormalizePlugin.h"

/** use a 100-element (1 second) window [units of 10ms] */
#define SMOOTHLEN 100

KWAVE_PLUGIN(NormalizePlugin,"normalize","Thomas Eschenbacher");

//***************************************************************************
NormalizePlugin::NormalizePlugin(const PluginContext &context)
    :Kwave::Plugin(context)
{
     i18n("normalize");
}

//***************************************************************************
NormalizePlugin::~NormalizePlugin()
{
}

//***************************************************************************
/*
 * Limiter function:
 *
 *        / tanh((x + lev) / (1-lev)) * (1-lev) - lev        (for x < -lev)
 *        |
 *   x' = | x                                                (for |x| <= lev)
 *        |
 *        \ tanh((x - lev) / (1-lev)) * (1-lev) + lev        (for x > lev)
 *
 * With limiter level = 0, this is equivalent to a tanh() function;
 * with limiter level = 1, this is equivalent to clipping.
 */
static inline double limiter(double x)
{
    double xp;
    const double lmtr_lvl = 0.5;

    if (x < -lmtr_lvl)
	xp = tanh((x + lmtr_lvl) / (1-lmtr_lvl)) * (1-lmtr_lvl) - lmtr_lvl;
    else if (x <= lmtr_lvl)
	xp = x;
    else
	xp = tanh((x - lmtr_lvl) / (1-lmtr_lvl)) * (1-lmtr_lvl) + lmtr_lvl;

    return xp;
}

//***************************************************************************
void NormalizePlugin::run(QStringList params)
{
    Q_UNUSED(params);
    UndoTransactionGuard undo_guard(*this, i18n("normalize"));

    // get the current selection
    unsigned int first = 0;
    unsigned int last  = 0;
    unsigned int length = selection(&first, &last, true);
    if (!length) return;

    // get the list of affected tracks
    QList<unsigned int> track_list = manager().selectedTracks();
    MultiTrackReader source(signalManager(), track_list,
	first, last);

    // connect the progress dialog
    connect(&source, SIGNAL(progress(unsigned int)),
	    this,  SLOT(updateProgress(unsigned int)),
	    Qt::QueuedConnection);

    // detect the peak value
    emit setProgressText(i18n("analyzing volume level..."));
//     qDebug("NormalizePlugin: getting peak...");
    double level = getMaxPower(source);
//     qDebug("NormalizePlugin: level is %g", level);

    MultiTrackWriter sink(signalManager(),   track_list, Overwrite,
	first, last);
    const unsigned int tracks = track_list.count();

    // break if aborted
    if (!sink.tracks()) return;

    double target = 0.2511886431509580; /* -12dBFS */
    double gain = target / level;
    bool use_limiter = (gain > 1.0);
    qDebug("NormalizePlugin: target=%g, gain=%g", target, gain);

    Kwave::SampleArray data(source.blockSize());
    source.reset();
    QString db;
    emit setProgressText(i18n("normalizing (%1dB) ...",
	db.sprintf("%+0.1f", 20 * log10(gain)))
    );
    while (!shouldStop() && !source.eof()) {

	for (unsigned int t = 0; t < tracks; t++) {
	    SampleReader *reader = source[t];
	    SampleWriter *writer = sink[t];
	    if (!reader || !writer) continue;
	    unsigned int len = reader->read(data, 0, data.size());

	    for (unsigned int i = 0; i < len; i++) {
		double s = sample2double(data[i]);
		s *= gain;
		if (use_limiter) s = limiter(s);
		data[i] = double2sample(s);
	    }

	    (*writer) << data;
	}
    }
    sink.flush();

    close();
}

//***************************************************************************
double NormalizePlugin::getMaxPower(MultiTrackReader &source)
{
    double maxpow = 0.0;
    const unsigned int tracks = source.tracks();
    const double rate = fileInfo().rate();
    const unsigned int window_size = static_cast<unsigned int>(rate / 100);
    if (!window_size) return 0;
    Kwave::SampleArray data(window_size);

    // set up smoothing window buffer
    QVector<average_t> average(tracks);
    for (unsigned int t = 0; t < tracks; t++) {
	average[t].fifo.resize(SMOOTHLEN);
	average[t].fifo.fill(0.0);
	average[t].wp  = 0;
	average[t].n   = 0;
	average[t].sum = 0.0;
    }

    while (!shouldStop() && !source.eof()) {
	for (unsigned int t = 0; t < tracks; t++) {
	    SampleReader *reader = source[t];
	    if (!reader) continue;
	    unsigned int len = reader->read(data, 0, window_size);

	    // calculate power of one block
	    double sum = 0;
	    for (unsigned int i = 0; i < len; i++) {
		sample_t s = data[i];
		double d = sample2double(s);
		sum += d * d;
	    }
	    double pow = sum / static_cast<double>(len);

	    // collect all power values in a FIFO
	    average_t &avg = average[t];
	    unsigned int wp = avg.wp;
	    avg.sum -= avg.fifo[wp];
	    avg.sum += pow;
	    avg.fifo[wp] = pow;
	    if (++wp >= SMOOTHLEN) wp = 0;
	    avg.wp = wp;
	    if (avg.n == SMOOTHLEN) {
		// detect power peak
		double p = avg.sum / static_cast<double>(SMOOTHLEN);
		if (p > maxpow) maxpow = p;
	    } else {
		avg.n++;
	    }
	}
    }

    // if file was too short, calculate power out of what we have
    if (average[0].n < SMOOTHLEN) {
	for (unsigned int t = 0; t < tracks; t++) {
	    average_t &avg = average[t];
	    double pow = avg.sum / static_cast<double>(avg.n);
	    if (pow > maxpow) maxpow = pow;
	}
    }

    double level = sqrt(maxpow);
    return level;
}

//***************************************************************************
#include "NormalizePlugin.moc"
//***************************************************************************
//***************************************************************************
