/***************************************************************************
  KwaveFilterPlugin.cpp  -  generic class for filter plugins with setup
                             -------------------
    begin                : Sat Jun 07 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
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
#include <unistd.h>

#include <QApplication>
#include <QDialog>
#include <QStringList>

#include <klocale.h>

#include "libkwave/KwaveConnect.h"
#include "libkwave/KwaveSampleSink.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/PluginManager.h"
#include "libkwave/modules/KwaveStreamObject.h"
#include "libkwave/undo/UndoTransactionGuard.h"

#include "libgui/KwaveFilterPlugin.h"

//***************************************************************************
Kwave::FilterPlugin::FilterPlugin(const PluginContext &context)
    :Kwave::Plugin(context), m_params(),
     m_listen(false), m_pause(false), m_sink(0)
{
}

//***************************************************************************
Kwave::FilterPlugin::~FilterPlugin()
{
}

//***************************************************************************
QStringList *Kwave::FilterPlugin::setup(QStringList &previous_params)
{
    // try to interprete and use the previous parameters
    if (!interpreteParameters(previous_params))
	m_params = previous_params;

    // create the setup dialog
    KwavePluginSetupDialog *setup_dialog = createDialog(parentWidget());
    Q_ASSERT(setup_dialog);
    if (!setup_dialog) return 0;

    // connect the signals for the pre-listen handling
    QDialog *dlg = setup_dialog->dialog();
    connect(dlg, SIGNAL(startPreListen()),
            this, SLOT(startPreListen()));
    connect(dlg, SIGNAL(stopPreListen()),
            this, SLOT(stopPreListen()));
    connect(this, SIGNAL(sigDone(Kwave::Plugin *)),
            dlg, SLOT(listenStopped()));

    if (!m_params.isEmpty()) setup_dialog->setParams(m_params);

    QStringList *list = new QStringList();
    Q_ASSERT(list);
    if (list && dlg->exec()) {
	// user has pressed "OK"
	*list = setup_dialog->params();
    } else {
	// user pressed "Cancel"
	if (list) delete list;
	list = 0;
    }

    if (setup_dialog) delete setup_dialog;
    return list;
}

//***************************************************************************
void Kwave::FilterPlugin::run(QStringList params)
{
    UndoTransactionGuard *undo_guard = 0;
    m_pause = false;

    if (!interpreteParameters(params)) m_params = params;

    unsigned int first, last;
    QList<unsigned int> tracks;
    selection(&tracks, &first, &last, true);

    // switch to interactive mode in pre-listen mode
    Kwave::StreamObject::setInteractive(m_listen);

    // create all objects
    MultiTrackReader source(
	(m_listen) ? Kwave::FullSnapshot : Kwave::SinglePassForward,
	signalManager(), tracks, first, last);

    Kwave::SampleSource *filter = createFilter(tracks.count());
    Q_ASSERT(filter);

    if (m_listen) {
	// pre-listen mode
	Q_ASSERT(m_sink);
    } else {
	// normal mode, with undo
	undo_guard = new UndoTransactionGuard(*this, actionName());
	Q_ASSERT(undo_guard);
	if (!undo_guard) {
	    if (filter) delete filter;
	    Kwave::StreamObject::setInteractive(false);
	    return;
	}
	m_sink = new MultiTrackWriter(signalManager(), tracks,
	    Overwrite, first, last);
	Q_ASSERT(m_sink);
    }
    if (!filter || !m_sink || m_sink->done()) {
	if (filter)     delete filter;
	if (undo_guard) delete undo_guard;
	if (m_sink)     delete m_sink;
	m_sink = 0;
	Kwave::StreamObject::setInteractive(false);
	return;
    }

    // set up the progress dialog when in processing (not pre-listen) mode
    if (!m_listen) {
	connect(&source, SIGNAL(progress(unsigned int)),
		this,    SLOT(updateProgress(unsigned int)),
		Qt::BlockingQueuedConnection);
    }

    // force initial update of the filter settings
    updateFilter(filter, true);

    // connect them
    Kwave::connect(source,  SIGNAL(output(Kwave::SampleArray)),
                   *filter, SLOT(input(Kwave::SampleArray)));
    Kwave::connect(*filter, SIGNAL(output(Kwave::SampleArray)),
                   *m_sink, SLOT(input(Kwave::SampleArray)));

    // transport the samples
    while (!shouldStop() && (!source.done() || m_listen)) {
	// process one step
	source.goOn();
	filter->goOn();

	// watch out for changed parameters when in
	// pre-listen mode
	if (m_listen && paramsChanged()) {
	    updateFilter(filter);
        }

	if (m_listen && source.done()) {
	    // start the next loop
	    source.reset();
	    continue;
	}

	// this lets the process wait if the user pressed cancel
	// and the confirm_cancel dialog is active
	while (m_pause && !shouldStop())
	    sleep(1);
    }

    // cleanup
    if (filter)     delete filter;
    if (m_sink)     delete m_sink;
    m_sink = 0;
    if (undo_guard) delete undo_guard;

    m_pause  = false;
    m_listen = false;

    Kwave::StreamObject::setInteractive(false);
}

//***************************************************************************
bool Kwave::FilterPlugin::paramsChanged()
{
    return false;
}

//***************************************************************************
void Kwave::FilterPlugin::updateFilter(Kwave::SampleSource * /*filter*/,
                                       bool /*force*/)
{
    /* default implementation, does nothing */
}

//***************************************************************************
void Kwave::FilterPlugin::startPreListen()
{
    Q_ASSERT(!m_sink);
    if (m_sink) delete m_sink;
    m_sink = manager().openMultiTrackPlayback(selectedTracks().count());
    Q_ASSERT(m_sink);

    if (m_sink) {
	m_listen = true;
	setProgressDialogEnabled(false);
	static QStringList empty_list;
	execute(empty_list);
    }
}

//***************************************************************************
void Kwave::FilterPlugin::stopPreListen()
{
    stop();
    m_listen = false;
    setProgressDialogEnabled(true);
}

//***************************************************************************
QString Kwave::FilterPlugin::progressText()
{
    return i18n("Applying '%1'...", actionName());
}

//***************************************************************************
using namespace Kwave;
#include "KwaveFilterPlugin.moc"
//***************************************************************************
//***************************************************************************
