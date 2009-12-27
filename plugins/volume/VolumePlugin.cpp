/***************************************************************************
       VolumePlugin.cpp  -  Plugin for adjusting a signal's volume
                             -------------------
    begin                : Sun Oct 27 2002
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

#include <QStringList>

#include <klocale.h>

#include "libkwave/KwaveConnect.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/KwaveMultiTrackSource.h"
#include "libkwave/Parser.h"
#include "libkwave/PluginManager.h"
#include "libkwave/SignalManager.h"
#include "libkwave/modules/KwaveMul.h"
#include "libkwave/undo/UndoTransactionGuard.h"

#include "libgui/OverViewCache.h"

#include "VolumePlugin.h"
#include "VolumeDialog.h"

KWAVE_PLUGIN(VolumePlugin,"volume","2.1","Thomas Eschenbacher");

//***************************************************************************
VolumePlugin::VolumePlugin(const PluginContext &context)
    :Kwave::Plugin(context), m_params(), m_factor(1.0), m_mode(0)
{
}

//***************************************************************************
VolumePlugin::~VolumePlugin()
{
}

//***************************************************************************
int VolumePlugin::interpreteParameters(QStringList &params)
{
    bool ok;
    QString param;

    // evaluate the parameter list
    if (params.count() != 2) return -EINVAL;

    param = params[0];
    m_factor = param.toDouble(&ok);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    param = params[1];
    m_mode = param.toUInt(&ok);
    Q_ASSERT(ok);
    if (!ok || (m_mode > 2)) return -EINVAL;

    // all parameters accepted
    m_params = params;

    return 0;
}

//***************************************************************************
QStringList *VolumePlugin::setup(QStringList &previous_params)
{
    // try to interprete the previous parameters
    interpreteParameters(previous_params);

    // initialize the overview cache
    SignalManager &mgr = manager().signalManager();
    QList<unsigned int> tracks;
    unsigned int first, last;
    unsigned int length = selection(&tracks, &first, &last, true);
    OverViewCache *overview_cache = new OverViewCache(mgr,
        first, length, tracks.isEmpty() ? 0 : &tracks);
    Q_ASSERT(overview_cache);

    // create the setup dialog
    VolumeDialog *dialog = new VolumeDialog(parentWidget(), overview_cache);
    if (!dialog) {
	if (overview_cache) delete overview_cache;
	return 0;
    }

    if (!m_params.isEmpty()) dialog->setParams(m_params);

    // execute the dialog
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

    if (dialog)         delete dialog;
    if (overview_cache) delete overview_cache;

    return list;
}

//***************************************************************************
void VolumePlugin::run(QStringList params)
{
    QList<unsigned int> tracks;
    unsigned int first, last;

    interpreteParameters(params);
    if (!selection(&tracks, &first, &last, true) || tracks.isEmpty())
	return;

    UndoTransactionGuard undo_guard(*this, i18n("Volume"));

    // create all objects
    MultiTrackReader source(Kwave::SinglePassForward,
	signalManager(), selectedTracks(), first, last);
    Kwave::MultiTrackWriter sink(signalManager(), tracks, Overwrite,
	first, last);
    Kwave::MultiTrackSource<Kwave::Mul, true> mul(tracks.count());

    // connect the progress dialog
    connect(&source, SIGNAL(progress(unsigned int)),
	    this,  SLOT(updateProgress(unsigned int)),
	     Qt::BlockingQueuedConnection);

    // connect them
    Kwave::connect(
	source, SIGNAL(output(Kwave::SampleArray)),
	mul,    SLOT(input_a(Kwave::SampleArray)));

    mul.setAttribute(SLOT(set_b(const QVariant)),
                     QVariant(m_factor));
    Kwave::connect(
	mul,    SIGNAL(output(Kwave::SampleArray)),
	sink,   SLOT(input(Kwave::SampleArray)));

    // transport the samples
    qDebug("VolumePlugin: filter started...");
    while (!shouldStop() && !source.done()) {
	source.goOn();
	mul.goOn();
    }
    qDebug("VolumePlugin: filter done.");
}

//***************************************************************************
#include "VolumePlugin.moc"
//***************************************************************************
//***************************************************************************
