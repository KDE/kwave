/***************************************************************************
   PitchShiftPlugin.cpp  -  plugin for modifying the "pitch_shift"
                             -------------------
    begin                : Sun Mar 23 2003
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

#include "PitchShiftPlugin.h"
#include "PitchShiftDialog.h"

KWAVE_PLUGIN(PitchShiftPlugin,"pitch_shift","Thomas Eschenbacher");

//***************************************************************************
PitchShiftPlugin::PitchShiftPlugin(PluginContext &context)
    :KwavePlugin(context), m_params(), m_speed(1.0),
     m_frequency(5.0), m_percentage_mode(false),
     m_stop(false), m_listen(false)
{
}

//***************************************************************************
PitchShiftPlugin::~PitchShiftPlugin()
{
}

//***************************************************************************
int PitchShiftPlugin::interpreteParameters(QStringList &params)
{
    bool ok;
    QString param;

    // evaluate the parameter list
    if (params.count() != 3) return -EINVAL;

    param = params[0];
    m_speed = param.toDouble(&ok);
    ASSERT(ok);
    if (!ok) return -EINVAL;

    param = params[1];
    m_frequency = param.toDouble(&ok);
    ASSERT(ok);
    if (!ok) return -EINVAL;

    param = params[2];
    m_percentage_mode = (param.toUInt(&ok));
    ASSERT(ok);
    if (!ok) return -EINVAL;

    // all parameters accepted
    m_params = params;
    
    return 0;
}

//***************************************************************************
QStringList *PitchShiftPlugin::setup(QStringList &previous_params)
{
    // try to interprete the previous parameters
    interpreteParameters(previous_params);

    // create the setup dialog
    PitchShiftDialog *dialog = new PitchShiftDialog(parentWidget());
    ASSERT(dialog);
    if (!dialog) return 0;

    if (!m_params.isEmpty()) dialog->setParams(m_params);

    // connect the signals for pre-listen mode
    // ###
    
    QStringList *list = new QStringList();
    ASSERT(list);
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
void PitchShiftPlugin::run(QStringList params)
{
    unsigned int first, last;

    Arts::Dispatcher *dispatcher = manager().artsDispatcher();
    dispatcher->lock();
    ASSERT(dispatcher);
    if (!dispatcher) close();

    UndoTransactionGuard undo_guard(*this, i18n("pitch shift"));
    m_stop = false;

    interpreteParameters(params);

    MultiTrackReader source;
    MultiTrackWriter sink;

    selection(&first, &last, true);
    manager().openMultiTrackReader(source, selectedTracks(), first, last);

    if (m_listen) {
	// pre-listen mode
	// manager().openMultiTrackPlayback(sink, selectedTracks());
    } else {
	// normal mode
	manager().openMultiTrackWriter(sink, selectedTracks(), Overwrite,
	    first, last);
    }

    // create all objects
    ArtsMultiTrackSource arts_source(source);

    unsigned int tracks = selectedTracks().count();
    ArtsNativeMultiTrackFilter pitch(tracks, "Arts::Synth_PITCH_SHIFT");
    ArtsMultiTrackSink   arts_sink(sink);

    pitch.setAttribute("frequency", m_frequency);
    pitch.setAttribute("speed", m_speed);
    
    // connect them
    pitch.connectInput(arts_source, "source",   "invalue");
    pitch.connectOutput(arts_sink,  "sink",    "outvalue");

    // start all
    arts_source.start();
    pitch.start();
    arts_sink.start();

    // transport the samples
    while (!m_stop && (!arts_source.done() || m_listen)) {
	arts_sink.goOn();
	
	if (m_listen && arts_source.done()) {
	    // start the next loop
	    source.reset();
	    arts_source.start();
	}
    }

    // shutdown
    pitch.stop();
    arts_sink.stop();
    arts_source.stop();

    dispatcher->unlock();

    close();
}

//***************************************************************************
int PitchShiftPlugin::stop()
{
    m_stop = true;
    return KwavePlugin::stop();
}

//***************************************************************************
//***************************************************************************
