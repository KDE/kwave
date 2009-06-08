/***************************************************************************
    NewSignalPlugin.cpp  -  plugin for creating a new signal
                             -------------------
    begin                : Wed Jul 18 2001
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

#include <QStringList>

#include "NewSignalPlugin.h"
#include "NewSignalDialog.h"

KWAVE_PLUGIN(NewSignalPlugin,"newsignal","2.1","Thomas Eschenbacher");

//***************************************************************************
NewSignalPlugin::NewSignalPlugin(const PluginContext &context)
    :Kwave::Plugin(context), m_samples(2646000), m_rate(44100),
    m_bits(16), m_tracks(2), m_bytime(true)
{
    i18n("newsignal");
}

//***************************************************************************
NewSignalPlugin::~NewSignalPlugin()
{
}

//***************************************************************************
int NewSignalPlugin::interpreteParameters(QStringList &params)
{
    bool ok;
    QString param;

    // evaluate the parameter list
    if (params.count() != 5) return -EINVAL;

    param = params[0];
    m_samples = param.toUInt(&ok);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    param = params[1];
    m_rate = static_cast<unsigned int>(param.toDouble(&ok));
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    param = params[2];
    m_bits = param.toUInt(&ok);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    param = params[3];
    m_tracks = param.toUInt(&ok);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    param = params[4];
    m_bytime = (param.toUInt(&ok) != 0);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    return 0;
}

//***************************************************************************
QStringList *NewSignalPlugin::setup(QStringList &previous_params)
{
    // try to interprete the previous parameters
    interpreteParameters(previous_params);

    // create the setup dialog
    NewSignalDialog *dialog = new NewSignalDialog(parentWidget(),
        m_samples, m_rate, m_bits, m_tracks, m_bytime);
    Q_ASSERT(dialog);
    if (!dialog) return 0;

    QStringList *list = new QStringList();
    Q_ASSERT(list);
    if (list && dialog->exec()) {
	// user has pressed "OK"
	*list << QString::number(dialog->samples());
	*list << QString::number(dialog->rate());
	*list << QString::number(dialog->bitsPerSample());
	*list << QString::number(dialog->tracks());
	*list << (dialog->byTime() ? "1" : "0");

	emitCommand("newsignal("+
	    QString::number(dialog->samples())+","+
	    QString::number(dialog->rate())+","+
	    QString::number(dialog->bitsPerSample())+","+
	    QString::number(dialog->tracks())+")"
	);
    } else {
	// user pressed "Cancel"
	if (list) delete list;
	list = 0;
    }

    if (dialog) delete dialog;
    return list;
}

//***************************************************************************
#include "NewSignalPlugin.moc"
//***************************************************************************
//***************************************************************************
