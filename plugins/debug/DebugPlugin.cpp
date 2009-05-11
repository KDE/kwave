/*************************************************************************
        DebugPlugin.cpp  -  various debug aids
                             -------------------
    begin                : Mon Feb 02 2009
    copyright            : (C) 2009 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de
 ***************************************************************************/

#include "config.h"
#include <math.h>
#include <klocale.h> // for the i18n macro

#include <QList>
#include <QStringList>

#include "libkwave/MultiTrackWriter.h"
#include "libkwave/PluginManager.h"
#include "libkwave/SampleWriter.h"
#include "libkwave/SignalManager.h"
#include "libkwave/undo/UndoTransactionGuard.h"

#include "libgui/SelectTimeWidget.h" // for selection mode

#include "DebugPlugin.h"

KWAVE_PLUGIN(DebugPlugin,"debug","Thomas Eschenbacher");

/** size of the internal buffer */
#define BUFFER_SIZE (64 * 1024)

/** helper for generating menu entries */
#define MENU_ENTRY(cmd,txt) \
    emitCommand(entry.arg(cmd).arg(i18n(txt)));

//***************************************************************************
DebugPlugin::DebugPlugin(const PluginContext &context)
    :Kwave::Plugin(context), m_buffer()
{
     i18n("debug");
}

//***************************************************************************
DebugPlugin::~DebugPlugin()
{
}

//***************************************************************************
void DebugPlugin::load(QStringList &params)
{
    Q_UNUSED(params);

    QString entry = "menu (plugin:execute(debug,%1),&Calculate/Debug/%2)";

    MENU_ENTRY("min_max",           "MinMax Pattern");
    MENU_ENTRY("sawtooth",          "Generate Sawtooth Pattern");
//     MENU_ENTRY("sawtooth",          "Verify Sawtooth Pattern");
//     MENU_ENTRY("stripe_index",      "Stripe Index");
//     MENU_ENTRY("hull_curve",        "Hull Curve");
//     MENU_ENTRY("offset_in_stripe",  "Offset in Stripe");
//     MENU_ENTRY("stripe_borders",    "Show Stripe Borders");
//     MENU_ENTRY("labels_at_stripes", "Generate Labels at Stripe borders");
}

//***************************************************************************
void DebugPlugin::run(QStringList params)
{
    unsigned int first = 0;
    unsigned int last  = 0;

    if (params.count() != 1) return;

    QString command = params.first();
    QString action = i18n("debug (%1)", command);
    UndoTransactionGuard undo_guard(*this, action);

    // get the buffer for faster processing
    if (m_buffer.size() != BUFFER_SIZE) {
	m_buffer.resize(BUFFER_SIZE);
	m_buffer.fill(0);
    }
    Q_ASSERT(m_buffer.size() == BUFFER_SIZE);

    bool make_new_track = (
	(command == "stripe_index") ||
	(command == "offset_in_stripe") ||
	(command == "stripe_borders")
    );

    if (command == "min_max") {
	// toggle between minimum and maximum possible sample value
	for (unsigned int i = 0; i < BUFFER_SIZE; i++)
	    m_buffer[i] = (i & 1) ? SAMPLE_MIN : SAMPLE_MAX;
    }

    MultiTrackWriter *writers = 0;

    if (make_new_track) {
	// append a new track
	signalManager().appendTrack();

	// and use only the new track as target
	last = signalLength() - 1;
	QList<unsigned int> track_list;
	track_list.append(signalManager().tracks() - 1);
	writers = new MultiTrackWriter(signalManager(), track_list,
	                               Overwrite, 0, last);
    } else {
	// use all currently selected tracks
	writers = new MultiTrackWriter(signalManager(), Overwrite);
    }

    Q_ASSERT(writers);
    if (!writers) return; // out-of-memory

    // break if aborted
    if (!writers->tracks()) return;

    first = (*writers)[0]->first();
    last  = (*writers)[0]->last();
    unsigned int count = writers->tracks();

    // connect the progress dialog
    connect(writers, SIGNAL(progress(unsigned int)),
	    this,  SLOT(updateProgress(unsigned int)),
	    Qt::QueuedConnection);

    // loop over the sample range
    sample_t v = 0;
    while ((first <= last) && (!shouldStop())) {
	unsigned int rest = last - first + 1;
	if (rest < m_buffer.size()) m_buffer.resize(rest);

	// sawtooth pattern from min to max
	if (command == "sawtooth") {
	    unsigned int shift = SAMPLE_BITS - fileInfo().bits();
	    for (unsigned int i = 0; i < m_buffer.size(); i++) {
		m_buffer[i] = v;
		v += (1 << shift);
		if (v > SAMPLE_MAX) v = 0;
	    }
	}

	// loop over all writers
	for (unsigned int w = 0; w < count; w++) {
	    *((*writers)[w]) << m_buffer;
	}

	first += m_buffer.size();
    }

    delete writers;
    close();
}

//***************************************************************************
#include "DebugPlugin.moc"
//***************************************************************************
//***************************************************************************
