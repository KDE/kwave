/***************************************************************************
      LowPassPlugin.cpp  -  Plugin for simple lowpass filtering
                             -------------------
    begin                : Fri Mar 07 2003
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

#include <qstringlist.h>
#include <klocale.h>

#include "libkwave/Parser.h"

#include "libkwave/ArtsMultiTrackSink.h"
#include "libkwave/ArtsMultiTrackSource.h"
#include "libkwave/ArtsKwaveMultiTrackFilter.h"
#include "libkwave/ArtsNativeMultiTrackFilter.h"

#include "kwave/PluginManager.h"
#include "kwave/UndoTransactionGuard.h"

#include "LowPassPlugin.h"
#include "LowPassDialog.h"

KWAVE_PLUGIN(LowPassPlugin,"lowpass","Thomas Eschenbacher");

//***************************************************************************
LowPassPlugin::LowPassPlugin(PluginContext &context)
    :KwavePlugin(context), m_params(), m_frequency(3500.0), m_stop(false)
{
}

//***************************************************************************
LowPassPlugin::~LowPassPlugin()
{
}

//***************************************************************************
int LowPassPlugin::interpreteParameters(QStringList &params)
{
    bool ok;
    QString param;

    // evaluate the parameter list
    Q_ASSERT(params.count() == 1);
    if (params.count() != 1) return -EINVAL;

    param = params[0];
    m_frequency = param.toDouble(&ok);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    // all parameters accepted
    m_params = params;
    
    return 0;
}

//***************************************************************************
QStringList *LowPassPlugin::setup(QStringList &previous_params)
{
    // try to interprete the previous parameters
    interpreteParameters(previous_params);

    // create the setup dialog
    LowPassDialog *dialog = new LowPassDialog(parentWidget(), signalRate());
    Q_ASSERT(dialog);
    if (!dialog) return 0;

    if (!m_params.isEmpty()) dialog->setParams(m_params);

    QStringList *list = new QStringList();
    Q_ASSERT(list);
    if (list && dialog->exec()) {
	// user has pressed "OK"
	*list = dialog->params();
    } else {
	// user pressed "Cancel"
	if (list) delete list;
	list = 0;
    }

    if (dialog) delete dialog;
    return list;
};

//***************************************************************************
void LowPassPlugin::run(QStringList params)
{
    unsigned int first, last;

    Arts::Dispatcher *dispatcher = manager().artsDispatcher();
    dispatcher->lock();
    Q_ASSERT(dispatcher);
    if (!dispatcher) close();

    UndoTransactionGuard undo_guard(*this, i18n("low pass"));
    m_stop = false;

    interpreteParameters(params);

    MultiTrackReader source;
    MultiTrackWriter sink;

    /*unsigned int input_length =*/ selection(&first, &last, true);
    manager().openMultiTrackReader(source, selectedTracks(), first, last);
    manager().openMultiTrackWriter(sink, selectedTracks(), Overwrite,
	first, last);

    // create all objects
    ArtsMultiTrackSource arts_source(source);

    unsigned int tracks = selectedTracks().count();
    ArtsNativeMultiTrackFilter filter(tracks, "Arts::Synth_SHELVE_CUTOFF");
    ArtsMultiTrackSink   arts_sink(sink);

    // connect them
    filter.setValue("frequency", m_frequency);
    filter.connectInput(arts_source, "source",  "invalue");
    filter.connectOutput(arts_sink,  "sink",    "outvalue");

    // start all
    arts_source.start();
    filter.start();
    arts_sink.start();

    // transport the samples
    while (!m_stop && !(arts_source.done())) {
	arts_sink.goOn();
    }

    // shutdown
    filter.stop();
    arts_sink.stop();
    arts_source.stop();

    dispatcher->unlock();

    close();
}

//***************************************************************************
int LowPassPlugin::stop()
{
    m_stop = true;
    return KwavePlugin::stop();
}

//***************************************************************************
//***************************************************************************
