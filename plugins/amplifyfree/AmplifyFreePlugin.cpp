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
#include "errno.h"

#include <qarray.h> // ###
#include <qstringlist.h>
#include <klocale.h>

#include <arts/artsflow.h>
#include <arts/connect.h>

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

//***************************************************************************
void AmplifyFreePlugin::run(QStringList)
{
    UndoTransactionGuard undo_guard(*this, i18n("amplify free"));
    m_stop = false;
    QArray<sample_t> m_zeroes;
    unsigned int ZERO_COUNT = 65536;

    MultiTrackWriter writers;
    manager().openMultiTrackWriter(writers, Overwrite);

    // break if aborted
    if (writers.isEmpty()) return;

    unsigned int first = writers[0]->first();
    unsigned int last  = writers[0]->last();
    unsigned int count = writers.count();

    // get the buffer with zeroes for faster filling
    if (m_zeroes.count() != ZERO_COUNT) {
	m_zeroes.resize(ZERO_COUNT);
	m_zeroes.fill(0);
    }

    // loop over the sample range
    while ((first <= last) && (!m_stop)) {
	unsigned int rest = last - first + 1;
	if (rest < m_zeroes.count()) m_zeroes.resize(rest);
	
	// loop over all writers
	unsigned int w;
	for (w=0; w < count; w++) {
	    *(writers[w]) << m_zeroes;
	}
	
	first += m_zeroes.count();
    }

    Arts::Dispatcher dispatcher;

//    Synth_FREQUENCY freq1,freq2;   // object creation
//    Synth_WAVE_SIN  sin1,sin2;
//    Synth_PLAY      play;
//
//    setValue(freq1, 440.0);	  // set frequencies
//    setValue(freq2, 880.0);
//
//    connect(freq1, sin1);	  // object connection
//    connect(freq2, sin2);
//    connect(sin1, play, "invalue_left");
//    connect(sin2, play, "invalue_right");
//
//    freq1.start(); freq2.start();  // start&go
//    sin1.start(); sin2.start();
//    play.start();
//    dispatcher.run();



    writers.setAutoDelete(true);
    writers.clear();

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
