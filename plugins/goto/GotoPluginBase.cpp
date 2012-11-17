/***************************************************************************
     GotoPluginBase.cpp  -  base class for the goto plugin
                             -------------------
    begin                : Thu May 12 2011
    copyright            : (C) 2011 by Thomas Eschenbacher
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

#include <QDialog>
#include <QString>
#include <klocale.h>

#include "libkwave/Plugin.h"
#include "libkwave/PluginManager.h"
#include "libkwave/SignalManager.h"

#include "GotoPluginBase.h"
#include "GotoDialog.h"

//***************************************************************************
GotoPluginBase::GotoPluginBase(const Kwave::PluginContext &c)
    :Kwave::Plugin(c), m_mode(Kwave::SelectTimeWidget::bySamples), m_position(0)
{
}

//***************************************************************************
GotoPluginBase::~GotoPluginBase()
{
}

//***************************************************************************
QStringList *GotoPluginBase::setup(QStringList &previous_params)
{
    // try to interprete the previous parameters
    interpreteParameters(previous_params);

    // create the setup dialog
    double rate = signalRate();
    sample_index_t length = signalLength();

    GotoDialog *dialog = new GotoDialog(parentWidget(),
        m_mode, m_position, rate, length);
    Q_ASSERT(dialog);
    if (!dialog) return 0;

    // set the title of the dialog, depending on the derived class
    dialog->setWindowTitle(title());

    QStringList *list = new QStringList();
    Q_ASSERT(list);
    if (list && dialog->exec()) {
	// user has pressed "OK"
	*list << QString::number(dialog->mode());
	*list << QString::number(dialog->pos());

	emitCommand("plugin:execute(" + command() + "," +
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
int GotoPluginBase::start(QStringList &params)
{
    // interprete the parameters
    int result = interpreteParameters(params);
    if (result) return result;

    // get current offset of the signal
    sample_index_t offset = Kwave::SelectTimeWidget::timeToSamples(
	m_mode, m_position, signalRate(), signalLength());

    // change the selection through the signal manager
    QString cmd = "nomacro:" + command() + "(%1)";
    emitCommand(cmd.arg(offset));

    return result;
}

//***************************************************************************
int GotoPluginBase::interpreteParameters(QStringList &params)
{
    bool ok;
    QString param;
    int mode;

    // evaluate the parameter list
    if (params.count() != 2)
	return -EINVAL;

    // selection mode for start
    param = params[0];
    mode = param.toInt(&ok);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;
    Q_ASSERT(
        (mode == static_cast<int>(Kwave::SelectTimeWidget::byTime)) ||
        (mode == static_cast<int>(Kwave::SelectTimeWidget::bySamples)) ||
        (mode == static_cast<int>(Kwave::SelectTimeWidget::byPercents))
    );
    if ((mode != static_cast<int>(Kwave::SelectTimeWidget::byTime)) &&
        (mode != static_cast<int>(Kwave::SelectTimeWidget::bySamples)) &&
        (mode != static_cast<int>(Kwave::SelectTimeWidget::byPercents)))
    {
	return -EINVAL;
    }
    m_mode = static_cast<Kwave::SelectTimeWidget::Mode>(mode);

    // position in ms, samples or percent
    param = params[1];
    m_position = param.toUInt(&ok);
    if (!ok) return -EINVAL;

    return 0;
}

//***************************************************************************
#include "GotoPluginBase.moc"
//***************************************************************************
//***************************************************************************
