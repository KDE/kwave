/***************************************************************************
  AmplifyFreePlugin.cpp  -  Plugin for free amplification curves
                             -------------------
    begin                : Sun Sep 02 2001
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

#include <KI18n/KLocalizedString>
#include <QStringList>

#include "libkwave/Connect.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/MultiTrackSource.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/Parser.h"
#include "libkwave/PluginManager.h"
#include "libkwave/String.h"
#include "libkwave/modules/CurveStreamAdapter.h"
#include "libkwave/modules/Mul.h"
#include "libkwave/undo/UndoTransactionGuard.h"

#include "AmplifyFreeDialog.h"
#include "AmplifyFreePlugin.h"

KWAVE_PLUGIN(Kwave::AmplifyFreePlugin, "amplifyfree", "2.3",
             I18N_NOOP("Amplify Free"), "Thomas Eschenbacher");

//***************************************************************************
Kwave::AmplifyFreePlugin::AmplifyFreePlugin(Kwave::PluginManager &plugin_manager)
    :Kwave::Plugin(plugin_manager), m_action_name(), m_params(), m_curve(),
     m_cmd_map()
{
    m_cmd_map[_("fade in")]      = i18n("Fade In");
    m_cmd_map[_("fade out")]     = i18n("Fade Out");
    m_cmd_map[_("fade intro")]   = i18n("Fade Intro");
    m_cmd_map[_("fade leadout")] = i18n("Fade Leadout");
}

//***************************************************************************
Kwave::AmplifyFreePlugin::~AmplifyFreePlugin()
{
}

//***************************************************************************
int Kwave::AmplifyFreePlugin::interpreteParameters(QStringList &params)
{
    // store last parameters
    m_params = params;

    m_action_name = _("");
    if (params.count() < 2) return -1;
    if (params.count() & 1) return -1; // no. of params must be even

    // first list entry == name of operation
    m_action_name = (params[0].length() && m_cmd_map.contains(params[0])) ?
	 m_cmd_map[params[0]] : i18n("Amplify Free");

    // convert string list into command again...
    QString cmd = _("curve(");
    for (int i = 1; i < params.count(); ++i) {
	cmd += params[i];
	if ((i + 1) < params.count()) cmd += _(",");
    }
    cmd += _(")");

    // and initialize our curve with it
    m_curve.fromCommand(cmd);

    return 0;
}

//***************************************************************************
QStringList *Kwave::AmplifyFreePlugin::setup(QStringList &previous_params)
{
    // try to interprete the previous parameters
    interpreteParameters(previous_params);

    // create the setup dialog
    Kwave::AmplifyFreeDialog *dialog =
	new Kwave::AmplifyFreeDialog(parentWidget());
    Q_ASSERT(dialog);
    if (!dialog) return 0;

    // remove the first list entry (action name), the rest is for the dialog
    if ((m_params.count() > 2) && !(m_params.count() & 1)) {
	QStringList curve_params = m_params;
	curve_params.takeFirst(); // ignore action name
	dialog->setParams(curve_params);
    }

    QStringList *list = new QStringList();
    Q_ASSERT(list);
    if (list && dialog->exec()) {
	// user has pressed "OK"
	*list << _("amplify free");
	QString cmd = dialog->getCommand();
	Kwave::Parser p(cmd);
	while (!p.isDone()) *list << p.nextParam();

	qDebug("setup -> emitCommand('%s')", DBG(cmd));
	emitCommand(cmd);
    } else {
	// user pressed "Cancel"
	if (list) delete list;
	list = 0;
    }

    if (dialog) delete dialog;
    return list;
}

//***************************************************************************
QString Kwave::AmplifyFreePlugin::progressText()
{
    return m_action_name;
}

//***************************************************************************
int Kwave::AmplifyFreePlugin::start(QStringList &params)
{
    interpreteParameters(params);
    return Kwave::Plugin::start(params);
}

//***************************************************************************
void Kwave::AmplifyFreePlugin::run(QStringList params)
{
    sample_index_t first, last;
    QList<unsigned int> track_list;

    interpreteParameters(params);

    Kwave::UndoTransactionGuard undo_guard(*this, m_action_name);

    sample_index_t input_length = selection(&track_list, &first, &last, true);
    unsigned int   tracks       = track_list.count();

    // create all objects
    Kwave::MultiTrackReader source(Kwave::SinglePassForward,
	signalManager(), selectedTracks(), first, last);
    Kwave::CurveStreamAdapter curve(m_curve, input_length);
    Kwave::MultiTrackWriter sink(signalManager(), track_list, Kwave::Overwrite,
	first, last);
    Kwave::MultiTrackSource<Kwave::Mul, true> mul(tracks, this);

    // break if aborted
    if (!sink.tracks()) return;

    // connect them
    bool ok = true;
    if (ok) ok = Kwave::connect(
	source, SIGNAL(output(Kwave::SampleArray)),
	mul,    SLOT(input_a(Kwave::SampleArray)));
    if (ok) ok = Kwave::connect(
	curve,  SIGNAL(output(Kwave::SampleArray)),
	mul,    SLOT(input_b(Kwave::SampleArray)));
    if (ok) ok = Kwave::connect(
	mul,    SIGNAL(output(Kwave::SampleArray)),
	sink,   SLOT(input(Kwave::SampleArray)));
    if (!ok) {
	return;
    }

    // connect the progress dialog
    connect(&sink, SIGNAL(progress(qreal)),
	    this,  SLOT(updateProgress(qreal)),
	    Qt::BlockingQueuedConnection);

    // transport the samples
    qDebug("AmplifyFreePlugin: filter started...");
    while (!shouldStop() && !source.done()) {
	source.goOn();
	curve.goOn();
	/* mul.goOn(); */
    }
    qDebug("AmplifyFreePlugin: filter done.");
}

//***************************************************************************
//***************************************************************************
//***************************************************************************
