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

#include <QtCore/QList>
#include <QtCore/QStringList>

#include "libkwave/MultiTrackReader.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/PluginManager.h"
#include "libkwave/SignalManager.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"
#include "libkwave/Writer.h"
#include "libkwave/undo/UndoTransactionGuard.h"

#include "libgui/SelectTimeWidget.h" // for selection mode

#include "DebugPlugin.h"

KWAVE_PLUGIN(Kwave::DebugPlugin, "debug", "2.3",
             I18N_NOOP("Debug Functions"), "Thomas Eschenbacher");

/** size of the internal buffer */
#define BUFFER_SIZE (64 * 1024)

/** helper for generating menu entries */
#define MENU_ENTRY(cmd,txt) \
    emitCommand(entry.arg(_(cmd)).arg(txt));

//***************************************************************************
Kwave::DebugPlugin::DebugPlugin(Kwave::PluginManager &plugin_manager)
    :Kwave::Plugin(plugin_manager), m_buffer()
{
}

//***************************************************************************
Kwave::DebugPlugin::~DebugPlugin()
{
}

//***************************************************************************
void Kwave::DebugPlugin::load(QStringList &params)
{
    Q_UNUSED(params);

    QString entry = _("menu (plugin:execute(debug,%1),Calculate/Debug/%2)");

    MENU_ENTRY("dc_50",             _(I18N_NOOP("Generate 50% DC Level")));
    MENU_ENTRY("dc_100",            _(I18N_NOOP("Generate 100% DC Level")));
    MENU_ENTRY("min_max",           _(I18N_NOOP("MinMax Pattern")));
    MENU_ENTRY("sawtooth",          _(I18N_NOOP("Generate Sawtooth Pattern")));
    MENU_ENTRY("sawtooth_verify",   _(I18N_NOOP("Verify Sawtooth Pattern")));
    MENU_ENTRY("fm_sweep",          _(I18N_NOOP("FM Sweep")));
//     MENU_ENTRY("stripe_index",      _(I18N_NOOP("Stripe Index")));
//     MENU_ENTRY("hull_curve",        _(I18N_NOOP("Hull Curve")));
//     MENU_ENTRY("offset_in_stripe",  _(I18N_NOOP("Offset in Stripe")));
//     MENU_ENTRY("stripe_borders",    _(I18N_NOOP("Show Stripe Borders")));
    MENU_ENTRY("labels_at_stripes", _(I18N_NOOP("Labels at Stripe borders")));

    emitCommand(_("menu (dump_metadata(), ") +
                _(I18N_NOOP("Help")) + _("/") +
                _(I18N_NOOP("Dump Meta Data")) + _(")"));
}

//***************************************************************************
void Kwave::DebugPlugin::run(QStringList params)
{
    sample_index_t first = 0;
    sample_index_t last  = 0;
    Kwave::SignalManager &sig    = signalManager();
    const double          rate   = signalRate();

    if (params.count() != 1) return;

    QString command = params.first();
    QString action = i18n("Debug (%1)", command);
    Kwave::UndoTransactionGuard undo_guard(*this, action);

    // get the buffer for faster processing
    {
	bool ok = m_buffer.resize(BUFFER_SIZE);
	Q_ASSERT(ok);
	if (!ok) return;
	m_buffer.fill(0);
    }

    bool make_new_track = (
	(command == _("stripe_index")) ||
	(command == _("offset_in_stripe")) ||
	(command == _("stripe_borders"))
    );

    if (command == _("min_max")) {
	// toggle between minimum and maximum possible sample value
	for (unsigned int i = 0; i < BUFFER_SIZE; i++)
	    m_buffer[i] = (i & 1) ? SAMPLE_MIN : SAMPLE_MAX;
    }

    if (command == _("labels_at_stripes")) {
	QList<Kwave::Stripe::List> all_stripes = sig.stripes(sig.allTracks());
	if (all_stripes.isEmpty()) return;

	const Kwave::Stripe::List &stripes = all_stripes.first();
	unsigned int index = 0;
	foreach (const Kwave::Stripe &stripe, stripes) {
	    QString text;
	    text = text.sprintf("stripe #%d [%llu .. %llu]",
		index++, stripe.start(), stripe.end());
	    sig.addLabel(stripe.start(), text);
	}
	return;
    } else if (command == _("dump_metadata")) {
	sig.metaData().dump();
	return;
    } else if (command == _("sawtooth_verify")) {
	Kwave::MultiTrackReader *readers = new Kwave::MultiTrackReader(
	    Kwave::SinglePassForward, sig, sig.selectedTracks(), first, last);
	Q_ASSERT(readers);
	if (!readers) return;

	sample_index_t pos = first;
	bool ok = true;
	while (ok && (first <= last) && (!shouldStop())) {
	    sample_index_t rest = last - first + 1;
	    if (rest < m_buffer.size()) {
		ok = m_buffer.resize(Kwave::toUint(rest));
		Q_ASSERT(ok);
		if (!ok) break;
	    }

	    unsigned int count = readers->tracks();
	    for (unsigned int r = 0; ok && (r < count); r++) {
		*((*readers)[r]) >> m_buffer;
		for (unsigned int ofs = 0; ofs < m_buffer.size(); ++ofs) {
		    sample_t value_is = m_buffer[ofs];
		    sample_t value_should = SAMPLE_MIN + static_cast<sample_t>(
			((pos + ofs) % (SAMPLE_MAX - SAMPLE_MIN)));
		    if (value_is != value_should) {
			qWarning("ERROR: mismatch detected at offset %llu: "
			         "value=%d, expected=%d", pos + ofs,
			         value_is, value_should);
			ok = false;
			break;
		    }
		}
		if (!ok) break;
	    }

	    pos   += m_buffer.size();
	    first += m_buffer.size();
	}
	delete readers;
	if (ok) {
	    qDebug("test pattern successfully detected, no errors :-)");
	}
	return;
    }

    Kwave::MultiTrackWriter *writers = 0;

    if (make_new_track) {
	// append a new track
	sig.appendTrack();

	// and use only the new track as target
	last = signalLength() - 1;
	QList<unsigned int> track_list;
	track_list.append(sig.tracks() - 1);
	writers = new Kwave::MultiTrackWriter(sig, track_list,
	                                      Kwave::Overwrite, 0, last);
    } else {
	// use all currently selected tracks
	writers = new Kwave::MultiTrackWriter(sig, Kwave::Overwrite);
    }

    Q_ASSERT(writers);
    if (!writers) return; // out-of-memory

    // break if aborted
    if (!writers->tracks()) return;

    first = (*writers)[0]->first();
    last  = (*writers)[0]->last();
    unsigned int count = writers->tracks();

    // connect the progress dialog
    connect(writers, SIGNAL(progress(qreal)),
	    this,  SLOT(updateProgress(qreal)),
	     Qt::BlockingQueuedConnection);

    // loop over the sample range
    const sample_index_t left   = first;
    const sample_index_t right  = last;
    const sample_index_t length = right - left + 1;
    sample_index_t pos = first;
    while ((first <= last) && (!shouldStop())) {
	sample_index_t rest = last - first + 1;
	if (rest < m_buffer.size()) {
	    bool ok = m_buffer.resize(Kwave::toUint(rest));
	    Q_ASSERT(ok);
	    if (!ok) break;
	}

	// sawtooth pattern from min to max
	if (command == _("fm_sweep")) {
	    const double f_max = rate / 2.0;
	    const double f_min = 1;
	    for (unsigned int i = 0; i < m_buffer.size(); i++, pos++) {
		double t = static_cast<double>((pos - left) / rate);
		double f = f_min + (((f_max - f_min) *
		    static_cast<double>(pos - left)) /
		    static_cast<double>(length));
		double y = 0.707 * sin(M_PI * f * t);
		m_buffer[i] = double2sample(y);
	    }
	} else if (command == _("sawtooth")) {
	    for (unsigned int i = 0; i < m_buffer.size(); i++, pos++) {
		m_buffer[i] = SAMPLE_MIN +
		    static_cast<sample_t>(pos % (SAMPLE_MAX - SAMPLE_MIN));
	    }
	} else if (command == _("dc_50")) {
	    const sample_t s = float2sample(0.5);
	    for (unsigned int i = 0; i < m_buffer.size(); i++) {
		m_buffer[i] = s;
	    }
	} else if (command == _("dc_100")) {
	    const sample_t s = SAMPLE_MAX;
	    for (unsigned int i = 0; i < m_buffer.size(); i++) {
		m_buffer[i] = s;
	    }
	}

	// loop over all writers
	for (unsigned int w = 0; w < count; w++) {
	    *((*writers)[w]) << m_buffer;
	}

	first += m_buffer.size();
    }

    delete writers;
}

//***************************************************************************
#include "DebugPlugin.moc"
//***************************************************************************
//***************************************************************************
