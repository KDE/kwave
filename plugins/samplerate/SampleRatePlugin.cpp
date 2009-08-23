/***************************************************************************
   SampleRatePlugin.cpp  -  sample rate conversion
                             -------------------
    begin                : Tue Jul 07 2009
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
#include <errno.h>

#include <klocale.h> // for the i18n macro
#include <threadweaver/Job.h>
#include <threadweaver/ThreadWeaver.h>
#include <threadweaver/DebuggingAids.h>

#include <QList>
#include <QListIterator>
#include <QStringList>

#include "libkwave/FileInfo.h"
#include "libkwave/KwaveConnect.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/PluginManager.h"
#include "libkwave/SignalManager.h"
#include "libkwave/Writer.h"
#include "libkwave/undo/UndoTransactionGuard.h"

#include "RateConverter.h"
#include "SampleRatePlugin.h"

KWAVE_PLUGIN(SampleRatePlugin,"samplerate","2.1","Thomas Eschenbacher");

//***************************************************************************
SampleRatePlugin::SampleRatePlugin(const PluginContext &context)
    :Kwave::Plugin(context), m_params(), m_new_rate(0.0),
     m_whole_signal(false)
{
     i18n("change sample rate");
}

//***************************************************************************
SampleRatePlugin::~SampleRatePlugin()
{
}

//***************************************************************************
int SampleRatePlugin::interpreteParameters(QStringList &params)
{
    bool ok;
    QString param;

    // set defaults
    m_new_rate     = 44100.0;
    m_whole_signal = false;

    // evaluate the parameter list
    if (params.count() < 1) return -EINVAL;

    param = params[0];
    m_new_rate = param.toDouble(&ok);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    // check whether we should change the whole signal (optional)
    if (params.count() == 2) {
	if (params[1] != "all")
	    return -EINVAL;
	m_whole_signal = true;
    }

    // all parameters accepted
    m_params = params;

    return 0;
}

//***************************************************************************
void SampleRatePlugin::run(QStringList params)
{
    SignalManager &mgr = signalManager();

    // parse parameters
    if (interpreteParameters(params) < 0)
	return;

    double old_rate = fileInfo().rate();
    if ((old_rate <= 0) || (old_rate == m_new_rate)) return;

    UndoTransactionGuard undo_guard(*this, i18n("change sample rate"));

    // get the current selection and the list of affected tracks
    unsigned int first = 0;
    unsigned int last  = 0;
    unsigned int length;
    QList<unsigned int> tracks;
    if (m_whole_signal) {
	length = signalLength();
	last   = (length) ? (length - 1) : 0;
	tracks = mgr.allTracks();
    } else {
	length = selection(&tracks, &first, &last, true);
	if ((length == signalLength()) &&
	    (tracks.count() == static_cast<int>(mgr.tracks())))
	{
	    // manually selected the whole signal
	    m_whole_signal = true;
	}
    }
    qDebug("SampleRatePlugin: from %9u - %9u (%9u)", first, last, length);
    if (!length || tracks.isEmpty()) return;

    // calculate the new length
    double ratio = m_new_rate / old_rate;
    unsigned int new_length = length * ratio;
    if ((new_length == length) || !new_length) return;

    // if the new length is bigger than the current length,
    // insert some space at the end
    if (new_length > length) {
	qDebug("SampleRatePlugin: inserting %u at %u",
	       new_length - length + 1, last + 1);
	mgr.insertSpace(last + 1, new_length - length + 1, tracks);
    }

    MultiTrackReader source(Kwave::SinglePassForward,
	mgr, tracks, first, last);

    // connect the progress dialog
    connect(&source, SIGNAL(progress(unsigned int)),
	    this,  SLOT(updateProgress(unsigned int)),
	     Qt::BlockingQueuedConnection);
    emit setProgressText(
	i18n("changing sample rate from %1 kHz to %2 kHz ...",
	QString::number(old_rate   / 1E3),
	QString::number(m_new_rate / 1E3))
    );

    // create the converter
    Kwave::MultiTrackSource<Kwave::RateConverter, true> converter(
	tracks.count(), this);
    converter.setAttribute(SLOT(setRatio(const QVariant)), QVariant(ratio));

    // create the writer with the appropriate length
    MultiTrackWriter sink(mgr, tracks, Overwrite,
	first, first + new_length - 1);

    // connect the objects
    bool ok = true;
    if (ok) ok = Kwave::connect(
	source,    SIGNAL(output(Kwave::SampleArray)),
	converter, SLOT(input(Kwave::SampleArray)));
    if (ok) ok = Kwave::connect(
	converter, SIGNAL(output(Kwave::SampleArray)),
	sink,      SLOT(input(Kwave::SampleArray)));
    if (!ok) {
	return;
    }

    while (!shouldStop() && !source.eof()) {
	source.goOn();
	converter.goOn();
    }

    sink.flush();

    // find out how many samples have been written and delete the leftovers
    unsigned int written = sink[0]->position() - first;
//     qDebug("SampleRatePlugin: old=%u, expexted=%u, written=%u",
// 	    length, new_length, written);
    if (written < length) {
	unsigned int to_delete = length - written;
	mgr.deleteRange(written, to_delete, tracks);
    }

    // adjust all label positions in the originally selected range
    // NOTE: if the ratio is > 1, work backwards, otherwise forward
    QListIterator<Label> it(mgr.labels());
    (ratio > 1) ? it.toBack() : it.toFront();
    while ((ratio > 1) ? it.hasPrevious() : it.hasNext()) {
	Label label = (ratio > 1) ? it.previous() : it.next();
	unsigned int pos = label.pos();
	if (pos < first) continue;
	if (pos > last)  continue;

	// label is in our range -> calculate new position
	pos -= first;
	pos *= ratio;
	pos += first;

	// move the label to his new position
	int index = mgr.labelIndex(label);
	if (!mgr.findLabel(pos).isNull()) {
	    // if there is already another label, drop this one
	    qWarning("SampleRatePlugin: deleting label at %u (%u is occupied)",
		    label.pos(), pos);
	    mgr.deleteLabel(index, true);
	} else {
// 	    qDebug("SampleRatePlugin: moving label from %u to %u",
// 		    label.pos(), pos);
	    mgr.modifyLabel(index, pos, label.name());
	}
    }

    // update the selection if it was not empty
    length = selection(0, &first, &last, false);
    if (length) {
	if (m_whole_signal) {
	    // if whole signal selected -> adjust start and end
	    first *= ratio;
	    last  *= ratio;
	    length = last - first + 1;
	} else {
	    // only a portion selected -> adjust only length
	    length *= ratio;
	}

	mgr.selectRange(first, length);
    }

    // set the sample rate if we modified the whole signal
    if (m_whole_signal) {
	FileInfo info = fileInfo();
	info.setRate(m_new_rate);
	mgr.setFileInfo(info, true);
    }

}

//***************************************************************************
#include "SampleRatePlugin.moc"
//***************************************************************************
//***************************************************************************
