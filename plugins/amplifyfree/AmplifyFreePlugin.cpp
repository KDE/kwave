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
#include "libkwave/Parser.h"
#include "libkwave/Sample.h" // ###
#include "libkwave/SampleWriter.h" // ###

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

// example_add_impl.cc

using namespace Arts;

#include "kwave_sample_source.h"
#include <stdsynthmodule.h>
#include "libkwave/Sample.h" // ###
#include "libkwave/SampleReader.h" // ###

class Kwave_SampleSource_impl
              :public Kwave_SampleSource_skel, Arts::StdSynthModule
{
public:

     Kwave_SampleSource_impl()
	:Kwave_SampleSource_skel(), Arts::StdSynthModule(),
	source(0)
     { }

     Kwave_SampleSource_impl(SampleReader *src)
	:Kwave_SampleSource_skel(), Arts::StdSynthModule(),
	source(src)
     { }

     void calculateBlock(unsigned long samples)
     {
         unsigned long i;
         for(i=0;i < samples;i++)
             result[i] = (int)(double(i % 1024) * (double)((1 << 22) / 1024));
     }

protected:
    SampleReader *source;

};

REGISTER_IMPLEMENTATION(Kwave_SampleSource_impl);

//***************************************************************************
void AmplifyFreePlugin::run(QStringList)
{
//    UndoTransactionGuard undo_guard(*this, i18n("amplify free"));
    m_stop = false;
//    QArray<sample_t> m_zeroes;
//    unsigned int ZERO_COUNT = 65536;
//
//    MultiTrackWriter writers;
//    manager().openMultiTrackWriter(writers, Overwrite);
//
//    // break if aborted
//    if (writers.isEmpty()) return;
//
//    unsigned int first = writers[0]->first();
//    unsigned int last  = writers[0]->last();
//    unsigned int count = writers.count();
//
//    // get the buffer with zeroes for faster filling
//    if (m_zeroes.count() != ZERO_COUNT) {
//	m_zeroes.resize(ZERO_COUNT);
//	m_zeroes.fill(0);
//    }
//
//    // loop over the sample range
//    while ((first <= last) && (!m_stop)) {
//	unsigned int rest = last - first + 1;
//	if (rest < m_zeroes.count()) m_zeroes.resize(rest);
//	
//	// loop over all writers
//	unsigned int w;
//	for (w=0; w < count; w++) {
//	    *(writers[w]) << m_zeroes;
//	}
//	
//	first += m_zeroes.count();
//    }

    Arts::Dispatcher dispatcher;
    dispatcher.lock();

    Kwave_SampleSource adapter = Kwave_SampleSource::_from_base(
	new Kwave_SampleSource_impl((SampleReader*)0));

    fprintf(stderr, "---%s:%d---\n",__FILE__, __LINE__);

    Arts::Synth_PLAY      play;

//    setValue(freq1, 440.0);	  // set frequencies
//    Arts::connect(freq1, sin1);	  // object connection
//    Arts::connect(sin1, play, "invalue_left");
//    Arts::connect(sin2, play, "invalue_right");

    fprintf(stderr, "---%s:%d\n",__FILE__, __LINE__);
    Arts::connect(adapter, play, "invalue_left");
    fprintf(stderr, "---%s:%d\n",__FILE__, __LINE__);

    adapter.start();
    play.start();
    dispatcher.run();

    fprintf(stderr, "---%s:%d\n",__FILE__, __LINE__);

//    writers.setAutoDelete(true);
//    writers.clear();

	fprintf(stderr, "---%s:%d---\n",__FILE__, __LINE__);
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
