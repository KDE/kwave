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
#include <qobject.h>
#include <qdialog.h>
#include <qglobal.h>
#include <qprogressdialog.h>
#include <qstringlist.h>

#include <klocale.h>

#include "libkwave/ArtsMultiTrackSink.h"
#include "libkwave/ArtsMultiTrackSource.h"
#include "libkwave/ArtsMultiTrackFilter.h"
#include "libkwave/KwaveFilterPlugin.h"

#include "libgui/ConfirmCancelProxy.h"

#include "kwave/PluginManager.h"
#include "kwave/UndoTransactionGuard.h"

//***************************************************************************
KwaveFilterPlugin::KwaveFilterPlugin(PluginContext &context)
    :KwavePlugin(context), m_params(),
     m_stop(false), m_listen(false), m_progress(0), m_spx_progress(0),
     m_confirm_cancel(0), m_pause(false)
{
    m_spx_progress = new SignalProxy1< unsigned int >
	(this, SLOT(updateProgress()), 2);
}

//***************************************************************************
KwaveFilterPlugin::~KwaveFilterPlugin()
{
    if (!m_progress) delete m_progress;
    m_progress = 0;
}

//***************************************************************************
QStringList *KwaveFilterPlugin::setup(QStringList &previous_params)
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
    connect(this, SIGNAL(sigDone()),
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
};

//***************************************************************************
void KwaveFilterPlugin::run(QStringList params)
{
    unsigned int first, last;

    Arts::Dispatcher *dispatcher = manager().artsDispatcher();
    Q_ASSERT(dispatcher);
    if (!dispatcher) close();
    dispatcher->lock();

    UndoTransactionGuard *undo_guard = 0;
    m_pause = false;
    m_stop  = false;

    if (!interpreteParameters(params)) m_params = params;

    MultiTrackReader source;
    MultiTrackWriter sink;

    selection(&first, &last, true);
    manager().openMultiTrackReader(source, selectedTracks(), first, last);

    // create all objects
    ArtsMultiTrackSource arts_source(source);
    ArtsMultiSink *arts_sink = 0;

    unsigned int tracks = selectedTracks().count();

    ArtsMultiTrackFilter *filter = createFilter(tracks);
    Q_ASSERT(filter);

    if (m_listen) {
	// pre-listen mode
	arts_sink = manager().openMultiTrackPlayback(selectedTracks().count());
    } else {
	// normal mode, with undo
	undo_guard = new UndoTransactionGuard(*this, actionName());
	Q_ASSERT(undo_guard);
	if (!undo_guard) {
	    close();
	    return;
	}
	manager().openMultiTrackWriter(sink, selectedTracks(), Overwrite,
	    first, last);
	arts_sink = new ArtsMultiTrackSink(sink);
    }

    Q_ASSERT(arts_sink);
    if (!filter || !arts_sink || arts_sink->done()) {
	if (filter)     delete filter;
	if (arts_sink)  delete arts_sink;
	if (undo_guard) delete undo_guard;
	if (!m_listen) close();
	return;
    }

    // create a progress dialog when in processing (not pre-listen) mode
    if (!m_listen) {
	Q_ASSERT(!m_progress);
	Q_ASSERT(!m_confirm_cancel);
	m_progress = new QProgressDialog(parentWidget(), name(), true);
	Q_ASSERT(m_progress);
	if (m_progress) {
	    m_progress->setMinimumDuration(1000);
	    m_progress->setTotalSteps((last-first+1)*tracks);
	    m_progress->setAutoClose(true);
	    m_progress->setProgress(0);
	    m_progress->setLabelText(
	        i18n("applying '%1' ...").arg(name()));
	    m_progress->setFixedSize(m_progress->sizeHint());
	
	    connect(&source, SIGNAL(progress(unsigned int)),
	            this,    SLOT(forwardProgress(unsigned int)));
	    connect(m_progress, SIGNAL(cancelled()),
	            this,       SLOT(forwardCancel()));
	    m_confirm_cancel = new ConfirmCancelProxy(m_progress,
	            0, 0, this, SLOT(cancel()));
	    Q_ASSERT(m_confirm_cancel);
	}
    }
    
    // force initial update of the filter settings
    updateFilter(filter, true);

    // connect them
    filter->connectInput(arts_source,  "source",  "invalue");
    filter->connectOutput(*arts_sink,  "sink",    "outvalue");

    // start all
    arts_source.start();
    filter->start();
    arts_sink->start();

    // transport the samples
    while (!m_stop && (!arts_source.done() || m_listen)) {
	// this lets the process wait if the user pressed cancel
	// and the confirm_cancel dialog is active
	while (m_pause)
	    sleep(1);

	// process one step
	arts_sink->goOn();

	// watch out for changed parameters when in
	// pre-listen mode
	if (m_listen && paramsChanged()) {
	    updateFilter(filter);
        }

	if (m_listen && arts_source.done()) {
	    // start the next loop
	    source.reset();
	    arts_source.start();
	    continue;
	}

    }

    // shutdown
    filter->stop();
    arts_sink->stop();
    arts_source.stop();

    dispatcher->unlock();

    delete arts_sink;
    if (m_progress) m_progress->deleteLater();
    m_progress = 0;
    if (m_confirm_cancel) m_confirm_cancel->deleteLater();
    m_confirm_cancel = 0;

    if (undo_guard) delete undo_guard;
    close();
}

//***************************************************************************
void KwaveFilterPlugin::forwardCancel()
{
    m_pause = true;
    if (m_confirm_cancel) m_confirm_cancel->cancel();
    m_pause = false;
}

//***************************************************************************
void KwaveFilterPlugin::forwardProgress(unsigned int progress)
{
    if (m_spx_progress) m_spx_progress->enqueue(progress);
}

//***************************************************************************
void KwaveFilterPlugin::updateProgress()
{
    if (!m_progress || !m_spx_progress) return;

    unsigned int *progress = m_spx_progress->dequeue();
    if (!progress) return;
    m_progress->setProgress(*progress);
    delete progress;
}

//***************************************************************************
bool KwaveFilterPlugin::paramsChanged()
{
    return false;
}

//***************************************************************************
void KwaveFilterPlugin::updateFilter(ArtsMultiTrackFilter * /*filter*/,
                                     bool /*force*/)
{
    /* default implementation, does nothing */
}

//***************************************************************************
int KwaveFilterPlugin::stop()
{
    m_stop = true;
    return KwavePlugin::stop();
}

//***************************************************************************
void KwaveFilterPlugin::startPreListen()
{
    m_listen = true;
    static QStringList empty_list;
    use();
    execute(empty_list);
}

//***************************************************************************
void KwaveFilterPlugin::stopPreListen()
{
    stop();
    m_listen = false;
}

//***************************************************************************
QString KwaveFilterPlugin::actionName()
{
    return this->name();
}

//***************************************************************************
void KwaveFilterPlugin::cancel()
{
    m_stop = true;
}

//***************************************************************************
//***************************************************************************
