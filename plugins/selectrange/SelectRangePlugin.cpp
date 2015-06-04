/***************************************************************************
  SelectRangePlugin.cpp  -  Plugin for selecting a range of samples
                             -------------------
    begin                : Sat Jun 15 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
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

#include <QtCore/QString>
#include <KI18n/KLocalizedString>

#include "libkwave/Plugin.h"
#include "libkwave/PluginManager.h"
#include "libkwave/String.h"
#include "libkwave/SignalManager.h"
#include "libkwave/undo/UndoTransactionGuard.h"

#include "SelectRangePlugin.h"
#include "SelectRangeDialog.h"

KWAVE_PLUGIN(Kwave::SelectRangePlugin, "selectrange", "2.3",
             I18N_NOOP("Select Range"), "Thomas Eschenbacher");

//***************************************************************************
Kwave::SelectRangePlugin::SelectRangePlugin(
    Kwave::PluginManager &plugin_manager)
    :Kwave::Plugin(plugin_manager),
     m_start_mode(Kwave::SelectTimeWidget::bySamples),
     m_range_mode(Kwave::SelectTimeWidget::bySamples),
     m_start(0), m_range(0)
{
}

//***************************************************************************
Kwave::SelectRangePlugin::~SelectRangePlugin()
{
}

//***************************************************************************
QStringList *Kwave::SelectRangePlugin::setup(QStringList &previous_params)
{
    // try to interpret the previous parameters
    interpreteParameters(previous_params);

    // create the setup dialog
    double rate = signalRate();
    sample_index_t offset = manager().selectionStart();
    sample_index_t length = signalLength();

    Kwave::SelectRangeDialog *dialog =
	new Kwave::SelectRangeDialog(parentWidget(),
	    m_start_mode, m_range_mode, m_range, rate, offset, length);
    Q_ASSERT(dialog);
    if (!dialog) return 0;

    QStringList *list = new QStringList();
    Q_ASSERT(list);
    if (list && dialog->exec()) {
	// user has pressed "OK"
	*list << QString::number(dialog->startMode());
	*list << QString::number(dialog->rangeMode());
	*list << QString::number(dialog->start());
	*list << QString::number(dialog->range());

	emitCommand(_("plugin:execute(selectrange,") +
	    QString::number(dialog->startMode()) + _(",") +
	    QString::number(dialog->rangeMode()) + _(",") +
	    QString::number(dialog->start()) + _(",") +
	    QString::number(dialog->range())+
	    _(")")
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
int Kwave::SelectRangePlugin::start(QStringList &params)
{
    // interprete the parameters
    int result = interpreteParameters(params);
    if (result) return result;

    // get current offset of the signal
    sample_index_t offset = Kwave::SelectTimeWidget::timeToSamples(
	m_start_mode, m_start, signalRate(), signalLength());

    // transform into offset and length [samples]
    sample_index_t length = Kwave::SelectTimeWidget::timeToSamples(
	m_range_mode, m_range, signalRate(), signalLength());

    // limit selection to end of signal
    if ((offset + length) >= signalLength())
	length = signalLength() - offset;

    // change the selection through the signal manager
    {
	Kwave::UndoTransactionGuard undo_guard(*this, i18n("Select Range"));
	selectRange(offset, length);
    }

    return result;
}

//***************************************************************************
int Kwave::SelectRangePlugin::interpreteParameters(QStringList &params)
{
    bool ok;
    QString param;
    int mode;

    // evaluate the parameter list
    if (params.count() != 4) {
	return -EINVAL;
    }

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
    m_start_mode = static_cast<Kwave::SelectTimeWidget::Mode>(mode);


    // selection mode
    param = params[1];
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
    m_range_mode = static_cast<Kwave::SelectTimeWidget::Mode>(mode);

    // offset in ms, samples or percent
    param = params[2];
    m_start = param.toUInt(&ok);
    if (!ok) return -EINVAL;

    // range in ms, samples or percent
    param = params[3];
    m_range = param.toUInt(&ok);
    if (!ok) return -EINVAL;

    return 0;
}

//***************************************************************************
#include "SelectRangePlugin.moc"
//***************************************************************************
//***************************************************************************
