/***************************************************************************
         GotoPlugin.cpp  -  Plugin for moving the view to a certain position
                             -------------------
    begin                : Sat Dec 06 2008
    copyright            : (C) 2008 by Thomas Eschenbacher
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
#include <math.h>

#include <QString>
#include <klocale.h>

#include "libkwave/KwavePlugin.h"
#include "libkwave/PluginManager.h"
#include "libkwave/SignalManager.h"

#include "GotoPlugin.h"
#include "GotoDialog.h"

KWAVE_PLUGIN(GotoPlugin,"goto","2.1","Thomas Eschenbacher");

//***************************************************************************
GotoPlugin::GotoPlugin(const PluginContext &c)
    :Kwave::Plugin(c), m_mode(SelectTimeWidget::bySamples), m_position(0)
{
     i18n("Goto");
}

//***************************************************************************
GotoPlugin::~GotoPlugin()
{
}

//***************************************************************************
QStringList *GotoPlugin::setup(QStringList &previous_params)
{
    // try to interprete the previous parameters
    interpreteParameters(previous_params);

    // create the setup dialog
    double rate = signalRate();
    unsigned int length = signalLength();

    GotoDialog *dialog = new GotoDialog(parentWidget(),
        m_mode, m_position, rate, length);
    Q_ASSERT(dialog);
    if (!dialog) return 0;

    QStringList *list = new QStringList();
    Q_ASSERT(list);
    if (list && dialog->exec()) {
	// user has pressed "OK"
	*list << QString::number(dialog->mode());
	*list << QString::number(dialog->pos());

	emitCommand("plugin:execute(goto," +
	    QString::number(dialog->mode()) + "," +
	    QString::number(dialog->pos()) +
	    ")"
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
int GotoPlugin::start(QStringList &params)
{
    // interprete the parameters
    int result = interpreteParameters(params);
    if (result) return result;

    // get current offset of the signal
    unsigned int offset = SelectTimeWidget::timeToSamples(
	m_mode, m_position, signalRate(), signalLength());

    // change the selection through the signal manager
    QString command = "nomacro:goto(%1)";
    emitCommand(command.arg(offset));

    return result;
}

//***************************************************************************
int GotoPlugin::interpreteParameters(QStringList &params)
{
    bool ok;
    QString param;
    int mode;

    // evaluate the parameter list
    if (params.count() != 2) {
	return -EINVAL;
    }

    // selection mode for start
    param = params[0];
    mode = param.toInt(&ok);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;
    Q_ASSERT((mode == static_cast<int>(SelectTimeWidget::byTime)) ||
           (mode == static_cast<int>(SelectTimeWidget::bySamples)) ||
           (mode == static_cast<int>(SelectTimeWidget::byPercents)));
    if ((mode != static_cast<int>(SelectTimeWidget::byTime)) &&
        (mode != static_cast<int>(SelectTimeWidget::bySamples)) &&
        (mode != static_cast<int>(SelectTimeWidget::byPercents)))
    {
	return -EINVAL;
    }
    m_mode = static_cast<SelectTimeWidget::Mode>(mode);

    // position in ms, samples or percent
    param = params[1];
    m_position = param.toUInt(&ok);
    if (!ok) return -EINVAL;

    return 0;
}

//***************************************************************************
#include "GotoPlugin.moc"
//***************************************************************************
//***************************************************************************
