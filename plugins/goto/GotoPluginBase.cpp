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
#include <new>

#include <QDialog>
#include <QPointer>
#include <QString>

#include <KLocalizedString>

#include "libkwave/Plugin.h"
#include "libkwave/PluginManager.h"
#include "libkwave/SignalManager.h"
#include "libkwave/String.h"

#include "GotoDialog.h"
#include "GotoPluginBase.h"

//***************************************************************************
Kwave::GotoPluginBase::GotoPluginBase(QObject *parent,
                                      const QVariantList &args)
    :Kwave::Plugin(parent, args),
     m_mode(Kwave::SelectTimeWidget::bySamples), m_position(0)
{
}

//***************************************************************************
Kwave::GotoPluginBase::~GotoPluginBase()
{
}

//***************************************************************************
QStringList *Kwave::GotoPluginBase::setup(QStringList &previous_params)
{
    // try to interpret the previous parameters
    interpreteParameters(previous_params);

    // create the setup dialog
    double rate = signalRate();
    sample_index_t length = signalLength();

    // get the name of the help section
    QString help_section = _("plugin_sect_") + command();

    QPointer<Kwave::GotoDialog> dialog = new(std::nothrow)
	Kwave::GotoDialog(parentWidget(),
	    m_mode, m_position, rate, length, help_section
	);
    Q_ASSERT(dialog);
    if (!dialog) return Q_NULLPTR;

    // set the title of the dialog, depending on the derived class
    dialog->setWindowTitle(title());

    QStringList *list = new(std::nothrow) QStringList();
    Q_ASSERT(list);
    if (list && dialog->exec() && dialog) {
	// user has pressed "OK"
	*list << QString::number(dialog->mode());
	*list << QString::number(dialog->pos());

	emitCommand(_("plugin:execute(") + command() +
	    _(",") +  QString::number(dialog->mode()) +
	    _(",") +  QString::number(dialog->pos()) +
	    _(")")
	);
    } else {
	// user pressed "Cancel"
	if (list) delete list;
        list = Q_NULLPTR;
    }

    if (dialog) delete dialog;
    return list;
}

//***************************************************************************
int Kwave::GotoPluginBase::start(QStringList &params)
{
    // interprete the parameters
    int result = interpreteParameters(params);
    if (result) return result;

    // get current offset of the signal
    sample_index_t offset = Kwave::SelectTimeWidget::timeToSamples(
	m_mode, m_position, signalRate(), signalLength());

    // change the selection through the signal manager
    QString cmd = _("nomacro:") + command() + _("(%1)");
    emitCommand(cmd.arg(offset));

    return result;
}

//***************************************************************************
int Kwave::GotoPluginBase::interpreteParameters(QStringList &params)
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
//***************************************************************************
