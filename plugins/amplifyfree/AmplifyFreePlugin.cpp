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

#include "libkwave/CurveStreamAdapter.h"
#include "libkwave/KwaveConnect.h"
#include "libkwave/KwaveMul.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/KwaveMultiTrackSource.h"
#include "libkwave/Parser.h"

#include "kwave/PluginManager.h"
#include "kwave/UndoTransactionGuard.h"

#include "AmplifyFreePlugin.h"
#include "AmplifyFreeDialog.h"

KWAVE_PLUGIN(AmplifyFreePlugin,"amplifyfree","Thomas Eschenbacher");

//***************************************************************************
AmplifyFreePlugin::AmplifyFreePlugin(const PluginContext &context)
    :KwavePlugin(context), m_params(), m_curve(), m_stop(false)
{
    i18n("amplifyfree");
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
    Q_ASSERT(dialog);
    if (!dialog) return 0;

    if (!m_params.isEmpty()) dialog->setParams(m_params);

    QStringList *list = new QStringList();
    Q_ASSERT(list);
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
}

//***************************************************************************
void AmplifyFreePlugin::run(QStringList params)
{
    unsigned int first, last;

    UndoTransactionGuard undo_guard(*this, i18n("amplify free"));
    m_stop = false;

    interpreteParameters(params);
    unsigned int input_length = selection(&first, &last, true);
    unsigned int tracks = selectedTracks().count();

    // create all objects
    MultiTrackReader source(signalManager(), selectedTracks(), first, last);
    Kwave::CurveStreamAdapter curve(m_curve, input_length);
    MultiTrackWriter sink(signalManager(), selectedTracks(), Overwrite,
	first, last);
    Kwave::MultiTrackSource<Kwave::Mul> mul(tracks, this, "AmplifyFree");
    for (unsigned int i=0; i < tracks; i++)
	mul.insert(i, new Kwave::Mul());

    // connect them
    bool ok = true;
    if (ok) ok = Kwave::connect(
	source, SIGNAL(output(Kwave::SampleArray &)),
	mul,    SLOT(input_a(Kwave::SampleArray &)));
    if (ok) ok = Kwave::connect(
	curve,  SIGNAL(output(Kwave::SampleArray &)),
	mul,    SLOT(input_b(Kwave::SampleArray &)));
    if (ok) ok = Kwave::connect(
	mul,    SIGNAL(output(Kwave::SampleArray &)),
	sink,   SLOT(input(Kwave::SampleArray &)));
    Q_ASSERT(ok);
    if (!ok) {
	close();
	return;
    }

    // transport the samples
    qDebug("AmplifyFreePlugin: filter started...");
    while (!m_stop && !source.done()) {
	source.goOn();
	curve.goOn();
	mul.goOn();
    }
    qDebug("AmplifyFreePlugin: filter done.");

    close();
}

//***************************************************************************
int AmplifyFreePlugin::stop()
{
    m_stop = true;
    return KwavePlugin::stop();
}

//***************************************************************************
#include "AmplifyFreePlugin.moc"
//***************************************************************************
//***************************************************************************
