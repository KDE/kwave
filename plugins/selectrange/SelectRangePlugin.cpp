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

#include <QString>
#include <klocale.h>

#include "kwave/PluginManager.h"
#include "kwave/SignalManager.h"
#include "kwave/UndoTransactionGuard.h"
#include "libkwave/KwavePlugin.h"
#include "SelectRangePlugin.h"
#include "SelectRangeDialog.h"

KWAVE_PLUGIN(SelectRangePlugin,"selectrange","Thomas Eschenbacher");

//***************************************************************************
SelectRangePlugin::SelectRangePlugin(const PluginContext &c)
    :KwavePlugin(c), m_start_mode(SelectTimeWidget::bySamples),
     m_range_mode(SelectTimeWidget::bySamples), m_range(0)
{
     i18n("selectrange");
}

//***************************************************************************
SelectRangePlugin::~SelectRangePlugin()
{
}

//***************************************************************************
QStringList *SelectRangePlugin::setup(QStringList &previous_params)
{
    // try to interprete the previous parameters
    interpreteParameters(previous_params);

    // create the setup dialog
    double rate = signalRate();
    unsigned int offset = 0;
    selection(&offset, 0, false);
    unsigned int length = signalLength();

    SelectRangeDialog *dialog = new SelectRangeDialog(parentWidget(),
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

	emitCommand("plugin:execute(selectrange,"+
	    QString::number(dialog->startMode())+","+
	    QString::number(dialog->rangeMode())+","+
	    QString::number(dialog->start())+","+
	    QString::number(dialog->range())+
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
int SelectRangePlugin::start(QStringList &params)
{
    // interprete the parameters
    int result = interpreteParameters(params);
    if (result) return result;

    // get current offset of the signal
    unsigned int offset = 0;
    switch (m_start_mode) {
	case SelectTimeWidget::byTime: {
	    // convert from ms to samples
	    double rate = signalRate();
	    offset = (unsigned int)rint(m_start / 1E3 * rate);
	    break;
	}
	case SelectTimeWidget::bySamples: {
	    // simple case -> already in samples
	    offset = (unsigned int)rint(m_start);
	    break;
	}
	case SelectTimeWidget::byPercents: {
	    // by percentage of whole signal
	    unsigned sig_length = signalLength();
	    offset = (unsigned int)rint((double)sig_length*(m_start/100.0));
	    break;
	}
    }

    // transform into offset and length [samples]
    unsigned int length = 0;
    switch (m_range_mode) {
	case SelectTimeWidget::byTime: {
	    // convert from ms to samples
	    double rate = signalRate();
	    length = (unsigned int)rint(m_range / 1E3 * rate);
	    break;
	}
	case SelectTimeWidget::bySamples: {
	    // simple case -> already in samples
	    length = (unsigned int)rint(m_range);
	    break;
	}
	case SelectTimeWidget::byPercents: {
	    // by percentage of whole signal
	    unsigned sig_length = signalLength();
	    length = (unsigned int)rint((double)sig_length*(m_range/100.0));
	    break;
	}
    }

    // limit selection to end of signal
    if ((offset + length) >= signalLength())
	length = signalLength() - offset;

    // change the selection through the signal manager
    {
	UndoTransactionGuard undo_guard(*this, i18n("select range"));
	selectRange(offset, length);
    }

    return result;
}

//***************************************************************************
int SelectRangePlugin::interpreteParameters(QStringList &params)
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
    Q_ASSERT((mode == (int)SelectTimeWidget::byTime) ||
           (mode == (int)SelectTimeWidget::bySamples) ||
           (mode == (int)SelectTimeWidget::byPercents));
    if ((mode != (int)SelectTimeWidget::byTime) &&
        (mode != (int)SelectTimeWidget::bySamples) &&
        (mode != (int)SelectTimeWidget::byPercents))
    {
	return -EINVAL;
    }
    m_start_mode = (SelectTimeWidget::Mode)mode;


    // selection mode
    param = params[1];
    mode = param.toInt(&ok);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;
    Q_ASSERT((mode == (int)SelectTimeWidget::byTime) ||
           (mode == (int)SelectTimeWidget::bySamples) ||
           (mode == (int)SelectTimeWidget::byPercents));
    if ((mode != (int)SelectTimeWidget::byTime) &&
        (mode != (int)SelectTimeWidget::bySamples) &&
        (mode != (int)SelectTimeWidget::byPercents))
    {
	return -EINVAL;
    }
    m_range_mode = (SelectTimeWidget::Mode)mode;

    // offset in ms, samples or percent
    param = params[2];
    m_start = (unsigned int)param.toDouble(&ok);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    // range in ms, samples or percent
    param = params[3];
    m_range = (unsigned int)param.toDouble(&ok);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    return 0;
}

//***************************************************************************
#include "SelectRangePlugin.moc"
//***************************************************************************
//***************************************************************************
