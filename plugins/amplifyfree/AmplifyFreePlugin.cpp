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

#include <qstringlist.h>
#include <klocale.h>

#include "libkwave/Parser.h"

#include "libkwave/CurveStreamAdapter_impl.h"
#include "libkwave/ArtsMultiTrackSink.h"
#include "libkwave/ArtsMultiTrackSource.h"
#include "libkwave/ArtsKwaveMultiTrackFilter.h"
#include "libkwave/ArtsNativeMultiTrackFilter.h"

#include "kwave/PluginManager.h"
#include "kwave/UndoTransactionGuard.h"

#include "AmplifyFreePlugin.h"
#include "AmplifyFreeDialog.h"

KWAVE_PLUGIN(AmplifyFreePlugin,"amplifyfree","Thomas Eschenbacher");

//***************************************************************************
AmplifyFreePlugin::AmplifyFreePlugin(PluginContext &context)
    :KwavePlugin(context), m_params(), m_curve(), m_stop(false)
{
}

//***************************************************************************
AmplifyFreePlugin::~AmplifyFreePlugin()
{
}

//***************************************************************************
int AmplifyFreePlugin::interpreteParameters(QStringList &params)
{
    // store last parameters
    m_params = params;

    // convert string list into command again...
    QString cmd;
    cmd = "curve(";
    for (unsigned int i=0; i < params.count(); ++i) {
	cmd += params[i];
	if (i+1 < params.count()) cmd += ",";
    }
    cmd += ")";

    // and initialize our curve with it
    m_curve.fromCommand(cmd);

    return 0;
}

//***************************************************************************
QStringList *AmplifyFreePlugin::setup(QStringList &previous_params)
{
    // try to interprete the previous parameters
    interpreteParameters(previous_params);

    // create the setup dialog
    AmplifyFreeDialog *dialog = new AmplifyFreeDialog(parentWidget());
    ASSERT(dialog);
    if (!dialog) return 0;

    if (!m_params.isEmpty()) dialog->setParams(m_params);

    QStringList *list = new QStringList();
    ASSERT(list);
    if (list && dialog->exec()) {
	// user has pressed "OK"
	QString cmd = dialog->getCommand();
	Parser p(cmd);
	while (!p.isDone()) *list << p.nextParam();
	emitCommand(cmd);
    } else {
	// user pressed "Cancel"
	if (list) delete list;
	list = 0;
    }

    if (dialog) delete dialog;
    return list;
};

//***************************************************************************
void AmplifyFreePlugin::run(QStringList params)
{
    unsigned int first, last;

    UndoTransactionGuard undo_guard(*this, i18n("amplify free"));
    m_stop = false;

    interpreteParameters(params);

    MultiTrackReader source;
    MultiTrackWriter sink;

    unsigned int input_length = selection(&first, &last);
    if (first == last) {
	input_length = signalLength()-1;
	first = 0;
	last = input_length-1;
    }

    openMultiTrackReader(source, selectedTracks(), first, last);
    manager().openMultiTrackWriter(sink, selectedTracks(), Overwrite, first, last);

    static Arts::Dispatcher dispatcher;
    dispatcher.lock();

    // create all objects
    ArtsMultiTrackSource arts_source(source);

    CurveStreamAdapter curve_adapter = CurveStreamAdapter::_from_base(
	new CurveStreamAdapter_impl(m_curve, input_length)
    );

    unsigned int tracks = selectedTracks().count();
    ArtsNativeMultiTrackFilter mul(tracks, "Arts::Synth_MUL");

    ArtsMultiTrackSink   arts_sink(sink);

    // connect them
    mul.connectInput(arts_source,   "source", "invalue1");
    mul.connectInput(curve_adapter, "output", "invalue2");
    mul.connectOutput(arts_sink,    "sink",   "outvalue");

    // start all
    arts_source.start();
    mul.start();
    curve_adapter.start();
    arts_sink.start();

    // transport the samples
    debug("AmplifyFreePlugin: filter started...");
    while (!m_stop && !(arts_source.done())) {
	arts_sink.goOn();
    }
    debug("AmplifyFreePlugin: filter done.");

    // shutdown
    curve_adapter.stop();
    mul.stop();
    arts_sink.stop();
    arts_source.stop();

    dispatcher.unlock();

    close();
}

//***************************************************************************
int AmplifyFreePlugin::stop()
{
    m_stop = true;
    return KwavePlugin::stop();
}

//***************************************************************************
//***************************************************************************
