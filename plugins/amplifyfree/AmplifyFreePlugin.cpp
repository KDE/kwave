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
void AmplifyFreePlugin::run(QStringList)
{
//    unsigned int first, last;
//
////    UndoTransactionGuard undo_guard(*this, i18n("amplify free"));
//    m_stop = false;
//
//    MultiTrackReader source;
//    MultiTrackWriter sink;
//
//    unsigned int input_length = selection(&first, &last);
//    if (first == last) {
//	input_length = signalLength()-1;
//	first = 0;
//	last = input_length-1;
//    }
//
//    openMultiTrackReader(source, selectedTracks(), first, last);
//    manager().openMultiTrackWriter(sink, Overwrite);
//
//    fprintf(stderr, "---%s:%d---\n",__FILE__, __LINE__);
//    Arts::Dispatcher dispatcher;
//    dispatcher.lock();
//
//    fprintf(stderr, "---%s:%d---\n",__FILE__, __LINE__);
////    for (track=0; track < tracks; track++) {
//    SampleReader *reader = source.at(0);
//    SampleWriter *writer = sink.at(0);
//
//    fprintf(stderr, "---%s:%d---\n",__FILE__, __LINE__);
//    ArtsSampleSource src_adapter = ArtsSampleSource::_from_base(
//	new ArtsSampleSource_impl(reader));
//
//    fprintf(stderr, "---%s:%d---\n",__FILE__, __LINE__);
//    ArtsSampleSink dst_adapter = ArtsSampleSink::_from_base(
//	new ArtsSampleSink_impl(writer));
//
//    fprintf(stderr, "---%s:%d\n",__FILE__, __LINE__);
//    Arts::connect(src_adapter, "source", dst_adapter, "sink");
//
//    fprintf(stderr, "---%s:%d\n",__FILE__, __LINE__);
//    src_adapter.start();
//    dst_adapter.start();
//
////    dispatcher.run();
//    fprintf(stderr, "---%s:%d\n",__FILE__, __LINE__);
//    while (!m_stop && !src_adapter.done()) {
//	dst_adapter.goOn();
//    }
//
//    dst_adapter.stop();
//    src_adapter.stop();
//
//    sink.setAutoDelete(true);
//    sink.clear();
//
//    source.setAutoDelete(true);
//    source.clear();
//
//    fprintf(stderr, "---%s:%d---\n",__FILE__, __LINE__);
//    dispatcher.unlock();
//
//    close();
}

//***************************************************************************
int AmplifyFreePlugin::stop()
{
    m_stop = true;
    return KwavePlugin::stop();
}

//***************************************************************************
//***************************************************************************
