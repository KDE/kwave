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

#include "libkwave/ArtsMultiTrackSink.h"
#include "libkwave/ArtsMultiTrackFilter.h"
#include "libkwave/ArtsMultiIO.h"
#include "libkwave/ArtsMultiSource.h"
#include "libkwave/ArtsMultiSink.h"

#include "config.h"
#include <errno.h>
#include <string.h> // ###
#include <stdio.h> // ###

#include <qarray.h> // ###
#include <qstringlist.h>
#include <klocale.h>

#include <arts/artsflow.h>
#include <arts/connect.h>
#include <arts/objectmanager.h> // ###
#include <arts/stdsynthmodule.h> // ###

#include "libkwave/InsertMode.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/Parser.h"
#include "libkwave/Sample.h" // ###
#include "libkwave/SampleReader.h" // ###
#include "libkwave/SampleWriter.h" // ###

#include "libkwave/ArtsSampleSource_impl.h"
#include "libkwave/ArtsSampleSink_impl.h"

#include "kwave/PluginManager.h"
#include "kwave/UndoTransactionGuard.h"

#include "AmplifyFreePlugin.h"
#include "AmplifyFreeDialog.h"

KWAVE_PLUGIN(AmplifyFreePlugin,"amplifyfree","Thomas Eschenbacher");

//***************************************************************************
AmplifyFreePlugin::AmplifyFreePlugin(PluginContext &context)
    :KwavePlugin(context), m_params(), m_stop(false)
{
}

//***************************************************************************
AmplifyFreePlugin::~AmplifyFreePlugin()
{
}

//***************************************************************************
int AmplifyFreePlugin::interpreteParameters(QStringList &params)
{
    // all params are curve params
    m_params = params;
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
//***************************************************************************

#include "PassThruFilter.h"

class PassThruFilter_impl
    :virtual public PassThruFilter_skel,
     virtual public Arts::StdSynthModule
{
public:
    void calculateBlock(unsigned long samples)
    {
//	debug("PassThruFilter_impl::calculateBlock(%lu)", samples);
	unsigned long x = 0;
	
	while (x < samples) {
	    if (m_pos < 30000) {
		output[x] = input[x] * ((double)m_pos / 30000.0);
	    } else output[x] = input[x];
	
//	    } else if (m_pos & 1) output[x] = 1.0; // 1 << 23;
//	    else output[x] = -1.0; // (1 << 23);
	
	    m_pos++;
	    x++;
	}
    };

    virtual void streamInit() {
	m_pos = 0;
    };

protected:
    unsigned long m_pos;
};

//***************************************************************************
//***************************************************************************

typedef ArtsMultiIO< ArtsMultiSource, ArtsSampleSource, \
    ArtsSampleSource_impl, MultiTrackReader > ArtsMultiTrackSource_base;

class ArtsMultiTrackSource
    :public ArtsMultiTrackSource_base
{
public:
    ArtsMultiTrackSource(MultiTrackReader &reader)
	:ArtsMultiTrackSource_base(reader) {};

    virtual ~ArtsMultiTrackSource() {};

    virtual bool done() {
	unsigned int t;
	for (t=0; t < m_count; ++t) {
	    if (!m_ios[t]->done()) return false;
	}
	debug("ArtsMultiTrackSource()::done() -> reached eof !");
	return true;
    };
};

//***************************************************************************


//***************************************************************************
//***************************************************************************

//***************************************************************************
void AmplifyFreePlugin::run(QStringList)
{
    unsigned int first, last;

    UndoTransactionGuard undo_guard(*this, i18n("amplify free"));
    m_stop = false;

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

    unsigned int tracks = selectedTracks().count();
    ArtsMultiTrackFilter<PassThruFilter, PassThruFilter_impl> filter1(tracks);
    ArtsMultiTrackFilter<PassThruFilter, PassThruFilter_impl> filter2(tracks);

    ArtsMultiTrackSource arts_source(source);
    ArtsMultiTrackSink   arts_sink(sink);

    filter1.connectInput( arts_source, "source");
    filter2.connectInput( filter1,     "output");
    filter2.connectOutput(arts_sink,   "sink");

    arts_source.start();
    arts_sink.start();
    filter1.start();
    filter2.start();

    debug("AmplifyFreePlugin: filter started...");
    while (!m_stop && !(arts_source.done())) {
	arts_sink.goOn();
    }
    debug("AmplifyFreePlugin: filter done.");

    filter2.stop();
    filter1.stop();
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
