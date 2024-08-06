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
#include <new>

#include <QFutureSynchronizer>
#include <QList>
#include <QStringList>
#include <QThread>
#include <QtConcurrentRun>

#include <KLocalizedString> // for the i18n macro

#include "libkwave/Connect.h"
#include "libkwave/FileInfo.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/PluginManager.h"
#include "libkwave/SignalManager.h"
#include "libkwave/Utils.h"
#include "libkwave/Writer.h"
#include "libkwave/undo/UndoTransactionGuard.h"

#include "NormalizePlugin.h"
#include "Normalizer.h"

/** use a 100-element (1 second) window [units of 10ms] */
#define SMOOTHLEN 100

/** target volume level [dB] */
#define TARGET_LEVEL -12

KWAVE_PLUGIN(normalize, NormalizePlugin)

//***************************************************************************
Kwave::NormalizePlugin::NormalizePlugin(QObject *parent,
                                        const QVariantList &args)
    :Kwave::Plugin(parent, args)
{
}

//***************************************************************************
Kwave::NormalizePlugin::~NormalizePlugin()
{
}

//***************************************************************************
void Kwave::NormalizePlugin::run(QStringList params)
{
    Q_UNUSED(params)
    Kwave::UndoTransactionGuard undo_guard(*this, i18n("Normalize"));

    // get the current selection
    QVector<unsigned int> tracks;
    sample_index_t first = 0;
    sample_index_t last  = 0;
    sample_index_t length = selection(&tracks, &first, &last, true);
    if (!length || tracks.isEmpty()) return;

    // get the current volume level
    double level = 0.0;
    {
        Kwave::MultiTrackReader src(Kwave::SinglePassForward,
            signalManager(), tracks, first, last);

        // connect the progress dialog
        connect(&src, SIGNAL(progress(qreal)),
                this,  SLOT(updateProgress(qreal)),
                Qt::BlockingQueuedConnection);

        // detect the peak value
        emit setProgressText(i18n("Analyzing volume level..."));
//         qDebug("NormalizePlugin: getting peak...");
        level = getMaxPower(src);
//         qDebug("NormalizePlugin: level is %g", level);
    }

    Kwave::MultiTrackReader source(Kwave::SinglePassForward,
        signalManager(), tracks, first, last);
    Kwave::MultiTrackWriter sink(signalManager(), tracks, Kwave::Overwrite,
        first, last);
    Kwave::MultiTrackSource<Kwave::Normalizer, true> normalizer(
        tracks.count(), this);

    // break if aborted
    if (!sink.tracks()) return;

    // connect the progress dialog
    connect(&source, SIGNAL(progress(qreal)),
            this,  SLOT(updateProgress(qreal)),
            Qt::BlockingQueuedConnection);

    // connect them
    bool ok = Kwave::connect(
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

    QString db;
    emit setProgressText(i18n("Normalizing (%1 dB) ...",
        db.asprintf("%+0.1f", 20 * log10(gain))));

    normalizer.setAttribute(SLOT(setGain(QVariant)), QVariant(gain));
    while (!shouldStop() && !source.eof()) {
        source.goOn();
    }

    sink.flush();
}

//***************************************************************************
double Kwave::NormalizePlugin::getMaxPower(Kwave::MultiTrackReader &source)
{
    double maxpow = 0.0;
    const unsigned int tracks = source.tracks();
    const double rate = Kwave::FileInfo(signalManager().metaData()).rate();
    const unsigned int window_size = Kwave::toUint(rate / 100);
    if (!window_size) return 0;

    // set up smoothing window buffer
    QVector<Kwave::NormalizePlugin::Average> average(tracks);
    for (unsigned int t = 0; t < tracks; t++) {
        average[t].fifo = QVector<double>(SMOOTHLEN, double(0.0));
        average[t].wp  = 0;
        average[t].n   = 0;
        average[t].sum = 0.0;
        average[t].max = 0.0;
    }

    while (!shouldStop() && !source.eof()) {
        QFutureSynchronizer<void> synchronizer;

        for (unsigned int t = 0; t < tracks; t++) {
            Kwave::SampleReader *reader = source[t];
            if (!reader) continue;
            if (reader->eof()) continue;

            synchronizer.addFuture(QtConcurrent::run(
                &Kwave::NormalizePlugin::getMaxPowerOfTrack,
                this,
                reader, &(average[t]), window_size
            ));
        }
        synchronizer.waitForFinished();
     }

    if (average[0].n < SMOOTHLEN) {
        // if file was too short, calculate power out of what we have
        for (unsigned int t = 0; t < tracks; t++) {
            Kwave::NormalizePlugin::Average &avg = average[t];
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
void Kwave::NormalizePlugin::getMaxPowerOfTrack(
    Kwave::SampleReader *reader,
    Kwave::NormalizePlugin::Average *p_average,
    unsigned int window_size)
{
    Kwave::NormalizePlugin::Average &average = *p_average;
    Kwave::SampleArray data(window_size);
    unsigned int round = 0;
    unsigned int loops = 5 * reader->blockSize() / window_size;
    loops++;

    while ((round++ < loops) && !reader->eof()) {
        unsigned int len = reader->read(data, 0, window_size);

        // calculate power of one block
        double sum = 0;
        const Kwave::SampleArray &in = data;
        for (unsigned int i = 0; i < len; i++) {
            sample_t s = in[i];
            double d   = sample2double(s);
            sum += (d * d);
        }
        double pow = sum / static_cast<double>(len);

        // collect all power values in a FIFO
        unsigned int wp = average.wp;
        average.sum -= average.fifo[wp];
        average.sum += pow;
        average.fifo[wp] = pow;
        if (++wp >= SMOOTHLEN) wp = 0;
        average.wp = wp;
        if (average.n == SMOOTHLEN) {
            // detect power peak
            double p = average.sum / static_cast<double>(SMOOTHLEN);
            if (p > average.max) average.max = p;
        } else {
            average.n++;
        }
    }
//     qDebug("%p -> pos=%llu, max=%g", this, reader->pos(), average.max);
}

//***************************************************************************
#include "NormalizePlugin.moc"
//***************************************************************************
//***************************************************************************

#include "moc_NormalizePlugin.cpp"
