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
#include <QProgressDialog>
#include <QStringList>

#include <klocale.h>

#include "libkwave/KwaveConnect.h"
#include "libkwave/KwaveSampleSink.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/PluginManager.h"
#include "libkwave/modules/KwaveStreamObject.h"
#include "libkwave/undo/UndoTransactionGuard.h"

#include "libgui/ConfirmCancelProxy.h"
#include "libgui/KwaveFilterPlugin.h"

//***************************************************************************
Kwave::FilterPlugin::FilterPlugin(const PluginContext &context)
    :KwavePlugin(context), m_params(),
     m_stop(false), m_listen(false), m_progress(0),
     m_confirm_cancel(0), m_pause(false), m_sink(0)
{
}

//***************************************************************************
Kwave::FilterPlugin::~FilterPlugin()
{
    if (m_confirm_cancel) delete m_confirm_cancel;
    if (m_progress)       delete m_progress;
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
    connect(this, SIGNAL(sigDone(KwavePlugin *)),
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
    m_stop  = false;

    if (!interpreteParameters(params)) m_params = params;

    unsigned int first, last;
    unsigned int tracks = selectedTracks().count();
    selection(&first, &last, true);

    // create all objects
    MultiTrackReader source(signalManager(), selectedTracks(), first, last);

    Kwave::SampleSource *filter = createFilter(tracks);
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
	    close();
	    return;
	}
	m_sink = new MultiTrackWriter(signalManager(), selectedTracks(),
	    Overwrite, first, last);
	Q_ASSERT(m_sink);
    }
    if (!filter || !m_sink || m_sink->done()) {
	if (filter)     delete filter;
	if (undo_guard) delete undo_guard;
	if (m_sink)     delete m_sink;
	m_sink = 0;
	if (!m_listen) close();
	return;
    }

    // set up the progress dialog when in processing (not pre-listen) mode
    if (!m_listen && m_progress && m_confirm_cancel) {
	connect(&source, SIGNAL(progress(unsigned int)),
		this,    SLOT(updateProgress(unsigned int)),
		Qt::BlockingQueuedConnection);
    }

    // force initial update of the filter settings
    updateFilter(filter, true);

    // connect them
    Kwave::connect(source,  SIGNAL(output(Kwave::SampleArray &)),
                   *filter, SLOT(input(Kwave::SampleArray &)));
    Kwave::connect(*filter, SIGNAL(output(Kwave::SampleArray &)),
                   *m_sink, SLOT(input(Kwave::SampleArray &)));

    // transport the samples
    while (!m_stop && (!source.done() || m_listen)) {
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
	while (m_pause)
	    sleep(1);
    }

    // cleanup
    if (filter)     delete filter;
    if (m_sink)     delete m_sink;
    m_sink = 0;
    if (undo_guard) delete undo_guard;

    m_pause  = false;
    m_stop   = false;
    m_listen = false;

    close();
}

//***************************************************************************
void Kwave::FilterPlugin::updateProgress(unsigned int progress)
{
    Q_ASSERT(m_progress);
    if (m_progress) m_progress->setValue(progress);
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
int Kwave::FilterPlugin::start(QStringList &params)
{
    Q_ASSERT(!m_progress);
    Q_ASSERT(!m_confirm_cancel);

    // create a progress dialog for processing mode (not used for pre-listen)
    if (!m_listen) {
	m_progress = new QProgressDialog(parentWidget());
	Q_ASSERT(m_progress);
    }

    // set up the progress dialog when in processing (not pre-listen) mode
    if (!m_listen && m_progress) {
	unsigned int first, last;
	unsigned int tracks = selectedTracks().count();

	selection(&first, &last, true);
	m_progress->setModal(true);
	m_progress->setVisible(false);
	m_progress->setMinimumDuration(1000);
	m_progress->setAutoClose(true);
	m_progress->setMaximum((last-first+1)*tracks);
	m_progress->setValue(0);
	m_progress->setLabelText(
	    i18n("applying '%1' ...", actionName()));
	int h = m_progress->sizeHint().height();
	int w = m_progress->sizeHint().height();
	if (w < 4*h) w = 4*h;
	m_progress->setFixedSize(w, h);

	// use a "proxy" that asks for confirmation of cancel
	m_confirm_cancel = new ConfirmCancelProxy(m_progress,
		0, 0, this, SLOT(cancel()));
	Q_ASSERT(m_confirm_cancel);
	connect(m_progress,      SIGNAL(canceled()),
		m_confirm_cancel, SLOT(cancel()));
	m_progress->setVisible(true);
    }

    return KwavePlugin::start(params);
}

//***************************************************************************
int Kwave::FilterPlugin::stop()
{
    m_stop = true;
    int result = KwavePlugin::stop();

    if (m_confirm_cancel) delete m_confirm_cancel;
    if (m_progress)       delete m_progress;
    m_confirm_cancel = 0;
    m_progress = 0;

    return result;
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
	static QStringList empty_list;
	use();
	execute(empty_list);
    }
}

//***************************************************************************
void Kwave::FilterPlugin::stopPreListen()
{
    stop();
    m_listen = false;
}

//***************************************************************************
void Kwave::FilterPlugin::cancel()
{
    m_stop = true;
}

//***************************************************************************
using namespace Kwave;
#include "KwaveFilterPlugin.moc"
//***************************************************************************
//***************************************************************************
